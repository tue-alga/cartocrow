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
#include "cartocrow/core/core.h"

namespace cartocrow::flow_map {

PolarPoint::PolarPoint() : m_r(0), m_phi(0) {}

PolarPoint::PolarPoint(const CGAL::Origin& o) : m_r(0), m_phi(0) {}

PolarPoint::PolarPoint(const Number<Inexact>& r, const Number<Inexact>& phi) : m_r(r), m_phi(phi) {
	if (r < 0) {
		throw std::runtime_error("Tried to construct a polar point with r < 0");
	}
	m_phi = wrapAngle(m_phi, -M_PI);
}

PolarPoint::PolarPoint(const PolarPoint& p) : m_r(p.r()), m_phi(p.phi()) {}

PolarPoint::PolarPoint(const PolarPoint& p, const Vector<Inexact>& t)
    : PolarPoint(translate(p, t)) {}

PolarPoint::PolarPoint(const Point<Inexact>& p) : PolarPoint(toPolar(p)) {}

PolarPoint::PolarPoint(const Point<Inexact>& p, const Vector<Inexact>& t)
    : PolarPoint(toPolar(p + t)) {}

const Number<Inexact> PolarPoint::r() const {
	return m_r;
}

const Number<Inexact> PolarPoint::rSquared() const {
	return m_r * m_r;
}

const Number<Inexact> PolarPoint::phi() const {
	return m_phi;
}

void PolarPoint::setR(Number<Inexact> r) {
	m_r = r;
}

void PolarPoint::setPhi(Number<Inexact> phi) {
	m_phi = phi;
}

Point<Inexact> PolarPoint::toCartesian() const {
	const Vector<Inexact> d = Vector<Inexact>(std::cos(phi()), std::sin(phi()));
	return Point<Inexact>(CGAL::ORIGIN) + r() * d;
}

PolarPoint PolarPoint::toPolar(const Point<Inexact>& p) {
	const Number<Inexact> r = CGAL::sqrt((p - Point<Inexact>(CGAL::ORIGIN)).squared_length());

	if (p.x() == 0 && p.y() == 0) {
		return PolarPoint(r, 0);
	}

	const Number<Inexact> phi = std::atan2(CGAL::to_double(p.y()), CGAL::to_double(p.x()));
	return PolarPoint(r, phi);
}

PolarPoint PolarPoint::translate(const PolarPoint& p, const Vector<Inexact>& t) {
	return toPolar(p.toCartesian() + t);
}

bool operator==(const PolarPoint& p, const PolarPoint& q) {
	return p.r() == q.r() && (p.r() == 0 || p.phi() == q.phi());
}

bool operator!=(const PolarPoint& p, const PolarPoint& q) {
	return !(p == q);
}

std::ostream& operator<<(std::ostream& os, const PolarPoint& point) {
	os << "(R=" << point.r() << ", φ=" << point.phi() << ")";
	return os;
}

} // namespace cartocrow::flow_map
