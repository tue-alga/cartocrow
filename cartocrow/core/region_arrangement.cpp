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

#include <CGAL/Arr_overlay_2.h>
#include <CGAL/Surface_sweep_2/Arr_default_overlay_traits_base.h>
#include <CGAL/draw_polygon_set_2.h>
#include <CGAL/number_utils.h>

namespace cartocrow {

namespace detail {
struct RegionOverlayTraits
    : public CGAL::_Arr_default_overlay_traits_base<RegionArrangement, PolygonSet<Exact>::Arrangement_2,
                                                    RegionArrangement> {

	RegionOverlayTraits(std::string newId) : m_newId(newId) {}

	virtual void create_face(Face_handle_A f1, Face_handle_B f2, Face_handle_R f) {
		if (f2->contained()) {
			std::string id = f1->data();
			if (f1->data() != "") {
				Point<Exact> p = f->outer_ccb()->source()->point();
				throw std::runtime_error("Found overlapping regions \"" + f1->data() + "\" and \"" +
				                         m_newId + "\" (at " + std::to_string(CGAL::to_double(p.x())) +
				                         ", " + std::to_string(CGAL::to_double(p.y())) + ")");
			}
			f->set_data(m_newId);
		} else {
			f->set_data(f1->data());
		}
	}

  private:
	std::string m_newId;
};
} // namespace detail

RegionArrangement regionMapToArrangement(const RegionMap& map) {
	RegionArrangement arrangement;

	for (const auto& [id, region] : map) {
		const auto& regionArrangement = region.shape.arrangement();
		RegionArrangement result;
		detail::RegionOverlayTraits overlayTraits(id);
		CGAL::overlay(arrangement, region.shape.arrangement(), result, overlayTraits);
		arrangement = result;
	}

	return arrangement;
}

} // namespace cartocrow
