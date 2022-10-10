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
		assert(phiCount <= 1); // line segment contained two of the given phis
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

bool SweepEdgeShape::departsToLeftOf(const SweepEdgeShape& shape) const {
	Number<Inexact> r = this->nearR();
	assert(shape.nearR() == r);

	// TODO temporary implementation with epsilon, should be replaced by angle
	// computations
	return PolarSegment(this->evalForR(r + 0.00001), shape.evalForR(r + 0.00001)).isLeftLine();
}

std::optional<Number<Inexact>> SweepEdgeShape::intersectWith(const SweepEdgeShape& other,
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
		initiallyLeftOfOther = departsToLeftOf(other);
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
	/*std::cout << "at rLower " << isLeftOf(rLower) << " " << evalForR(rLower) << " "
	          << other.evalForR(rLower) << std::endl;
	std::cout << "at rLower + 0.01 " << isLeftOf(rLower + 0.01) << " " << evalForR(rLower + 0.01)
	          << " " << other.evalForR(rLower + 0.01) << std::endl;*/
	Number<Inexact> rUpper = rLower;
	Number<Inexact> rLimit = std::min(rMax, rMin * std::exp(2 * M_PI / std::tan(alpha)));
	while (rUpper < rLimit) {
		rUpper = std::min(rMax, rUpper * std::exp(M_PI / (8 * std::tan(alpha))));
		//std::cout << "interval: (" << rLower << ", " << rUpper << "]" << std::endl;

		if (isLeftOf(rUpper) != initiallyLeftOfOther) {
			Number<Inexact> angleDifference = std::abs(phiForR(rUpper) - other.phiForR(rUpper));
			//std::cout << angleDifference << std::endl;
			if (angleDifference < M_PI / 2 || angleDifference > 3 * M_PI / 2) {
				// found intersection
				break;
			} else {
				// found wraparound, continue searching
				//std::cout << "wraparound" << std::endl;
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
		//std::cout << "interval: (" << rLower << ", " << rUpper << "]" << std::endl;
		Number<Inexact> rMid = (rLower + rUpper) / 2;

		bool leftOfOther = isLeftOf(rMid);
		//std::cout << "midpoint: " << rMid << " " << leftOfOther << std::endl;

		if (leftOfOther == initiallyLeftOfOther) {
			rLower = rMid;
		} else {
			rUpper = rMid;
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
