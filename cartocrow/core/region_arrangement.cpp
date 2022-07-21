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

#include "region_arrangement.h"

namespace cartocrow {

RegionArrangement regionMapToArrangement(const RegionMap& map) {
	RegionArrangement arrangement;

	for (const auto& [id, region] : map) {
		std::vector<PolygonWithHoles<Exact>> polygons;
		region.shape.polygons_with_holes(std::back_inserter(polygons));
		Vector<Exact> sum(0, 0);
		Number<Exact> areaSum = 0;
		for (const PolygonWithHoles<Exact>& p : polygons) {
			const Polygon<Exact>& outside = p.outer_boundary();
			for (auto e = outside.edges_begin(); e < outside.edges_end(); ++e) {
				CGAL::insert(arrangement, Segment<Exact>(e->source(), e->target()));
			}
			// TODO handle holes, insert id
		}
	}

	return arrangement;
}

} // namespace cartocrow
