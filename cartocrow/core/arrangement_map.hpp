// -----------------------------------------------------------------------------
// IMPLEMENTATION OF TEMPLATE FUNCTIONS
// Do not include this file, but the .h file instead
// -----------------------------------------------------------------------------

namespace cartocrow {

template <typename TVertexData, typename TEdgeData, typename TFaceData>
void boundaryMapToArrangementMap(BoundaryMap& map,
                                 ArrangementMap<TVertexData, TEdgeData, TFaceData>& arr) {

	using Arr = ArrangementMap<TVertexData, TEdgeData, TFaceData>;

	CGAL::Arr_walk_along_line_point_location<Map> loc(arr);
	using PLResult = CGAL::Arr_point_location_result<Map>::Type;

	for (Boundary& b : map.boundaries) {

		// NB: we're assuming here that boundaries don't share vertices except common endpoints

		if (b.closed) {
			auto face = loc.locate(b.points[0]);
			Arr::Face_const_handle* f = boost::get<Arr::Face_const_handle>(&face);
			Arr::Halfedge_handle first =
			    arr.insert_in_face_interior(Segment<Exact>(b.points[0], b.points[1]), f->ptr());

			bool fwd = b.points[0] < b.points[1];

			Arr::Vertex_handle prev = fwd ? first->target() : first->source();
			for (int i = 2; i < p.size(); ++i) {

				if (b.points[i - 1] < b.points[i]) {
					Arr::Halfedge_handle curr = arr.insert_from_left_vertex(
					    Segment<Exact>(b.points[i - 1], b.points[i]), prev);
					prev = curr->target();
				} else {
					Arr::Halfedge_handle curr = arr.insert_from_right_vertex(
					    Segment<Exact>(b.points[i - 1], b.points[i]), prev);
					prev = curr->target();
				}
			}

			arr.insert_at_vertices(Segment(b.points[b.points.size() - 1], b.points[0]), prev,
			                       fwd ? first->source() : first->target());

		} else {

			Arr::Vertex_handle v0;
			{
				PLResult res = loc.locate(b.points[0]);

				Arr::Vertex_const_handle* v = boost::get<Arr::Vertex_const_handle>(&res);

				if (v) {
					v0 = arr.non_const_handle(*v);
				} else {
					Arr::Face_const_handle* f = boost::get<Arr::Face_const_handle>(&res);
					v0 = arr.insert_in_face_interior(b.points[0], f->ptr());
				}
			}

			Arr::Vertex_handle vn;
			{
				PLResult res = loc.locate(b.points[b.points.size() - 1]);
				Arr::Vertex_const_handle* v = boost::get<Arr::Vertex_const_handle>(&res);
				if (v) {
					vn = map->non_const_handle(*v);
				} else {
					Arr::Face_const_handle* f = boost::get<Arr::Face_const_handle>(&res);
					vn = arr.insert_in_face_interior(b.points[b.points.size() - 1], f->ptr());
				}
			}

			Arr::Vertex_handle prev = v0;

			for (int i = 1; i < b.points.size() - 1; ++i) {
				if (b.points[i - 1] < b.points[i]) {
					Arr::Halfedge_handle curr = arr.insert_from_left_vertex(
					    Segment<Exact>(b.points[i - 1], b.points[i]), prev);
					prev = curr->target();
				} else {
					Arr::Halfedge_handle curr = arr.insert_from_right_vertex(
					    Segment<Exact>(b.points[i - 1], b.points[i]), prev);
					prev = curr->target();
				}
			}

			arr.insert_at_vertices(
			    Segment<Exact>(b.points[b.points.size() - 2], b.points[b.points.size() - 1]), prev,
			    vn);
		}
	}
}

} // namespace cartocrow