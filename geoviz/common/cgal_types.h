/*
The GeoViz library implements algorithmic geo-visualization methods,
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 07-11-2019
*/

#ifndef GEOVIZ_COMMON_CGAL_TYPES_H
#define GEOVIZ_COMMON_CGAL_TYPES_H

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>

namespace geoviz
{

// The geometric data types are taken from the CGAL library where possible.
using Kernel = CGAL::Exact_predicates_inexact_constructions_kernel;
using Number = Kernel::FT;

using Point = CGAL::Point_2<Kernel>;
using Vector = CGAL::Vector_2<Kernel>;

using Box = CGAL::Bbox_2;
using Circle = CGAL::Circle_2<Kernel>;
using Line = CGAL::Line_2<Kernel>;
using Segment = CGAL::Segment_2<Kernel>;


constexpr const Number kEpsilon = 0.0000001;

} // namespace geoviz

#endif //GEOVIZ_COMMON_CGAL_TYPES_H
