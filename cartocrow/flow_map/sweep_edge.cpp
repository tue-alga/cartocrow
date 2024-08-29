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

#include "sweep_edge.h"

#include <cmath>
#include <limits>

#include "polar_segment.h"
#include "spiral_segment.h"
#include "sweep_interval.h"

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

void SweepEdgeShape::pruneNearSide(PolarPoint newNear) {
	if (!m_end) {
		m_start = newNear;
	} else if (m_start.r() < m_end->r()) {
		m_start = newNear;
	} else {
		m_end = newNear;
	}
}

void SweepEdgeShape::pruneFarSide(PolarPoint newFar) {
	if (!m_end) {
		m_end = newFar;
	} else if (m_start.r() < m_end->r()) {
		m_end = newFar;
	} else {
		m_start = newFar;
	}
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

std::optional<Number<Inexact>> SweepEdgeShape::averageR() const {
	if (farR()) {
		return (nearR() + *farR()) / 2;
	} else {
		return std::nullopt;
	}
}

Number<Inexact> SweepEdgeShape::phiForR(Number<Inexact> r) const {
	assert(!farR() || (r >= nearR() && r <= farR())); // trying to compute φ for out-of-bounds r

	// for robustness, it is important that if we are exactly at the beginning
	// or end of the edge, we return the exact φ of its begin or end point
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
		assert(phiCount <= 1); // line segment contained two of the given φs
		if (phiCount == 0) {
			// for floating-point robustness: if we are just within the near-far
			// range, then it can happen that PolarSegment claims there is no
			// intersection even though there should be one; in this case simply
			// return the near or far φ
			if (!farR() || std::abs(r - nearR()) < std::abs(r - *farR())) {
				return nearEndpoint().phi();
			} else {
				return farEndpoint()->phi();
			}
		}
		return phis[0];
	} else {
		Spiral s(m_start, m_type == Type::LEFT_SPIRAL ? -m_alpha : m_alpha);
		return s.phiForR(r);
	}
}

PolarPoint SweepEdgeShape::evalForR(Number<Inexact> r) const {
	return PolarPoint(r, phiForR(r));
}

Number<Inexact> SweepEdgeShape::tangentAngleForR(Number<Inexact> r) const {
	assert(!farR() || (r >= nearR() && r <= farR())); // trying to compute tangent angle for out-of-bounds r
	if (m_type == Type::SEGMENT) {
		Point<Inexact> start = m_start.toCartesian();
		Point<Inexact> end = m_end->toCartesian();
		if (m_start.r() > m_end->r()) {
			std::swap(start, end);
		}
		return std::atan2(end.y() - start.y(), end.x() - start.x());
	} else {
		return wrapAngle(evalForR(r).phi() + (m_type == Type::LEFT_SPIRAL ? m_alpha : -m_alpha), -M_PI);
	}
}

bool SweepEdgeShape::departsOutwardsToLeftOf(Number<Inexact> r, const SweepEdgeShape& other) const {
	assert(phiForR(r) == other.phiForR(r));

	Number<Inexact> thisAngle = tangentAngleForR(r);
	Number<Inexact> otherAngle = other.tangentAngleForR(r);
	return wrapAngle(thisAngle - otherAngle, -M_PI) > 0;
}

std::optional<Number<Inexact>> SweepEdgeShape::intersectOutwardsWith(const SweepEdgeShape& other,
                                                                     Number<Inexact> rMin) const {
	if (this->type() == Type::SEGMENT && other.type() == Type::SEGMENT) {
		return std::nullopt;
	}
	Number<Inexact> alpha = this->type() != Type::SEGMENT ? m_alpha : other.m_alpha;

	auto isLeftOf = [&](Number<Inexact> r) {
		return PolarSegment(evalForR(r), other.evalForR(r)).isLeftLine();
	};
	// at distance rMin + ε, are we to the left of the other shape?
	bool initiallyLeftOfOther;
	if (phiForR(rMin) == other.phiForR(rMin)) {
		initiallyLeftOfOther = departsOutwardsToLeftOf(rMin, other);
	} else {
		initiallyLeftOfOther = isLeftOf(rMin);
	}

	Number<Inexact> rMax = std::numeric_limits<Number<Inexact>>::infinity();
	if (farR()) {
		rMax = std::min(rMax, *farR());
	}
	if (other.farR()) {
		rMax = std::min(rMax, *other.farR());
	}

	Number<Inexact> rLower = rMin;
	Number<Inexact> rUpper = rLower;
	Number<Inexact> rLimit = std::min(rMax, rMin * std::exp(2 * M_PI / std::tan(alpha)));
	while (rUpper < rLimit) {
		rUpper = std::min(rMax, rUpper * std::exp(M_PI / (8 * std::tan(alpha))));

		if (isLeftOf(rUpper) != initiallyLeftOfOther) {
			Number<Inexact> angleDifference = std::abs(phiForR(rUpper) - other.phiForR(rUpper));
			if (angleDifference < M_PI / 2 || angleDifference > 3 * M_PI / 2) {
				// found intersection
				break;
			} else {
				// found wraparound, continue searching
				rLower = rUpper;
				initiallyLeftOfOther = !initiallyLeftOfOther;
			}
		}
	}

	if (isLeftOf(rUpper) == initiallyLeftOfOther) {
		return std::nullopt;
	}

	// binary search
	for (int i = 0; i < 30; i++) {
		Number<Inexact> rMid = (rLower + rUpper) / 2;
		bool leftOfOther = isLeftOf(rMid);
		if (leftOfOther == initiallyLeftOfOther) {
			rLower = rMid;
		} else {
			rUpper = rMid;
		}
	}

	return (rLower + rUpper) / 2;
}

