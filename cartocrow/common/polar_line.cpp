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

#include <glog/logging.h>

namespace cartocrow {

/**@class PolarLine
 * @brief A straight line with polar point coordinates.
 */

/**@brief Construct a line with polar coordinates.
 * @param closest the point on the line that is closest to the pole.
 */
PolarLine::PolarLine(const PolarPoint& closest) : foot_(closest) {}

/**@brief Construct a line containing two polar points.
 * @param point_1 one of the points on the line.
 * @param point_2 the other point on the line.
 */
PolarLine::PolarLine(const PolarPoint& point_1, const PolarPoint& point_2) {
	SetFoot(point_1, point_2);
}

/**@brief Access the foot.
 *
 * The foot is the point on the line closest to the pole.
 * @return the foot.
 */
const PolarPoint& PolarLine::foot() const { return foot_; }

/**@brief Access the foot.
 *
 * The foot is the point on the line closest to the pole.
 * @return the foot.
 */
PolarPoint& PolarLine::foot() { return foot_; }

/**@brief Check whether the line contains a point at a given distance from the pole.
 * @param R the given distance from the pole.
 * @return whether the line has any part within the desired distance from the pole.
 */
bool PolarLine::ContainsR(const Number& R) const { return foot().R() <= R; }

/**@brief Check whether the line contains any point with a given phi coordinate.
 * @param phi the desired phi coordinate.
 * @return whether the line contains any point with the given phi coordinate.
 */
bool PolarLine::ContainsPhi(const Number& phi) const {
	const Number phi_d = std::abs(foot().phi() - phi);
	return foot().R() == 0 || phi_d < M_PI_2 || 3 * M_PI_2 < phi_d;
}

/**@brief Evaluate the distance between a point on the line and the pole.
 * @param t the time value of the point on the line. This is effectively the signed distance between the point and the foot in counter-clockwise direction.
 * @return the distance from the pole at the evaluated point.
 */
Number PolarLine::EvaluateR(const Number& t) const {
	return std::sqrt(t * t + foot().R() * foot().R());
}

/**@brief Evaluate the phi of a point on the line.
 * @param t the time value of the point on the line. This is effectively the signed distance between the point and the foot in counter-clockwise direction.
 * @return the phi of the evaluated point.
 */
Number PolarLine::EvaluatePhi(const Number& t) const {
	const Number phi_t = std::atan2(t, foot().R());
	return Modulo(foot().phi() + phi_t);
}

/**@brief Evaluate a point on the line and the pole.
 * @param t the time value of the point on the line. This is effectively the signed distance between the point and the foot in counter-clockwise direction.
 * @return the evaluated point.
 */
PolarPoint PolarLine::Evaluate(const Number& t) const {
	return PolarPoint(EvaluateR(t), EvaluatePhi(t));
}

/**@brief Compute the time value of a point on the line at a given phi.
 *
 * Note, throws an exception if phi is not contained in the line.
 * @param phi the phi of the point.
 * @return the time value of the point on the line. This is effectively the signed distance between the point and the foot in counter-clockwise direction.
 */
Number PolarLine::ComputeT(const Number& phi) const {
	CHECK(ContainsPhi(phi));
	return foot().R() * std::tan(phi - foot().phi());
}

/**@brief Compute the distance to the pole of a point on the line at a given phi.
 *
 * Note, throws an exception if phi is not contained in the line.
 * @param phi the phi of the point.
 * @return the distance to the pole of the point on the line.
 */
Number PolarLine::ComputeR(const Number& phi) const {
	CHECK(ContainsPhi(phi));
	return foot().R() / std::cos(phi - foot().phi());
}

/**@brief Compute the angle the line makes with the direction towards the pole at a given distance from the pole.
 * @param R the desired distance from the pole.
 * @param angle_rad the angle (in radians) between the line and the line through the pole and a point on the line at the given distance from the pole.
 * @return whether the line contains any such point at the desired distance from the pole.
 */
bool PolarLine::ComputeAngle(const Number& R, Number& angle_rad) const {
	CHECK_LE(0, R);

	// Compute the angle at a given from the pole.
	// Note that the segment is not guaranteed to come close enough to the pole.
	// Otherwise, the smaller angle is returned.

	if (R < foot().R())
		return false;

	angle_rad = std::asin(foot().R() / R);
	return true;
}

Number PolarLine::SetFoot(const PolarPoint& point_1, const PolarPoint& point_2) {
	const Number C = Modulo(point_2.phi() - point_1.phi());
	const int sign = /*C < std::sin(C) ? -1 : 1;*/ std::sin(C) < 0 ? -1 : 1;

	// Cosine law.
	const Number c = sign * std::sqrt(point_1.R() * point_1.R() + point_2.R() * point_2.R() -
	                                  2 * point_1.R() * point_2.R() * std::cos(C));

	const Number x =
	    (point_2.R() * std::sin(point_2.phi()) - point_1.R() * std::sin(point_1.phi())) / c;
	const Number y =
	    -(point_2.R() * std::cos(point_2.phi()) - point_1.R() * std::cos(point_1.phi())) / c;

	foot_ = PolarPoint(point_1.R() * point_2.R() * std::sin(C) / c, std::atan2(y, x));

	return c;
}

std::ostream& operator<<(std::ostream& os, const PolarLine& line) {
	os << "l[" << line.Evaluate(0).to_cartesian() << ", " << line.Evaluate(1).to_cartesian() << "]";
	return os;
}

} //namespace cartocrow
