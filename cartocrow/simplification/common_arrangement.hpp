// -----------------------------------------------------------------------------
// IMPLEMENTATION OF TEMPLATE FUNCTIONS
// Do not include this file, but the .h file instead
// -----------------------------------------------------------------------------

namespace cartocrow::simplification {

template <class TArr> Number<Exact> face_area(typename TArr::Face_handle face) {
	Number<Exact> total = 0;

	if (!face->is_unbounded()) {
		// the outer rim
		typename TArr::Ccb_halfedge_circulator circ = face->outer_ccb();
		typename TArr::Ccb_halfedge_circulator curr = circ;
		do {
			typename TArr::Vertex_handle u = curr->source();
			typename TArr::Vertex_handle v = curr->target();

			total += CGAL::determinant(u->point() - CGAL::ORIGIN, v->point() - CGAL::ORIGIN);

		} while (++curr != circ);
	}

	// holes
	for (typename TArr::Hole_iterator h = face->holes_begin(); h != face->holes_end(); ++h) {
		typename TArr::Ccb_halfedge_circulator circ = *h;
		typename TArr::Ccb_halfedge_circulator curr = circ;
		do {
			typename TArr::Vertex_handle u = curr->source();
			typename TArr::Vertex_handle v = curr->target();

			total += CGAL::determinant(u->point() - CGAL::ORIGIN, v->point() - CGAL::ORIGIN);
			// NB: holes are CW so area is subtracted

		} while (++curr != circ);
	}

	// outerface will have negative area, being the total of its holes
	return total / 2.0;
}

template <class TArr>
inline TArr::Halfedge_handle merge_with_prev(TArr& dcel, typename TArr::Halfedge_handle edge) {
	return dcel.merge_edge(edge->prev(), edge,
	                       Segment<Exact>(edge->prev()->source()->point(), edge->target()->point()));
}

template <class TArr>
inline TArr::Halfedge_handle merge_with_next(TArr& dcel, typename TArr::Halfedge_handle edge) {
	return dcel.merge_edge(edge, edge->next(),
	                       Segment<Exact>(edge->source()->point(), edge->next()->target()->point()));
}

template <class TArr>
inline void shift(TArr& dcel, typename TArr::Vertex_handle vertex, Point<Exact> pt) {
	CGAL::Arr_accessor<TArr> acc(dcel);
	acc.modify_vertex_ex(vertex, pt);
}

template <class TArr>
inline void shift(TArr& dcel, typename TArr::Halfedge_handle edge, Point<Exact> pt_source,
                  Point<Exact> pt_target) {
	CGAL::Arr_accessor<TArr> acc(dcel);
	acc.modify_edge_ex(edge, Segment<Exact>(pt_source, pt_target));
}

template <class TArr>
inline void shift_source(TArr& dcel, typename TArr::Halfedge_handle edge, Point<Exact> pt) {
	CGAL::Arr_accessor<TArr> acc(dcel);
	acc.modify_edge_ex(edge, Segment<Exact>(pt, edge->target()->point()));
}

template <class TArr>
inline void shift_target(TArr& dcel, typename TArr::Halfedge_handle edge, Point<Exact> pt) {
	CGAL::Arr_accessor<TArr> acc(dcel);
	acc.modify_edge_ex(edge, Segment<Exact>(edge->source()->point(), pt));
}

template <class TArr>
inline TArr::Halfedge_handle split(TArr& dcel, typename TArr::Halfedge_handle edge, Point<Exact> pt) {
	CGAL::Arr_accessor<TArr> acc(dcel);
	return acc.split_edge_ex(edge, pt, Segment<Exact>(edge->source()->point(), pt),
	                         Segment<Exact>(pt, edge->target()->point()));
}

} // namespace cartocrow::simplification