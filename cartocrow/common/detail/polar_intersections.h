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

#ifndef CARTOCROW_COMMON_DETAIL_POLAR_INTERSECTIONS_H
#define CARTOCROW_COMMON_DETAIL_POLAR_INTERSECTIONS_H

#include "cartocrow/common/core_types.h"
#include "cartocrow/common/polar_line.h"
#include "cartocrow/common/polar_point.h"
#include "cartocrow/common/polar_segment.h"
#include "cartocrow/common/spiral.h"
#include "cartocrow/common/spiral_segment.h"


namespace cartocrow
{

template <typename OutputIterator>
int ComputeIntersectionT(const Spiral& spiral_1, const Spiral& spiral_2, OutputIterator t_1);

template <typename OutputIterator>
int ComputeIntersectionT(const PolarLine& line_1, const PolarLine& line_2, OutputIterator t_1);

template <typename OutputIterator>
int ComputeIntersectionT(const PolarLine& line, const Spiral& spiral, OutputIterator t_line);

// Note that there is no function ComputeIntersectionT(const PolarLine& line, const Spiral& spiral, OutputIterator t_line),
// because the output must be on the line to enable returning intersections on the pole.


template <class Type1_, class Type2_, typename OutputIterator>
int ComputeIntersections(const Type1_& object_1, const Type2_& object_2, OutputIterator intersections);

} // namespace cartocrow

#include "polar_intersections.inc"

#endif //CARTOCROW_COMMON_DETAIL_POLAR_INTERSECTIONS_H
