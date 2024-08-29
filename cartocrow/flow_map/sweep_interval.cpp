/*
The CartoCrow library implements algorithmic geo-visualization methods,
developed at TU Eindhoven.
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
*/

#include "sweep_interval.h"

#include "../core/core.h"
#include "../renderer/geometry_renderer.h"
#include "intersections.h"
#include "polar_segment.h"
#include "spiral_segment.h"
#include "sweep_edge.h"

#include <ranges>

#include <CGAL/Ray_2.h>
#include <CGAL/enum.h>

namespace cartocrow::flow_map {

SweepInterval::SweepInterval(Type type)
    : m_type(type), m_previousBoundary(nullptr), m_nextBoundary(nullptr) {}

SweepInterval::SweepInterval(const SweepInterval& other, SweepEdge* previousBoundary,
                             SweepEdge* nextBoundary)
    : m_type(other.m_type), m_node(other.m_node), m_activeDescendant(other.m_activeDescendant),
      m_previousBoundary(previousBoundary), m_nextBoundary(nextBoundary) {}

SweepEdge* SweepInterval::previousBoundary() {
	return m_previousBoundary;
}

SweepEdge* SweepInterval::nextBoundary() {
	return m_nextBoundary;
}

void SweepInterval::setType(Type type) {
	m_type = type;
}

SweepInterval::Type SweepInterval::type() const {
	return m_type;
}

void SweepInterval::setNode(std::shared_ptr<Node> node) {
	m_node = node;
}

std::shared_ptr<Node> SweepInterval::node() const {
	return m_node;
}

void SweepInterval::setActiveDescendant(std::shared_ptr<Node> activeDescendant) {
	m_activeDescendant = activeDescendant;
}

std::shared_ptr<Node> SweepInterval::activeDescendant() const {
	return m_activeDescendant;
}

std::optional<PolarPoint> SweepInterval::outwardsVanishingPoint(Number<Inexact> rMin) const {
	if (m_previousBoundary == nullptr || m_nextBoundary == nullptr) {
		return std::nullopt;
	}
	std::optional<Number<Inexact>> r =
	    m_previousBoundary->shape().intersectOutwardsWith(m_nextBoundary->shape(), rMin);
	if (!r) {
		return std::nullopt;
	}
	return m_previousBoundary->shape().evalForR(*r);
}

std::optional<PolarPoint> SweepInterval::inwardsVanishingPoint(Number<Inexact> rMax) const {
	if (m_previousBoundary == nullptr || m_nextBoundary == nullptr) {
		return std::nullopt;
	}
	std::optional<Number<Inexact>> r =
	    m_previousBoundary->shape().intersectInwardsWith(m_nextBoundary->shape(), rMax);
	if (!r) {
		return std::nullopt;
	}
	return m_previousBoundary->shape().evalForR(*r);
}

Polygon<Inexact> SweepInterval::sweepShape(Number<Inexact> rFrom, Number<Inexact> rTo) const {

	const Number<Inexact> SWEEP_SHAPE_RESOLUTION = 0.05;

	// special case: if we're the only interval on the circle, just draw an annulus
	if (!m_nextBoundary) {
		Polygon<Inexact> result;
		for (Number<Inexact> phi = 0; phi < 2 * M_PI; phi += SWEEP_SHAPE_RESOLUTION) {
			result.push_back(PolarPoint(rFrom, phi).toCartesian());
		}
		result.push_back(PolarPoint(rFrom, 2 * M_PI).toCartesian());
		for (Number<Inexact> phi = 2 * M_PI; phi > 0; phi -= SWEEP_SHAPE_RESOLUTION) {
			result.push_back(PolarPoint(rTo, phi).toCartesian());
		}
		result.push_back(PolarPoint(rTo, 0).toCartesian());
		return result;
	}

	Number<Inexact> rMid = (rFrom + rTo) / 2;
	Number<Inexact> alpha = wrapAngleUpper(m_nextBoundary->shape().phiForR(rMid) -
	                                       m_previousBoundary->shape().phiForR(rMid));

	// compute side edges
	std::vector<PolarPoint> leftNearEdge, rightNearEdge, leftFarEdge, rightFarEdge;
	for (Number<Inexact> r = rMid; r > rFrom; r /= 1 + SWEEP_SHAPE_RESOLUTION) {
		leftNearEdge.push_back(m_nextBoundary->shape().evalForR(r));
		rightNearEdge.push_back(m_previousBoundary->shape().evalForR(r));
	}
	leftNearEdge.push_back(m_nextBoundary->shape().evalForR(rFrom));
	rightNearEdge.push_back(m_previousBoundary->shape().evalForR(rFrom));
	for (Number<Inexact> r = rMid; r < rTo; r *= 1 + SWEEP_SHAPE_RESOLUTION) {
		leftFarEdge.push_back(m_nextBoundary->shape().evalForR(r));
		rightFarEdge.push_back(m_previousBoundary->shape().evalForR(r));
	}
	leftFarEdge.push_back(m_nextBoundary->shape().evalForR(rTo));
	rightFarEdge.push_back(m_previousBoundary->shape().evalForR(rTo));

	// near arc
	std::vector<PolarPoint> nearArc;
	Number<Inexact> nearPhiNext = m_nextBoundary->shape().phiForR(rFrom);
	Number<Inexact> alphaNear = alpha + angleSpan(leftNearEdge) - angleSpan(rightNearEdge);
	for (Number<Inexact> phi = nearPhiNext; phi > nearPhiNext - alphaNear; phi -= SWEEP_SHAPE_RESOLUTION) {
		nearArc.push_back(PolarPoint(rFrom, phi));
	}

	// far arc
	std::vector<PolarPoint> farArc;
	Number<Inexact> farPhiNext = m_nextBoundary->shape().phiForR(rTo);
	Number<Inexact> alphaFar = alpha + angleSpan(leftFarEdge) - angleSpan(rightFarEdge);
	for (Number<Inexact> phi = farPhiNext; phi > farPhiNext - alphaFar; phi -= SWEEP_SHAPE_RESOLUTION) {
		farArc.push_back(PolarPoint(rTo, phi));
	}

	// assemble the result
	Polygon<Inexact> result;
	for (auto vertex : leftNearEdge) {
		result.push_back(vertex.toCartesian());
	}
	for (auto vertex : nearArc) {
		result.push_back(vertex.toCartesian());
	}
	for (auto vertex = rightNearEdge.rbegin(); vertex != rightNearEdge.rend(); ++vertex) {
		result.push_back(vertex->toCartesian());
	}
	for (auto vertex : rightFarEdge) {
		result.push_back(vertex.toCartesian());
	}
	for (auto vertex = farArc.rbegin(); vertex != farArc.rend(); ++vertex) {
		result.push_back(vertex->toCartesian());
	}
	for (auto vertex = leftFarEdge.rbegin(); vertex != leftFarEdge.rend(); ++vertex) {
		result.push_back(vertex->toCartesian());
	}
	return result;
}

Number<Inexact> SweepInterval::angleSpan(std::vector<PolarPoint>& vertices) const {
	Number<Inexact> angle = 0;
	for (size_t i = 0; i + 1 < vertices.size(); i++) {
		Number<Inexact> phi1 = vertices[i].phi();
		Number<Inexact> phi2 = vertices[i + 1].phi();
		angle += wrapAngle(phi2 - phi1, -M_PI);
	}
	return angle;
}

void SweepInterval::paintSweepShape(renderer::GeometryRenderer& renderer, Number<Inexact> rFrom,
                                    Number<Inexact> rTo) const {
	renderer.pushStyle();
	renderer.setMode(cartocrow::renderer::GeometryRenderer::fill |
	                 cartocrow::renderer::GeometryRenderer::stroke);
	renderer.setStroke(Color{127, 127, 127}, 0.2);
	renderer.setFillOpacity(50);
	if (m_type == Type::SHADOW || m_type == Type::FREE) {
		renderer.setFill(Color{255, 255, 255});
	} else if (m_type == Type::REACHABLE) {
		renderer.setFill(Color{162, 255, 128});
	} else if (m_type == Type::OBSTACLE) {
		renderer.setFill(Color{220, 160, 130});
	}
	renderer.draw(sweepShape(rFrom, rTo));
	renderer.popStyle();
}

} // namespace cartocrow::flow_map
