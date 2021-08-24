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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 20-11-2019
*/

#include "svg_polygon_parser.h"

#include <glog/logging.h>

namespace cartocrow {
namespace detail {

/**@class SvgPolygonConverter
 * @brief An implementation of SvgPathConverter for converting an SVG path element to a collection of polygons.
 *
 * Only closed polygons are stored.
 *
 * A path that uses the move command while the last polygon is not closed will result in a fatal error.
 */

/**@fn SvgPolygonConverter::PolygonSet
 * @brief A collection of polygons.
 */

/**@brief Construct an object for converting SVG path elements to collections of polygons.
 * @param shape the output polygon shape.
 */
SvgPolygonConverter::SvgPolygonConverter(PolygonSet& shape) : SvgPathConverter(), shape_(shape) {}

void SvgPolygonConverter::MoveTo_(const Point& to) {
	CHECK(current_.is_empty()) << "Trying to move while polygon is not closed.";
	current_.push_back(to);
}

void SvgPolygonConverter::LineTo_(const Point& to) {
	current_.push_back(to);
}

void SvgPolygonConverter::Close_() {
	CHECK(current_.is_simple());
	if (current_.is_clockwise_oriented())
		current_.reverse_orientation();
	shape_.emplace_back(current_);
	current_.clear();
}

} // namespace detail
} // namespace cartocrow
