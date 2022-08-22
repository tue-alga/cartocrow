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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 19-02-2021
*/

#include "polar_segment.h"

#include <glog/logging.h>

namespace cartocrow::flow_map {

PolarSegment::PolarSegment(const PolarPoint& p1, const PolarPoint& p2) : PolarLine(p1, p2) {
	m_length = PolarLine::setFoot(p1, p2);
	m_start = (p1.r() * p1.r() - p2.r() * p2.r() + m_length * m_length) / (2 * m_length);
}

Number<Inexact> PolarSegment::rMin() const {
	if (containsPhi(foot().phi())) {
		return foot().r();
	}
	return std::min(pointAlongSegment(0).r(), pointAlongSegment(1).r());
}

Number<Inexact> PolarSegment::rMax() const {
	return std::max(pointAlongSegment(0).r(), pointAlongSegment(1).r());
}

bool PolarSegment::isLeftLine() const {
	return 0 < foot().r() && m_length < 0;
}

bool PolarSegment::isRightLine() const {
	return 0 < foot().r() && 0 < m_length;
}

bool PolarSegment::isCollinear() const {
	return 0 == foot().r();
}

bool PolarSegment::containsFoot() const {
	return containsPhi(foot().phi());
}

bool PolarSegment::containsR(const Number<Inexact>& R) const {
	return rMin() <= R && R <= rMax();
}

bool PolarSegment::containsPhi(const Number<Inexact>& phi) const {
	if (!PolarLine::containsPhi(phi)) {
		return false;
	}

	const Number<Inexact> t = parameterForPhi(phi);
	return t >= 0 && t <= 1;
}

PolarPoint PolarSegment::pointAlongSegment(const Number<Inexact>& t) const {
	const Number<Inexact> distance = toDistance(t);
	return PolarLine::pointAlongLine(distance);
}

Number<Inexact> PolarSegment::parameterForPhi(const Number<Inexact>& phi) const {
	return toParameter(PolarLine::distanceAlongLineForPhi(phi));
}

PolarPoint PolarSegment::closestToOrigin() const {
	if (containsPhi(foot().phi())) {
		return foot();
	}
	const PolarPoint p0 = pointAlongSegment(0);
	const PolarPoint p1 = pointAlongSegment(1);
	return p0.r() < p1.r() ? p0 : p1;
}

const PolarLine& PolarSegment::supportingLine() const {
	return *this;
}

Number<Inexact> PolarSegment::toDistance(const Number<Inexact>& t) const {
	return m_length * t - m_start;
}

Number<Inexact> PolarSegment::toParameter(const Number<Inexact>& distance) const {
	return (distance + m_start) / m_length;
}

std::ostream& operator<<(std::ostream& os, const PolarSegment& line) {
	os << "s[" << line.pointAlongSegment(0).toCartesian() << ", "
	   << line.pointAlongSegment(1).toCartesian() << "]";
	return os;
}

} // namespace cartocrow::flow_map
