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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 26-02-2021
*/

#include "spiral_segment.h"

#include <cmath>

#include <glog/logging.h>
#include <stdexcept>

namespace cartocrow::flow_map {

SpiralSegment::SpiralSegment(const PolarPoint& point_1, const PolarPoint& point_2)
    : Spiral(point_1, point_2), m_rMin(point_1.r()), m_rMax(point_2.r()) {
	if (m_rMax < m_rMin) {
		std::swap(m_rMin, m_rMax);
	}
}

SpiralSegment::SpiralSegment(const PolarPoint& far, const Number<Inexact>& angle,
                             const Number<Inexact>& rMin)
    : Spiral(far, angle), m_rMin(rMin), m_rMax(far.r()) {
	if (m_rMin > m_rMax) {
		throw std::runtime_error("Tried to construct a spiral segment with rMin > far.r()");
	}
}

SpiralSegment::SpiralSegment(const PolarPoint& anchor, const Number<Inexact>& angle,
                             const Number<Inexact>& rMin, const Number<Inexact>& rMax)
    : Spiral(anchor, angle), m_rMin(rMin), m_rMax(rMax) {
	if (m_rMin > m_rMax) {
		throw std::runtime_error("Tried to construct a spiral segment with rMin > rMax");
	}
}

const PolarPoint& SpiralSegment::anchor() const {
	return m_anchor;
}

const Number<Inexact>& SpiralSegment::angle() const {
	return m_angle;
}

PolarPoint SpiralSegment::far() const {
	return PolarPoint(rMax(), phiForR(rMax()));
}

PolarPoint SpiralSegment::near() const {
	return PolarPoint(rMax(), phiForR(rMin()));
}

const Number<Inexact>& SpiralSegment::rMin() const {
	return m_rMin;
}

const Number<Inexact>& SpiralSegment::rMax() const {
	return m_rMax;
}

bool SpiralSegment::containsParameter(const Number<Inexact>& t) const {
	return containsR(evaluate(t).r());
}

bool SpiralSegment::containsR(const Number<Inexact>& r) const {
	return rMin() <= r && r <= rMax();
}

const Spiral& SpiralSegment::supportingSpiral() const {
	return *this;
}

std::ostream& operator<<(std::ostream& os, const SpiralSegment& segment) {
	os << "S<@= " << segment.anchor() << ", ang= " << segment.angle()
	   << ", rMin= " << segment.rMin() << ", rMax= " << segment.rMax() << ">";
	return os;
}

} // namespace cartocrow::flow_map
