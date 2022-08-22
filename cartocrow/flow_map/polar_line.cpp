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

#include "polar_line.h"

#include <cmath>
#include <optional>

namespace cartocrow::flow_map {

PolarLine::PolarLine(const PolarPoint& foot) : m_foot(foot) {}

PolarLine::PolarLine(const PolarPoint& p1, const PolarPoint& p2) {
	setFoot(p1, p2);
}

const PolarPoint& PolarLine::foot() const {
	return m_foot;
}

PolarPoint& PolarLine::foot() {
	return m_foot;
}

bool PolarLine::containsR(const Number<Inexact>& R) const {
	return foot().r() <= R;
}

bool PolarLine::containsPhi(const Number<Inexact>& phi) const {
	const Number<Inexact> phi_d = std::abs(foot().phi() - phi);
	return foot().r() == 0 || phi_d < M_PI_2 || 3 * M_PI_2 < phi_d;
}

PolarPoint PolarLine::pointAlongLine(const Number<Inexact>& t) const {
	const Number<Inexact> r = std::sqrt(t * t + foot().r() * foot().r());
	const Number<Inexact> phi_t = std::atan2(t, foot().r());
	const Number<Inexact> phi = wrapAngle(foot().phi() + phi_t);
	return PolarPoint(r, phi);
}

Number<Inexact> PolarLine::distanceAlongLineForPhi(const Number<Inexact>& phi) const {
	if (!containsPhi(phi)) {
		throw std::runtime_error("Polar line does not contain point at phi = " + std::to_string(phi));
	}
	return foot().r() * std::tan(phi - foot().phi());
}

Number<Inexact> PolarLine::distanceForPhi(const Number<Inexact>& phi) const {
	if (!containsPhi(phi)) {
		throw std::runtime_error("Polar line does not contain point at phi = " + std::to_string(phi));
	}
	return foot().r() / std::cos(phi - foot().phi());
}

std::optional<Number<Inexact>> PolarLine::tangentAngle(const Number<Inexact>& r) const {
	if (!containsR(r)) {
		return std::nullopt;
	}
	return std::asin(foot().r() / r);
}

Number<Inexact> PolarLine::setFoot(const PolarPoint& point_1, const PolarPoint& point_2) {
	const Number<Inexact> C = wrapAngle(point_2.phi() - point_1.phi());
	const int sign = /*C < std::sin(C) ? -1 : 1;*/ std::sin(C) < 0 ? -1 : 1;

	// cosine law
	const Number<Inexact> c = sign * std::sqrt(point_1.r() * point_1.r() + point_2.r() * point_2.r() -
	                                           2 * point_1.r() * point_2.r() * std::cos(C));

	const Number<Inexact> x =
	    (point_2.r() * std::sin(point_2.phi()) - point_1.r() * std::sin(point_1.phi())) / c;
	const Number<Inexact> y =
	    -(point_2.r() * std::cos(point_2.phi()) - point_1.r() * std::cos(point_1.phi())) / c;

	m_foot = PolarPoint(point_1.r() * point_2.r() * std::sin(C) / c, std::atan2(y, x));

	return c;
}

std::ostream& operator<<(std::ostream& os, const PolarLine& line) {
	os << "l[" << line.pointAlongLine(0).toCartesian() << ", "
	   << line.pointAlongLine(1).toCartesian() << "]";
	return os;
}

} // namespace cartocrow::flow_map
