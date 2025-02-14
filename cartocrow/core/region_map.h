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
 * A RegionMap can be read from an Ipe file by using \ref ipeToRegionMap().
 */
using RegionMap = std::unordered_map<std::string, Region>;

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
RegionLabel& findLabelAtCentroid(const PolygonSet<Exact>& shape,
                                 std::vector<RegionLabel>& labels);
} // namespace detail

/// Creates a \ref RegionMap from a region map in Ipe format.
///
/// The Ipe figure to be read needs to contain a single page. This page
/// has polygonal shapes (possibly containing holes or separate connected
/// components), each representing a region. Each region then needs to contain
/// exactly one label in its interior, indicating the name of the region.
///
/// Throws if the file could not be read, if the file is not a valid Ipe file,
/// or if the file does not contain regions like specified above.
RegionMap ipeToRegionMap(const std::filesystem::path& file, bool labelAtCentroid = false);

} // namespace cartocrow

#endif //CARTOCROW_CORE_REGION_MAP_H
