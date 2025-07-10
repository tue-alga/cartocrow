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
*/

#ifndef CARTOCROW_CORE_REGION_ARRANGEMENT_H
#define CARTOCROW_CORE_REGION_ARRANGEMENT_H

#include <CGAL/Arr_extended_dcel.h>
#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/Polygon_set_2.h>

#include <filesystem>

#include "core.h"
#include "region_map.h"

namespace cartocrow {

/// An arrangement consisting of polygonal regions.
///
/// This is an \ref Arrangement where each face has an ID.
///
/// A RegionMap can be constructed from a \ref RegionMap by using \ref
/// regionMapToArrangement().
using RegionArrangement =
    CGAL::Arrangement_2<CGAL::Arr_segment_traits_2<Exact>,
                        CGAL::Arr_face_extended_dcel<CGAL::Arr_segment_traits_2<Exact>, std::string>>;

/// Creates a \ref RegionArrangement from a \ref RegionMap.
RegionArrangement regionMapToArrangement(const RegionMap& map);

/// A simple parallel implementation for converting a region map to a region arrangement.
RegionArrangement regionMapToArrangementParallel(const RegionMap& map);

} // namespace cartocrow

#endif //CARTOCROW_CORE_REGION_ARRANGEMENT_H