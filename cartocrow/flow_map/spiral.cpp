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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 16-10-2020
*/

#include "spiral.h"

#include <cmath>
#include <stdexcept>

namespace cartocrow::flow_map {

Spiral::Spiral(const PolarPoint& anchor, const Number<Inexact>& angle)
    : m_anchor(anchor), m_angle(angle) {
	if (anchor.r() == 0) {
		throw std::runtime_error("Tried to construct a spiral with the root as its anchor");
	}
}

Spiral::Spiral(const PolarPoint& p1, const PolarPoint& p2)
    : m_anchor(p1.r() < p2.r() ? p2 : p1), m_angle(alpha(p1, p2)) {}

Number<Inexact> Spiral::alpha(const PolarPoint& p1, const PolarPoint& p2) {
	const PolarPoint& source = p1.r() < p2.r() ? p1 : p2;
	const PolarPoint& target = p1.r() < p2.r() ? p2 : p1;
	if (source.r() == target.r()) {
		throw std::runtime_error(
		    "Cannot compute α for a spiral connecting two points equidistant to the root");
	}
	if (source.r() == 0) {
		return 0;
	}
	Number<Inexact> diff_phi = wrapAngle(target.phi() - source.phi(), -M_PI);
	return std::atan(diff_phi / -std::log(target.r() / source.r()));
}

Number<Inexact> Spiral::dAlphaDPhi1(const PolarPoint& p1, const PolarPoint& p2) {
	const PolarPoint& source = p1.r() < p2.r() ? p1 : p2;
	const PolarPoint& target = p1.r() < p2.r() ? p2 : p1;
	if (source.r() == 0) {
		return 0;
	}
	Number<Inexact> phiDiff = wrapAngle(target.phi() - source.phi(), -M_PI);
	Number<Inexact> rDiffLog = std::log(target.r() / source.r());
	return rDiffLog / (rDiffLog * rDiffLog + phiDiff * phiDiff);
}

Number<Inexact> Spiral::dAlphaDPhi2(const PolarPoint& p1, const PolarPoint& p2) {
	return -dAlphaDPhi1(p1, p2);
}

Number<Inexact> Spiral::dAlphaDR1(const PolarPoint& p1, const PolarPoint& p2) {
	const PolarPoint& source = p1.r() < p2.r() ? p1 : p2;
	const PolarPoint& target = p1.r() < p2.r() ? p2 : p1;
	if (source.r() == 0) {
		return 0;
	}
	Number<Inexact> phiDiff = wrapAngle(target.phi() - source.phi(), -M_PI);
	Number<Inexact> rDiffLog = std::log(target.r() / source.r());
	return (-phiDiff / p1.r()) / (rDiffLog * rDiffLog + phiDiff * phiDiff);
}

Number<Inexact> Spiral::dAlphaDR2(const PolarPoint& p1, const PolarPoint& p2) {
	const PolarPoint& source = p1.r() < p2.r() ? p1 : p2;
	const PolarPoint& target = p1.r() < p2.r() ? p2 : p1;
	if (source.r() == 0) {
		return 0;
	}
	Number<Inexact> phiDiff = wrapAngle(target.phi() - source.phi(), -M_PI);
	Number<Inexact> rDiffLog = std::log(target.r() / source.r());
	return (phiDiff / p2.r()) / (rDiffLog * rDiffLog + phiDiff * phiDiff);
}

const PolarPoint& Spiral::anchor() const {
	return m_anchor;
}

const Number<Inexact>& Spiral::angle() const {
	return m_angle;
}

bool Spiral::isLeft() const {
	return m_angle < 0;
}

bool Spiral::isRight() const {
	return m_angle > 0;
}

bool Spiral::isSegment() const {
	return m_angle == 0;
}

PolarPoint Spiral::evaluate(const Number<Inexact>& t) const {
	return PolarPoint(m_anchor.r() * std::exp(-t), wrapAngle(m_anchor.phi() + std::tan(m_angle) * t));
}

Number<Inexact> Spiral::parameterForR(const Number<Inexact>& r) const {
	if (r <= 0) {
		throw std::runtime_error("A spiral never reaches points with r <= 0");
	}
	return -std::log(r / m_anchor.r());
}

Number<Inexact> Spiral::phiForR(const Number<Inexact>& r) const {
	const Number<Inexact> t = parameterForR(r);
	return evaluate(t).phi();
}

Number<Inexact> Spiral::parameterForPhi(const Number<Inexact>& phi) const {
	return wrapAngle(phi - m_anchor.phi(), -M_PI) / std::tan(m_angle);
}

Number<Inexact> Spiral::period() const {
	return M_2xPI / std::tan(m_angle);
}

void Spiral::moveAnchor(const Number<Inexact>& r) {
	const Number<Inexact> phi = phiForR(r);
	m_anchor = PolarPoint(r, phi);
}

std::ostream& operator<<(std::ostream& os, const Spiral& spiral) {
	os << "S<@= " << spiral.anchor() << ", ang= " << spiral.angle() << ">";
	return os;
}

} // namespace cartocrow::flow_map
