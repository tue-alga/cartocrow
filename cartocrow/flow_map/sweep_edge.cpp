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

#include "sweep_edge.h"

#include "cartocrow/flow_map/sweep_interval.h"
#include "polar_segment.h"
#include "spiral_segment.h"

namespace cartocrow::flow_map {

SweepEdgeShape::SweepEdgeShape(PolarPoint start, PolarPoint end)
    : m_type(Type::SEGMENT), m_start(start), m_end(end) {}

SweepEdgeShape::SweepEdgeShape(Type type, PolarPoint start, Number<Inexact> alpha)
    : m_type(type), m_start(start), m_alpha(alpha) {}

SweepEdgeShape::Type SweepEdgeShape::type() const {
	return m_type;
}

PolarPoint SweepEdgeShape::start() const {
	return m_start;
}

std::optional<PolarPoint> SweepEdgeShape::end() const {
	return m_end;
}

PolarPoint SweepEdgeShape::nearEndpoint() const {
	if (m_end) {
		return m_start.r() < m_end->r() ? m_start : *m_end;
	} else {
		return m_start;
	}
}

std::optional<PolarPoint> SweepEdgeShape::farEndpoint() const {
	if (m_end) {
		return m_start.r() < m_end->r() ? *m_end : m_start;
	} else {
		return std::nullopt;
	}
}

Number<Inexact> SweepEdgeShape::nearR() const {
	return nearEndpoint().r();
}

std::optional<Number<Inexact>> SweepEdgeShape::farR() const {
	if (farEndpoint()) {
		return farEndpoint()->r();
	} else {
		return std::nullopt;
	}
}

Number<Inexact> SweepEdgeShape::phiForR(Number<Inexact> r) const {
	assert(r >= nearR() && (!farR() || r <= farR())); // trying to compute phi for out-of-bounds r

	// for robustness, it is important that if we are exactly at the beginning
	// or end of the edge, we return the exact phi of its begin or end point
	if (r == nearR()) {
		return nearEndpoint().phi();
	}
	if (farR() && r == farR()) {
		return farEndpoint()->phi();
	}

	if (m_type == Type::SEGMENT) {
		PolarSegment s(m_start, *m_end);
		std::array<Number<Inexact>, 2> phis;
		int phiCount = s.collectPhi(r, &phis[0]);
		assert(phiCount != 0); // line segment did not contain the given phi
		assert(phiCount == 1); // line segment contained two of the given phis
		return phis[0];
	} else {
		Spiral s(m_start, m_type == Type::LEFT_SPIRAL ? -m_alpha : m_alpha);
		return s.phiForR(r);
	}
}

PolarPoint SweepEdgeShape::evalForR(Number<Inexact> r) const {
	return PolarPoint(r, phiForR(r));
}

bool SweepEdgeShape::departsToLeftOf(const SweepEdgeShape& shape) const {
	Number<Inexact> r = this->nearR();
	assert(shape.nearR() == r);

	// TODO temporary implementation with epsilon, should be replaced by angle
	// computations
	return PolarSegment(this->evalForR(r + 0.00001), shape.evalForR(r + 0.00001)).isLeftLine();
}

SpiralSegment SweepEdgeShape::toSpiralSegment() const {
	return SpiralSegment(m_start, m_type == Type::LEFT_SPIRAL ? -m_alpha : m_alpha, nearR(),
	                     nearR() * 100); // TODO
}

SweepEdge::SweepEdge(SweepEdgeShape shape)
    : m_shape(shape), m_previousInterval(nullptr),
      m_nextInterval(SweepInterval(SweepInterval::Type::REACHABLE)) {}

const SweepEdgeShape& SweepEdge::shape() const {
	return m_shape;
}

SweepEdge* SweepEdge::previousEdge() {
	return m_previousInterval->previousBoundary();
}

SweepInterval* SweepEdge::previousInterval() {
	return m_previousInterval;
}

SweepInterval* SweepEdge::nextInterval() {
	return &m_nextInterval;
}

SweepEdge* SweepEdge::nextEdge() {
	return m_nextInterval.nextBoundary();
}

} // namespace cartocrow::flow_map
