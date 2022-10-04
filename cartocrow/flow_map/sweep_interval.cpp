/*
The CartoCrow library implements algorithmic geo-visualization methods,
developed at TU Eindhoven.
Copyright (C) 2021  Netherlands eScience Center and TU Eindhoven

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3f of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "sweep_interval.h"

#include "../renderer/geometry_renderer.h"
#include "cartocrow/core/core.h"
#include "intersections.h"
#include "polar_segment.h"
#include "spiral_segment.h"
#include "sweep_edge.h"
#include <CGAL/Ray_2.h>
#include <CGAL/enum.h>

namespace cartocrow::flow_map {

SweepInterval::SweepInterval(Type type)
    : m_type(type), m_previousBoundary(nullptr), m_nextBoundary(nullptr) {}

SweepInterval::SweepInterval(Type type, SweepEdge* previousBoundary, SweepEdge* nextBoundary)
    : m_type(type), m_previousBoundary(previousBoundary), m_nextBoundary(nextBoundary) {}

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

std::optional<PolarPoint> SweepInterval::vanishingPoint(Number<Inexact> rMin) const {
	if (m_previousBoundary == nullptr || m_nextBoundary == nullptr) {
		return std::nullopt;
	}
	std::optional<Number<Inexact>> r =
	    m_previousBoundary->shape().intersectWith(m_nextBoundary->shape(), rMin);
	if (!r) {
		return std::nullopt;
	}
	return m_previousBoundary->shape().evalForR(*r);
}

Polygon<Inexact> SweepInterval::sweepShape(Number<Inexact> rFrom, Number<Inexact> rTo) const {
	std::vector<PolarPoint> vertices;
	// left side
	if (m_nextBoundary) {
		for (Number<Inexact> r = rTo; r > rFrom; r /= 1.05) {
			auto p = m_nextBoundary->shape().evalForR(r);
			vertices.push_back(p);
		}
	} else {
		vertices.push_back(PolarPoint(rTo, M_PI));
		vertices.push_back(PolarPoint(rFrom, M_PI));
	}
	// near side
	Number<Inexact> nearPhiNext = m_nextBoundary ? m_nextBoundary->shape().phiForR(rFrom) : M_PI;
	Number<Inexact> nearPhiPrevious =
	    m_previousBoundary ? m_previousBoundary->shape().phiForR(rFrom) : -M_PI;
	if (nearPhiNext < nearPhiPrevious ||
	    (nearPhiNext == nearPhiPrevious &&
	     m_previousBoundary->shape().departsToLeftOf(m_nextBoundary->shape()))) {
		nearPhiPrevious -= 2 * M_PI;
	}
	for (Number<Inexact> phi = nearPhiNext; phi > nearPhiPrevious; phi -= 0.05) {
		vertices.push_back(PolarPoint(rFrom, phi));
	}
	// right side
	if (m_previousBoundary) {
		for (Number<Inexact> r = rFrom; r < rTo; r *= 1.05) {
			auto p = m_previousBoundary->shape().evalForR(r);
			vertices.push_back(p);
		}
	} else {
		vertices.push_back(PolarPoint(rFrom, -M_PI));
		vertices.push_back(PolarPoint(rTo, -M_PI));
	}
	// far side
	Number<Inexact> farPhiPrevious =
	    m_previousBoundary ? m_previousBoundary->shape().phiForR(rTo) : -M_PI;
	Number<Inexact> farPhiNext = m_nextBoundary ? m_nextBoundary->shape().phiForR(rTo) : M_PI;
	Number<Inexact> angle;
	for (size_t i = 0; i < vertices.size() - 1; i++) {
		Number<Inexact> phi1 = PolarPoint(vertices[i]).phi();
		Number<Inexact> phi2 = PolarPoint(vertices[i + 1]).phi();
		angle += wrapAngle(phi2 - phi1, -M_PI);
	}
	for (Number<Inexact> phi = farPhiPrevious; phi < farPhiPrevious - angle; phi += 0.05) {
		vertices.push_back(PolarPoint(rTo, phi));
	}

	Polygon<Inexact> result;
	for (auto vertex : vertices) {
		result.push_back(vertex.toCartesian());
	}
	return result;
}

void SweepInterval::paintSweepShape(renderer::GeometryRenderer& renderer, Number<Inexact> rFrom,
                                    Number<Inexact> rTo) const {
	renderer.pushStyle();
	renderer.setMode(cartocrow::renderer::GeometryRenderer::fill |
	                 cartocrow::renderer::GeometryRenderer::stroke);
	renderer.setStroke(Color{127, 127, 127}, 0.2);
	renderer.setFillOpacity(50);
	if (m_type == Type::SHADOW) {
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
