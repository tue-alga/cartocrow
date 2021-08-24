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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 29-01-2020
*/

#ifndef CARTOCROW_COMMON_BOUNDING_BOX_H
#define CARTOCROW_COMMON_BOUNDING_BOX_H

#include <glog/logging.h>

#include "cartocrow/common/core_types.h"

namespace cartocrow {

inline Box GrowBoundingBox(const Box& box, const Number& buffer) {
	CHECK_GE(buffer, 0);
	return Box(box.xmin() - buffer, box.ymin() - buffer, box.xmax() + buffer, box.ymax() + buffer);
}

inline Box GrowBoundingBox(const Point& center, const Number& buffer) {
	CHECK_GE(buffer, 0);
	return Box(center.x() - buffer, center.y() - buffer, center.x() + buffer, center.y() + buffer);
}

} // namespace cartocrow

#endif //CARTOCROW_COMMON_BOUNDING_BOX_H