bool SweepEdgeShape::departsInwardsToLeftOf(Number<Inexact> r, const SweepEdgeShape& other) const {
	assert(phiForR(r) == other.phiForR(r));

	Number<Inexact> thisAngle = tangentAngleForR(r);
	Number<Inexact> otherAngle = other.tangentAngleForR(r);
	Number<Inexact> angleDifference = wrapAngle(otherAngle - thisAngle, -M_PI);
	if (std::abs(angleDifference) > M_EPSILON) {
		return angleDifference > 0;
	}
	// if they have the same angle, then decide based on the curve direction
	// (that is: a right spiral curves inwards more to the left than a segment,
	// which in turn curves more to the left than a left spiral)
	return other.signedAlpha() > this->signedAlpha();
}

std::optional<Number<Inexact>> SweepEdgeShape::intersectInwardsWith(const SweepEdgeShape& other,
                                                                    Number<Inexact> rMax) const {
	if (this->type() == Type::SEGMENT && other.type() == Type::SEGMENT) {
		return std::nullopt;
	}
	Number<Inexact> alpha = this->type() != Type::SEGMENT ? m_alpha : other.m_alpha;

	auto isLeftOf = [&](Number<Inexact> r) {
		return PolarSegment(evalForR(r), other.evalForR(r)).isLeftLine();
	};
	// at distance rMax - ε, are we to the left of the other shape?
	bool initiallyLeftOfOther;
	if (phiForR(rMax) == other.phiForR(rMax)) {
		initiallyLeftOfOther = departsInwardsToLeftOf(rMax, other);
	} else {
		initiallyLeftOfOther = isLeftOf(rMax);
	}

	Number<Inexact> rMin = 0;
	if (farR()) {
		rMin = std::max(rMin, nearR());
	}
	if (other.farR()) {
		rMin = std::max(rMin, other.nearR());
	}

	Number<Inexact> rUpper = rMax;
	Number<Inexact> rLower = rUpper;
	Number<Inexact> rLimit = std::max(rMin, rMax / std::exp(2 * M_PI / std::tan(alpha)));
	while (rLower > rLimit) {
		rLower = std::max(rMin, rLower / std::exp(M_PI / (8 * std::tan(alpha))));

		if (isLeftOf(rLower) != initiallyLeftOfOther) {
			Number<Inexact> angleDifference = std::abs(phiForR(rLower) - other.phiForR(rLower));
			if (angleDifference < M_PI / 2 || angleDifference > 3 * M_PI / 2) {
				// found intersection
				break;
			} else {
				// found wraparound, continue searching
				rUpper = rLower;
				initiallyLeftOfOther = !initiallyLeftOfOther;
			}
		}
	}

	if (isLeftOf(rLower) == initiallyLeftOfOther) {
		return std::nullopt;
	}

	// binary search
	for (int i = 0; i < 30; i++) {
		Number<Inexact> rMid = (rLower + rUpper) / 2;
		bool leftOfOther = isLeftOf(rMid);
		if (leftOfOther == initiallyLeftOfOther) {
			rUpper = rMid;
		} else {
			rLower = rMid;
		}
	}
	return (rLower + rUpper) / 2;
}

PolarSegment SweepEdgeShape::toPolarSegment() const {
	assert(m_type == Type::SEGMENT);
	return PolarSegment(m_start, *m_end);
}

SpiralSegment SweepEdgeShape::toSpiralSegment() const {
	assert(m_type == Type::LEFT_SPIRAL || m_type == Type::RIGHT_SPIRAL);
	return SpiralSegment(m_start, m_type == Type::LEFT_SPIRAL ? -m_alpha : m_alpha, nearR(),
	                     nearR() * 100);
	// TODO instead of "* 100" implement a spiral that doesn't end, or instead
	// pick a value so that the spiral rotates by 2π
}

Number<Inexact> SweepEdgeShape::signedAlpha() const {
	if (m_type == SweepEdgeShape::Type::LEFT_SPIRAL) {
		return -m_alpha;
	} else {
		return m_alpha;
	}
}

SweepEdge::SweepEdge(SweepEdgeShape shape)
    : m_shape(shape), m_previousInterval(nullptr),
      m_nextInterval(SweepInterval(SweepInterval::Type::REACHABLE)) {}

SweepEdgeShape& SweepEdge::shape() {
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

bool SweepEdge::isOnCircle() const {
	return m_onCircle;
}

} // namespace cartocrow::flow_map
