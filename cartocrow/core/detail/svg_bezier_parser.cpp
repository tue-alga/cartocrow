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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 13-05-2020
*/

#include "svg_bezier_parser.h"

#include <glog/logging.h>

namespace cartocrow {
namespace detail {

/**@class SvgBezierConverter
 * @brief An implementation of SvgPathConverter for converting an SVG path element to a Bezier spline.
 *
 * A path that uses the move command while the spline is not closed will result in a fatal error.
 */

/**@fn SvgBezierConverter::BezierNecklace
 * @brief A Bezier spline.
 */

/**@brief Construct an object for converting SVG path elements to a Bezier spline.
 * @param shape the output Bezier spline.
 */
SvgBezierConverter::SvgBezierConverter(BezierSpline& shape) : SvgPathConverter(), shape_(shape) {}

void SvgBezierConverter::MoveTo_(const Point& to) {
	CHECK(shape_.IsEmpty() || shape_.IsClosed()) << "Trying to move while spline is not closed.";
	source_ = to;
}

void SvgBezierConverter::LineTo_(const Point& to) {
	const Point midpoint = CGAL::midpoint(source_, to);
	shape_.AppendCurve(source_, midpoint, midpoint, to);
	source_ = to;
}

void SvgBezierConverter::QuadBezierTo_(const Point& control, const Point& to) {
	shape_.AppendCurve(source_, control, control, to);
	source_ = to;
}

void SvgBezierConverter::CubeBezierTo_(const Point& control_1, const Point& control_2,
                                       const Point& to) {
	shape_.AppendCurve(source_, control_1, control_2, to);
	source_ = to;
}

void SvgBezierConverter::Close_() {
	CHECK(shape_.IsValid());
}

} // namespace detail
} // namespace cartocrow
