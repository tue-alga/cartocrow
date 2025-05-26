// -----------------------------------------------------------------------------
// IMPLEMENTATION OF TEMPLATE FUNCTIONS
// Do not include this file, but the .h file instead
// -----------------------------------------------------------------------------

namespace cartocrow {

template <typename TVertexData, typename TEdgeData,
          typename TFaceData>
ArrangementMap<TVertexData, TEdgeData, TFaceData>
boundaryMapToArrangementMap(BoundaryMap& map) {

	using Arr = ArrangementMap<TVertexData, TEdgeData, TFaceData>;

	Arr arr;

//	CGAL::Arr_walk_along_line_point_location<Arr> loc(arr);
    CGAL::Arr_landmarks_point_location<Arr> loc(arr);
	using PLResult = typename CGAL::Arr_point_location_result<Arr>::Type;

	for (Boundary& b : map.boundaries) {

		// NB: we're assuming here that boundaries don't share vertices except common endpoints

		if (b.closed) {
			auto face = loc.locate(b.points[0]);
			typename Arr::Face_const_handle* f = boost::get<typename Arr::Face_const_handle>(&face);
			typename Arr::Halfedge_handle first =
			    arr.insert_in_face_interior(Segment<Exact>(b.points[0], b.points[1]), f->ptr());

			bool fwd = b.points[0] < b.points[1];

			typename Arr::Vertex_handle prev = fwd ? first->target() : first->source();
			for (int i = 2; i < b.points.size(); ++i) {

				if (b.points[i - 1] < b.points[i]) {
					typename Arr::Halfedge_handle curr = arr.insert_from_left_vertex(
					    Segment<Exact>(b.points[i - 1], b.points[i]), prev);
					prev = curr->target();
				} else {
					typename Arr::Halfedge_handle curr = arr.insert_from_right_vertex(
					    Segment<Exact>(b.points[i - 1], b.points[i]), prev);
					prev = curr->target();
				}
			}

			arr.insert_at_vertices(Segment<Exact>(b.points[b.points.size() - 1], b.points[0]), prev,
			                       fwd ? first->source() : first->target());

		} else {

			typename Arr::Vertex_handle v0;
			{
				PLResult res = loc.locate(b.points[0]);

				typename Arr::Vertex_const_handle* v = boost::get<typename Arr::Vertex_const_handle>(&res);

				if (v) {
					v0 = arr.non_const_handle(*v);
				} else {
					typename Arr::Face_const_handle* f = boost::get<typename Arr::Face_const_handle>(&res);
					v0 = arr.insert_in_face_interior(b.points[0], f->ptr());
				}
			}

			typename Arr::Vertex_handle vn;
			{
				PLResult res = loc.locate(b.points[b.points.size() - 1]);
				typename Arr::Vertex_const_handle* v = boost::get<typename Arr::Vertex_const_handle>(&res);
				if (v) {
					vn = arr.non_const_handle(*v);
				} else {
					typename Arr::Face_const_handle* f = boost::get<typename Arr::Face_const_handle>(&res);
					vn = arr.insert_in_face_interior(b.points[b.points.size() - 1], f->ptr());
				}
			}

			typename Arr::Vertex_handle prev = v0;

			for (int i = 1; i < b.points.size() - 1; ++i) {
				if (b.points[i - 1] < b.points[i]) {
					typename Arr::Halfedge_handle curr = arr.insert_from_left_vertex(
					    Segment<Exact>(b.points[i - 1], b.points[i]), prev);
					prev = curr->target();
				} else {
					typename Arr::Halfedge_handle curr = arr.insert_from_right_vertex(
					    Segment<Exact>(b.points[i - 1], b.points[i]), prev);
					prev = curr->target();
				}
			}

			arr.insert_at_vertices(
			    Segment<Exact>(b.points[b.points.size() - 2], b.points[b.points.size() - 1]), prev,
			    vn);
		}
	}

	return arr;
}

template <class TVertexData, class TEdgeData>
ArrangementMap<TVertexData, TEdgeData, std::string>
regionArrangementToArrangementMap(const RegionArrangement& arr) {
    using Map = ArrangementMap<TVertexData, TEdgeData, std::string>;

    struct PickRegion {
        std::string operator()(const std::string& region1, const std::string& region2) const {
            if (!region1.empty() && !region2.empty()) {
                std::cerr << "Overlapping regions! " << region1 << " and " << region2 << std::endl;
                return region1;
            }
            if (region1.empty()) {
                return region2;
            } else {
                return region1;
            }
        }
    };

    using OverlayTraits = CGAL::Arr_face_overlay_traits<RegionArrangement, Map,
            Map, PickRegion>;

    OverlayTraits overlayTraits;

    Map result;
    Map map;
    CGAL::overlay(arr, map, result, overlayTraits);

    return result;
}

template <class TVertexData, class TEdgeData>
RegionArrangement
arrangementMapToRegionArrangement(const ArrangementMap<TVertexData, TEdgeData, std::string>& arr) {
    using Map = ArrangementMap<TVertexData, TEdgeData, std::string>;

    struct PickRegion {
        std::string operator()(const std::string& region1, const std::string& region2) const {
            if (!region1.empty() && !region2.empty()) {
                std::cerr << "Overlapping regions! " << region1 << " and " << region2 << std::endl;
                return region1;
            }
            if (region1.empty()) {
                return region2;
            } else {
                return region1;
            }
        }
    };

    using OverlayTraits = CGAL::Arr_face_overlay_traits<Map, RegionArrangement,
            RegionArrangement, PickRegion>;

    OverlayTraits overlayTraits;

    RegionArrangement result;
    RegionArrangement map;
    CGAL::overlay(arr, map, result, overlayTraits);

    return result;
}
} // namespace cartocrow