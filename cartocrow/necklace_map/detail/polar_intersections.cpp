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

#include "polar_intersections.h"

#include <glog/logging.h>

namespace cartocrow {
namespace detail {

// Return the side of the line that the point lies: -1: pole side, 0: on line, 1: far side.
int Orientation(const PolarLine& line, const PolarPoint& point) {
	if (!line.ContainsPhi(point.phi())) {
		return -1;
	}

	// Note that using the standardized method for computing the R at phi of the point on the line and comparing that with the given point fails for lines through the pole.
	// Instead, we project the given point onto the pedal vector and compare the distance to the foot of the line.
	const Number R_diff = point.R() * std::cos(point.phi() - line.foot().phi()) - line.foot().R();
	return R_diff < 0 ? -1 : 0 < R_diff ? 1 : 0;
}

// Search for the t of a point on the spiral in (t_spiral_near, t_spiral_far] that is within some small distance of the intersection with the line.
// Some restrictions on valid t_spiral_near and t_spiral_far:
// * The points on the spiral at t_spiral_near and t_spiral_far are not on the same side of the line.
// * the spiral intersects the line exactly once in (t_spiral_near, t_spiral_far].
bool BinarySearch(const PolarLine& line, const Spiral& spiral, Number& t_spiral_near,
                  Number& t_spiral_far, const Number t_precision /*= Number(1e-15)*/) {
	CHECK_LT(t_spiral_far, t_spiral_near);
	const int orientation_far = Orientation(line, spiral.Evaluate(t_spiral_far));
	if (orientation_far == 0) {
		t_spiral_near = t_spiral_far;
		return true;
	}
	if (Orientation(line, spiral.Evaluate(t_spiral_near)) != -orientation_far) {
		return false;
	}

	while (t_precision < t_spiral_near - t_spiral_far) {
		const Number t_mid = (t_spiral_far + t_spiral_near) / 2;

		// Safety switch
		if (t_mid == t_spiral_far || t_mid == t_spiral_near) {
			break;
		}

		const int orientation_mid = Orientation(line, spiral.Evaluate(t_mid));

		if (orientation_mid == 0) {
			t_spiral_near = t_spiral_far = t_mid;
			return true;
		} else if (orientation_mid == orientation_far) {
			t_spiral_far = t_mid;
		} else {
			t_spiral_near = t_mid;
		}
	}

	return true;
}

bool CheckIntersection(const Spiral& spiral, const PolarPoint& point) {
	return true;
}

bool CheckIntersection(const SpiralSegment& segment, const PolarPoint& point) {
	return segment.ContainsR(point.R());
}

bool CheckIntersection(const PolarLine& line, const PolarPoint& point) {
	return true;
}

bool CheckIntersection(const PolarSegment& segment, const PolarPoint& point) {
	return segment.ContainsPhi(point.phi());
}

} // namespace detail

} // namespace cartocrow
