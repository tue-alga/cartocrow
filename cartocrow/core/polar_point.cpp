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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 04-09-2020
*/

#include "polar_point.h"

#include <glog/logging.h>

namespace cartocrow {

PolarPoint::PolarPoint() : m_r(0), m_phi(0) {}

PolarPoint::PolarPoint(const CGAL::Origin& o) : m_r(0), m_phi(0) {}

PolarPoint::PolarPoint(const Number& r, const Number& phi) : m_r(r), m_phi(phi) {
	CHECK_LE(0, r);

	// TODO [ws] this is stupid
	while (m_phi < -M_PI) {
		m_phi += M_2xPI;
	}
	while (M_PI <= m_phi) {
		m_phi -= M_2xPI;
	}
}

PolarPoint::PolarPoint(const PolarPoint& p) : m_r(p.R()), m_phi(p.phi()) {}

/**@brief Construct a polar point from a polar point with a different pole.
 *
 * @param p the reference polar point.
 * @param t the Cartesian coordinates of p's pole (relative to the pole of the point to construct)
 */
PolarPoint::PolarPoint(const PolarPoint& p, const Vector& t) : PolarPoint(translate_pole(p, t)) {}

PolarPoint::PolarPoint(const Point& p) : PolarPoint(to_polar(p)) {}

/**@brief Construct a polar point with a different pole.
 *
 * @param p the Cartesian coordinates of the polar point.
 * @param t the Cartesian coordinates of p's pole (relative to the pole of the point to construct)
 */
PolarPoint::PolarPoint(const Point& p, const Vector& t) : PolarPoint(to_polar(p + t)) {}

const Number& PolarPoint::R() const {
	return m_r;
}

const Number& PolarPoint::phi() const {
	return m_phi;
}

Point PolarPoint::to_cartesian() const {
	const Vector d = Vector(std::cos(phi()), std::sin(phi()));
	return Point(CGAL::ORIGIN) + R() * d;
}

PolarPoint PolarPoint::to_polar(const Point& p) {
	const Number r = CGAL::sqrt((p - Point(CGAL::ORIGIN)).squared_length());

	if (p.x() == 0 && p.y() == 0) {
		return PolarPoint(r, 0);
	}

	const Number phi = std::atan2(CGAL::to_double(p.y()), CGAL::to_double(p.x()));
	return PolarPoint(r, phi);
}

PolarPoint PolarPoint::translate_pole(const PolarPoint& p, const Vector& t) {
	return to_polar(p.to_cartesian() + t);
}

bool operator==(const PolarPoint& p, const PolarPoint& q) {
	return p.R() == q.R() && (p.R() == 0 || p.phi() == q.phi());
}

bool operator!=(const PolarPoint& p, const PolarPoint& q) {
	return !(p == q);
}

std::ostream& operator<<(std::ostream& os, const PolarPoint& point) {
	os << "(R=" << point.R() << ", φ=" << point.phi() << ")";
	return os;
}

} // namespace cartocrow
