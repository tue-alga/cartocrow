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

namespace cartocrow {

/**@class SpiralSegment
 * @brief A logarithmic spiral segment.
 *
 * The polar coordinates of the points on the spiral segment p(t) = (R(t), phi(t)) are R(t) = R(0)*e^{-t}, and phi(t) = phi(0) + tan(alpha) * t, where p(0) = (R(0), phi(0)) is the position of the spiral segment's anchor.
 *
 * The special case where the angle is 0, this spiral segment is a straight line segment instead.
 */

/**@brief Construct a logarithmic spiral segment that connects two points.
 *
 * A logarithmic spiral segment cannot connect two points that are equidistant from the pole. For any other combination of points, an infinite number of spiral segments exist that connect them. This constructors the shortest spiral segment connecting the two points.
 *
 * This will construct a straight line spiral segment if the source and target are collinear with the pole.
 *
 * The input point farthest from the pole will become the point on the logarithmic spiral segment at t = 0.
 * @param point_1 one of the endpoints of the logarithmic spiral segment.
 * @param point_2 the other endpoint on the logarithmic spiral segment.
 */
SpiralSegment::SpiralSegment(const PolarPoint& point_1, const PolarPoint& point_2)
    : Spiral(point_1, point_2), R_min_(point_1.R()), R_max_(point_2.R()) {
	CHECK_LE(0, R_min_);
	if (R_max_ < R_min_) {
		std::swap(R_min_, R_max_);
	}
}

/**@brief Construct a logarithmic spiral segment.
 * @param far @parblock the far endpoint of the logarithmic spiral segment.
 * This will become the point on the spiral at t = 0.
 * This point cannot be the pole, because then the spiral could not be determined uniquely.
 * @endparblock
 * @param angle_rad the alpha used to compute the phi coordinate of a point on the logarithmic spiral.
 * @param R_min @parblock the R of the near endpoint of the logarithmic spiral segment.
 * This cannot be larger than the R of the far point.
 * @endparblock
 */
SpiralSegment::SpiralSegment(const PolarPoint& far, const Number& angle_rad, const Number& R_min)
    : Spiral(far, angle_rad), R_min_(R_min), R_max_(far.R()) {
	CHECK_LE(0, R_min_);
	CHECK_LE(R_min_, R_max_);
}

/**@brief Construct a logarithmic spiral segment.
 *
 * Note that the anchor need not be either endpoint of the logarithmic spiral segment.
 * @param anchor a point on the supporting logarithmic spiral.
 * @param angle_rad the alpha used to compute the phi coordinate of a point on the logarithmic spiral.
 * @param R_min the R of the near endpoint of the logarithmic spiral segment.
 * @param R_max the R of the far endpoint of the logarithmic spiral segment. This cannot be smaller than R_min.
 */
SpiralSegment::SpiralSegment(const PolarPoint& anchor, const Number& angle_rad, const Number& R_min,
                             const Number& R_max)
    : Spiral(anchor, angle_rad), R_min_(R_min), R_max_(R_max) {
	CHECK_LE(0, R_min_);
	CHECK_LE(R_min_, R_max_);
}

/**@brief Access the far endpoint of the logarithmic spiral segment.
 * @return the far endpoint of the logarithmic spiral segment.
 */
const PolarPoint SpiralSegment::far() const {
	return PolarPoint(R_max(), ComputePhi(R_max()));
}

/**@brief Access the near endpoint of the logarithmic spiral segment.
 * @return the near endpoint of the logarithmic spiral segment.
 */
const PolarPoint SpiralSegment::near() const {
	return PolarPoint(R_min(), ComputePhi(R_min()));
}

/**@brief Access the smallest R on the logarithmic spiral segment.
 * @return the R of the near endpoint of the logarithmic spiral segment.
 */
const Number& SpiralSegment::R_min() const {
	return R_min_;
}

/**@brief Access the largest R on the logarithmic spiral segment.
 * @return the R of the far endpoint of the logarithmic spiral segment.
 */
const Number& SpiralSegment::R_max() const {
	return R_max_;
}

/**@brief Check whether the supporting logarithmic spiral evaluated at a given time falls within the segment.
 * @param t the time at which to evaluate the supporting logarithmic spiral.
 * @return whether the segment contains the evaluated logarithmic spiral.
 */
bool SpiralSegment::ContainsT(const Number& t) const {
	return ContainsR(EvaluateR(t));
}

/**@brief Check whether the logarithmic spiral segment contains a point at a given distance from the pole.
 * @param R the distance from the pole to check.
 * @return whether the segment contains a point at a given distance.
 */
bool SpiralSegment::ContainsR(const Number& R) const {
	return R_min() <= R && R <= R_max();
}

std::ostream& operator<<(std::ostream& os, const SpiralSegment& segment) {
	os << "S<@= " << segment.anchor() << ", ang= " << segment.angle_rad()
	   << ", min_R= " << segment.R_min() << ">";
	return os;
}

} // namespace cartocrow
