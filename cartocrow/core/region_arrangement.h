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
#include <CGAL/Surface_sweep_2/Arr_default_overlay_traits_base.h>

#include <concepts>
#include <filesystem>

#include "core.h"
#include "region_map.h"

namespace cartocrow {

namespace detail {

template <typename T> concept HasName = requires(T v) {
	{ v.name } -> std::convertible_to<std::string>;
};

struct Void {};

struct DefaultFace {
	std::string name;
};
} // namespace detail

/// An arrangement consisting of polygonal regions.
///
/// This is an \ref Arrangement where each face has an ID.
///
/// A RegionMap can be constructed from a \ref RegionMap by using \ref
/// regionMapToArrangement().
template <typename TVertexData = detail::Void, typename TEdgeData = detail::Void,
          detail::HasName TFaceData = detail::DefaultFace>
using RegionArrangement = CGAL::Arrangement_2<
    CGAL::Arr_segment_traits_2<Exact>,
    CGAL::Arr_extended_dcel<CGAL::Arr_segment_traits_2<Exact>, TVertexData, TEdgeData, TFaceData>>;

namespace detail {

template <typename V, typename E, HasName F>
using RegionOverlayTraitsBase =
    CGAL::_Arr_default_overlay_traits_base<RegionArrangement<V, E, F>, PolygonSet<Exact>::Arrangement_2,
                                           RegionArrangement<V, E, F>>;

template <typename V, typename E, HasName F>
struct RegionOverlayTraits : public RegionOverlayTraitsBase<V, E, F> {

	RegionOverlayTraits(std::string newId) : m_newId(newId) {}

	virtual void create_face(typename RegionOverlayTraitsBase<V, E, F>::Face_handle_A f1,
	                         typename RegionOverlayTraitsBase<V, E, F>::Face_handle_B f2,
	                         typename RegionOverlayTraitsBase<V, E, F>::Face_handle_R f) {
		if (f2->contained()) {
			F data = f1->data();
			if (data.name != "") {
				Point<Exact> p = f->outer_ccb()->source()->point();
				throw std::runtime_error("Found overlapping regions \"" + data.name + "\" and \"" +
				                         m_newId + "\" (at " + std::to_string(CGAL::to_double(p.x())) +
				                         ", " + std::to_string(CGAL::to_double(p.y())) + ")");
			}
			data.name = m_newId;
			f->set_data(data);
		} else {
			f->set_data(f1->data());
		}
	}

  private:
	std::string m_newId;
};
} // namespace detail

/// Creates a \ref RegionArrangement from a \ref RegionMap.
template <typename V = detail::Void, typename E = detail::Void, detail::HasName F = detail::DefaultFace>
RegionArrangement<V, E, F> regionMapToArrangement(const RegionMap& map) {
	RegionArrangement<V, E, F> arrangement;

	for (const auto& [id, region] : map) {
		const auto& regionArrangement = region.shape.arrangement();
		RegionArrangement<V, E, F> result;
		detail::RegionOverlayTraits<V, E, F> overlayTraits(id);
		CGAL::overlay(arrangement, region.shape.arrangement(), result, overlayTraits);
		arrangement = result;
	}

	return arrangement;
}

} // namespace cartocrow

#endif //CARTOCROW_CORE_REGION_ARRANGEMENT_H
