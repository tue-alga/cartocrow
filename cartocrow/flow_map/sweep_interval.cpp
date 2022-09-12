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

#include "cartocrow/renderer/geometry_renderer.h"
#include "polar_segment.h"
#include "spiral_segment.h"
#include "sweep_edge.h"

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

Polygon<Inexact> SweepInterval::sweepShape(Number<Inexact> rFrom, Number<Inexact> rTo) const {
	/*std::cout << rFrom << " " << rTo;
	if (m_previousBoundary) {
		std::cout << " previous " << m_previousBoundary->shape().nearR();
		if (m_previousBoundary->shape().farR()) {
			std::cout << "-" << *m_previousBoundary->shape().farR();
		}
	}
	if (m_nextBoundary) {
		std::cout << " next " << m_nextBoundary->shape().nearR();
		if (m_nextBoundary->shape().farR()) {
			std::cout << "-" << *m_nextBoundary->shape().farR();
		}
	}
	std::cout << std::endl;*/
	Polygon<Inexact> result;
	// right side
	if (m_previousBoundary) {
		for (Number<Inexact> r = rFrom; r < rTo; r *= 1.05) {
			result.push_back(m_previousBoundary->shape().evalForR(r).toCartesian());
		}
	} else {
		result.push_back(PolarPoint(rFrom, 0).toCartesian());
		result.push_back(PolarPoint(rTo, 0).toCartesian());
	}
	// far side
	Number<Inexact> farPhiFrom = m_previousBoundary ? m_previousBoundary->shape().phiForR(rTo) : 0;
	Number<Inexact> farPhiTo = m_nextBoundary ? m_nextBoundary->shape().phiForR(rTo) : M_PI * 2;
	for (Number<Inexact> phi = farPhiFrom; phi < farPhiTo; phi += 0.01) {
		result.push_back(PolarPoint(rTo, phi).toCartesian());
	}
	// left side
	if (m_nextBoundary) {
		for (Number<Inexact> r = rTo; r > rFrom; r /= 1.05) {
			result.push_back(m_nextBoundary->shape().evalForR(r).toCartesian());
		}
	} else {
		result.push_back(PolarPoint(rTo, 0).toCartesian());
		result.push_back(PolarPoint(rFrom, 0).toCartesian());
	}
	// near side
	Number<Inexact> nearPhiFrom = m_nextBoundary ? m_nextBoundary->shape().phiForR(rFrom) : M_PI * 2;
	Number<Inexact> nearPhiTo = m_previousBoundary ? m_previousBoundary->shape().phiForR(rFrom) : 0;
	for (Number<Inexact> phi = nearPhiFrom; phi > nearPhiTo; phi -= 0.01) {
		result.push_back(PolarPoint(rFrom, phi).toCartesian());
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
		renderer.setFill(Color{230, 190, 170});
	}
	renderer.draw(sweepShape(rFrom, rTo));
	renderer.popStyle();
}

} // namespace cartocrow::flow_map
