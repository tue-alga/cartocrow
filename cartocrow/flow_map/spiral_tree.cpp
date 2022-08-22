/*
The Flow Map library implements the algorithmic geo-visualization
method by the same name, developed by Kevin Verbeek, Kevin Buchin,
and Bettina Speckmann at TU Eindhoven
(DOI: 10.1007/s00453-013-9867-z & 10.1109/TVCG.2011.202).
Copyright (C) 2021  Netherlands eScience Center and TU Eindhoven

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

Created by tvl (t.vanlankveld@esciencecenter.nl) on 28-10-2020
*/

#include "spiral_tree.h"

#include <cmath>
#include <optional>
#include <ostream>

#include <glog/logging.h>

//#include "circular_range.h"
#include "circulator.h"
#include "intersections.h"
#include "polar_segment.h"
#include "spiral_segment.h"

namespace cartocrow::flow_map {

SpiralTree::SpiralTree(const Point<Inexact> rootPosition, const Number<Inexact> restrictingAngle)
    : m_restrictingAngle(restrictingAngle), m_rootPosition(rootPosition) {
	if (restrictingAngle < 0 || restrictingAngle > M_PI_2) {
		throw std::runtime_error("Restricting angle outside [0, 2 * pi]");
	}

	auto root = std::make_shared<Place>("root", rootPosition, 0);
	m_places.push_back(root);
	m_root = std::make_shared<Node>(PolarPoint(0, 0), root);
	m_nodes.push_back(m_root);
}

const std::vector<std::shared_ptr<Place>>& SpiralTree::places() const {
	return m_places;
}

const std::vector<std::shared_ptr<Node>>& SpiralTree::nodes() const {
	return m_nodes;
}

const std::vector<SpiralTree::Obstacle>& SpiralTree::obstacles() const {
	return m_obstacles;
}

void SpiralTree::addPlace(const std::string& name, const Point<Inexact>& position,
                          Number<Inexact> flow) {
	auto newPlace = std::make_shared<Place>(name, position, flow);
	m_places.push_back(newPlace);
	PolarPoint polarPosition(position - (m_rootPosition - CGAL::ORIGIN));
	m_nodes.push_back(std::make_shared<Node>(polarPosition, newPlace));
}

void SpiralTree::addObstacle(const Polygon<Inexact>& obstacle) {
	//CHECK_NE(obstacle.oriented_side(root()), CGAL::ON_BOUNDED_SIDE) << "Root inside an obstacle.";

	std::cout << "adding obstacle with vertex count " << obstacle.size() << std::endl;
	Obstacle& addedObstacle = m_obstacles.emplace_back();
	for (const auto& vertex : obstacle) {
		PolarPoint p(vertex, CGAL::ORIGIN - rootPosition());
		addedObstacle.push_back(p);
	}

	// enforce clockwise vertex order
	if (!obstacle.is_counterclockwise_oriented()) {
		addedObstacle.reverse();
	}

	// subdivide each edge as necessary to include the closest point to the root
	// and spiral points
	const Number<Inexact> phi_offset = M_PI_2 - m_restrictingAngle;
	CHECK_LT(0, phi_offset);

	Obstacle::iterator vertex_prev = --addedObstacle.end();
	for (Obstacle::iterator vertex_iter = addedObstacle.begin(); vertex_iter != addedObstacle.end();
	     vertex_prev = vertex_iter++) {
		const PolarSegment edge{*vertex_prev, *vertex_iter};
		const PolarPoint closest = edge.supportingLine().foot();

		// the spiral points have fixed R and their phi is offset from the phi of the closest by +/- phi_offset
		const Number<Inexact> r_spiral = closest.r() / std::sin(m_restrictingAngle);

		const int sign = vertex_prev->phi() < vertex_iter->phi() ? -1 : 1;
		const Number<Inexact> phi_spiral_prev = closest.phi() + sign * phi_offset;
		const Number<Inexact> phi_spiral_next = closest.phi() - sign * phi_offset;

		// the closest point and spiral points must be added ordered from p to q and only if they are on the edge
		if (edge.containsPhi(phi_spiral_prev)) {
			addedObstacle.insert(vertex_iter, PolarPoint(r_spiral, phi_spiral_prev));
		}
		if (edge.containsPhi(closest.phi())) {
			addedObstacle.insert(vertex_iter, closest);
		}
		if (edge.containsPhi(phi_spiral_next)) {
			addedObstacle.insert(vertex_iter, PolarPoint(r_spiral, phi_spiral_next));
		}
	}
}

void SpiralTree::addShields() {
	for (const std::shared_ptr<Place>& place : places()) {
		PolarPoint position(place->m_position, CGAL::ORIGIN - rootPosition());
		std::cout << position << std::endl;
		if (position.r() == 0) {
			continue;
		}
		Point<Inexact> p = position.toCartesian() + (rootPosition() - CGAL::ORIGIN);
		const Number<Inexact> shieldWidth = 2;
		Vector<Inexact> v1 =
		    PolarPoint(shieldWidth, position.phi() + M_PI_2).toCartesian() - CGAL::ORIGIN;
		const Number<Inexact> shieldThickness = 2;
		Vector<Inexact> v2 = PolarPoint(shieldThickness, position.phi()).toCartesian() - CGAL::ORIGIN;
		Polygon<Inexact> polygon;
		polygon.push_back(p + 0.5 * v2 + v1);
		polygon.push_back(p + 0.5 * v2 - v1);
		polygon.push_back(p + v2);
		addObstacle(polygon);
	}
}

/*void SpiralTree::setRoot(const Point<Inexact>& root) {
	m_root_translation = Point<Inexact>(CGAL::ORIGIN) - root;
	clean();
}*/

Point<Inexact> SpiralTree::rootPosition() const {
	return m_rootPosition;
}

/*void SpiralTree::setRestrictingAngle(const Number<Inexact>& restricting_angle) {
	CHECK_GT(restricting_angle, 0);
	CHECK_LT(restricting_angle, M_PI_2);
	m_restricting_angle = restricting_angle;
	clean();
}*/

Number<Inexact> SpiralTree::restrictingAngle() const {
	return m_restrictingAngle;
}

void SpiralTree::clean() {
	size_t num_places = 0;

	// clean the node connections
	for (std::shared_ptr<Node>& node : m_nodes) {
		if (node->m_place == nullptr) {
			break;
		}
		++num_places;

		node->m_parent = nullptr;
		node->m_children.clear();
	}

	// remove support nodes, e.g. join nodes
	m_nodes.resize(num_places);
}

bool SpiralTree::isReachable(const PolarPoint& parent_point, const PolarPoint& child_point) const {
	if (parent_point == child_point) {
		return true;
	}

	const Spiral spiral(child_point, parent_point);
	return std::abs(spiral.angle()) <= m_restrictingAngle;
}

} // namespace cartocrow::flow_map
