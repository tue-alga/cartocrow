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

#ifndef CARTOCROW_CORE_REGION_MAP_H
#define CARTOCROW_CORE_REGION_MAP_H

#include <CGAL/Arr_extended_dcel.h>
#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/Polygon_set_2.h>

#include <filesystem>

#include "core.h"

namespace cartocrow {

namespace detail {
using DCELTraits = CGAL::Arr_segment_traits_2<Exact>;

template <typename TVertexData, typename TEdgeData, typename TFaceData>
using MapArrangement =
    CGAL::Arrangement_2<CGAL::Arr_extended_dcel<DCELTraits, TVertexData, TEdgeData, TFaceData>>;
} // namespace detail

/// The data describing a single region in a region map.
class Region {
  public:
	/// The name of the region.
	std::string name;
	/// The color of the region, used for drawing it.
	Color color;
	/// The shape of the region, as a set of polygons.
	PolygonSet<Exact> shape;
};

/// A map consisting of polygonal regions.
/**
 * A RegionList can be read from an Ipe file by using \ref ipeToRegionList().
 */
using RegionList = std::vector<Region>;

namespace detail {
/// Storage for a label in the input map.
struct RegionLabel {
	/// Position of the label.
	Point<Exact> position;
	/// The label text.
	std::string text;
	/// Whether we have already matched this label to a region.
	bool matched;
};
/// Returns the label from \c labels inside the given region consisting of
/// \c polygons.
std::optional<size_t> findLabelInside(const PolygonSet<Exact>& shape,
                                      const std::vector<RegionLabel>& labels);
} // namespace detail

/// Creates a \ref RegionList from a region map in Ipe format.
RegionList ipeToRegionList(const std::filesystem::path& file);

class Empty {};

/// A map consisting of polygonal regions.
/**
 * This is an \ref Arrangement where each face has \ref RegionData.
 *
 * A RegionMap can be read from an Ipe file by using \ref readIpeRegionMap().
 */
using RegionArrangement = detail::MapArrangement<Empty, Empty, Region>;

} // namespace cartocrow

#endif //CARTOCROW_CORE_REGION_MAP_H