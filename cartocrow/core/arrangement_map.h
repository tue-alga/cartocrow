#pragma once

#include <CGAL/Arr_extended_dcel.h>
#include <CGAL/Arr_landmarks_point_location.h>
#include <CGAL/Arr_segment_traits_2.h>
#include <CGAL/Arrangement_2.h>
#include <CGAL/Polygon_set_2.h>
#include <CGAL/Surface_sweep_2/Arr_default_overlay_traits_base.h>

#include <concepts>
#include <filesystem>

#include "core.h"
#include "boundary_map.h"
#include "region_arrangement.h"

namespace cartocrow {

namespace detail    {
struct Void {};
}
/// An arrangement consisting of polygonal regions.
///
/// This is an \ref Arrangement, or doubly-connected edge list (DCEL)
template <typename TVertexData = detail::Void, typename TEdgeData = detail::Void,
          typename TFaceData = detail::Void>
using ArrangementMap = CGAL::Arrangement_2<
    CGAL::Arr_segment_traits_2<Exact>,
    CGAL::Arr_extended_dcel<CGAL::Arr_segment_traits_2<Exact>, TVertexData, TEdgeData, TFaceData>>;

template <typename TVertexData = detail::Void, typename TEdgeData = detail::Void,
          typename TFaceData = detail::Void>
ArrangementMap<TVertexData, TEdgeData, TFaceData>
boundaryMapToArrangementMap(BoundaryMap& map);

/// Change DCEL to have appropriate additional vertex and edge data.
template <class TVertexData = detail::Void, class TEdgeData = detail::Void>
ArrangementMap<TVertexData, TEdgeData, std::string>
regionArrangementToArrangementMap(const RegionArrangement& arr);

/// Change DCEL to forget auxiliary vertex and edge data.
template <class TVertexData, class TEdgeData>
RegionArrangement
arrangementMapToRegionArrangement(const ArrangementMap<TVertexData, TEdgeData, std::string>& arr);
} // namespace cartocrow

#include "arrangement_map.hpp"
