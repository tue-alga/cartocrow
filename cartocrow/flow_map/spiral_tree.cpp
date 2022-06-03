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

#include "cartocrow/core/circular_range.h"
#include "cartocrow/core/circulator.h"
#include "cartocrow/core/intersections.h"
#include "cartocrow/core/polar_segment.h"
#include "cartocrow/core/spiral_segment.h"

namespace cartocrow::flow_map {

SpiralTree::SpiralTree(const Point& root, const Number& restricting_angle)
    : m_restricting_angle(restricting_angle), m_root_translation(Point(CGAL::ORIGIN) - root) {
	CHECK_GT(restricting_angle, 0);
	CHECK_LE(restricting_angle, M_PI_2);
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

void SpiralTree::addNode(std::shared_ptr<Node> node) {
	m_nodes.push_back(node);
}

void SpiralTree::addPlace(const Place& place) {
	auto newPlace = std::make_shared<Place>(place);
	m_places.push_back(newPlace);
	m_nodes.push_back(std::make_shared<Node>(newPlace));
}

void SpiralTree::addObstacle(const Polygon& obstacle) {
	//CHECK_NE(obstacle.oriented_side(root()), CGAL::ON_BOUNDED_SIDE) << "Root inside an obstacle.";

	std::cout << "adding obstacle with vertex count " << obstacle.size() << std::endl;
	Obstacle& addedObstacle = m_obstacles.emplace_back();
	for (const auto& vertex : obstacle) {
		PolarPoint p(vertex, CGAL::ORIGIN - root());
		addedObstacle.push_back(p);
	}

	// enforce clockwise vertex order
	if (!obstacle.is_counterclockwise_oriented()) {
		addedObstacle.reverse();
	}

	// subdivide each edge as necessary to include the closest point to the root
	// and spiral points
	const Number phi_offset = M_PI_2 - m_restricting_angle;
	CHECK_LT(0, phi_offset);

	Obstacle::iterator vertex_prev = --addedObstacle.end();
	for (Obstacle::iterator vertex_iter = addedObstacle.begin(); vertex_iter != addedObstacle.end();
	     vertex_prev = vertex_iter++) {
		const PolarSegment edge{*vertex_prev, *vertex_iter};
		const PolarPoint closest = edge.SupportingLine().foot();

		// the spiral points have fixed R and their phi is offset from the phi of the closest by +/- phi_offset
		const Number r_spiral = closest.R() / std::sin(m_restricting_angle);

		const int sign = vertex_prev->phi() < vertex_iter->phi() ? -1 : 1;
		const Number phi_spiral_prev = closest.phi() + sign * phi_offset;
		const Number phi_spiral_next = closest.phi() - sign * phi_offset;

		// the closest point and spiral points must be added ordered from p to q and only if they are on the edge
		if (edge.ContainsPhi(phi_spiral_prev)) {
			addedObstacle.insert(vertex_iter, PolarPoint(r_spiral, phi_spiral_prev));
		}
		if (edge.ContainsPhi(closest.phi())) {
			addedObstacle.insert(vertex_iter, closest);
		}
		if (edge.ContainsPhi(phi_spiral_next)) {
			addedObstacle.insert(vertex_iter, PolarPoint(r_spiral, phi_spiral_next));
		}
	}
}

void SpiralTree::addShields() {
	for (const std::shared_ptr<Place>& place : places()) {
		PolarPoint position(place->position, CGAL::ORIGIN - root());
		std::cout << position << std::endl;
		if (position.R() == 0) {
			continue;
		}
		Point p = position.to_cartesian() + (root() - CGAL::ORIGIN);
		const Number shieldWidth = 2;
		Vector v1 = PolarPoint(shieldWidth, position.phi() + M_PI_2).to_cartesian() - CGAL::ORIGIN;
		const Number shieldThickness = 2;
		Vector v2 = PolarPoint(shieldThickness, position.phi()).to_cartesian() - CGAL::ORIGIN;
		Polygon polygon;
		polygon.push_back(p + 0.5 * v2 + v1);
		polygon.push_back(p + 0.5 * v2 - v1);
		polygon.push_back(p + v2);
		addObstacle(polygon);
	}
}

void SpiralTree::setRoot(const Point& root) {
	m_root_translation = Point(CGAL::ORIGIN) - root;
	clean();
}

Point SpiralTree::root() const {
	return Point(CGAL::ORIGIN) - m_root_translation;
}

void SpiralTree::setRestrictingAngle(const Number& restricting_angle) {
	CHECK_GT(restricting_angle, 0);
	CHECK_LT(restricting_angle, M_PI_2);
	m_restricting_angle = restricting_angle;
	clean();
}

Number SpiralTree::restrictingAngle() const {
	return m_restricting_angle;
}

void SpiralTree::clean() {
	size_t num_places = 0;

	// Clean the node connections.
	for (std::shared_ptr<Node>& node : m_nodes) {
		if (node->m_place == nullptr) {
			break;
		}
		++num_places;

		node->m_parent = nullptr;
		node->m_children.clear();
	}

	// Remove support nodes, e.g. join nodes.
	m_nodes.resize(num_places);
}

bool SpiralTree::isReachable(const PolarPoint& parent_point, const PolarPoint& child_point) const {
	if (parent_point == child_point) {
		return true;
	}

	const Spiral spiral(child_point, parent_point);
	return std::abs(spiral.angle_rad()) <= m_restricting_angle;
}

} // namespace cartocrow::flow_map
