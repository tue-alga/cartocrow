#pragma once

#include "../core/core.h"

namespace cartocrow::simplification {

template <class TDCEL, class TFace> Number<Exact> face_area(TDCEL& dcel, TFace face) {
	Number<Exact> total = 0;

	if (!face->is_unbounded()) {
		// the outer rim
		typename TDCEL::Ccb_halfedge_circulator circ = face->outer_ccb();
		typename TDCEL::Ccb_halfedge_circulator curr = circ;
		do {
			typename TDCEL::Vertex_handle u = curr->source();
			typename TDCEL::Vertex_handle v = curr->target();

			total += CGAL::determinant(u->point() - CGAL::ORIGIN, v->point() - CGAL::ORIGIN);

		} while (++curr != circ);
	}

	// holes
	for (typename TDCEL::Hole_iterator h = face->holes_begin(); h != face->holes_end(); ++h) {
		typename TDCEL::Ccb_halfedge_circulator circ = *h;
		typename TDCEL::Ccb_halfedge_circulator curr = circ;
		do {
			typename TDCEL::Vertex_handle u = curr->source();
			typename TDCEL::Vertex_handle v = curr->target();

			total += CGAL::determinant(u->point() - CGAL::ORIGIN, v->point() - CGAL::ORIGIN);
			// NB: holes are CW so area is subtracted

		} while (++curr != circ);
	}

	// outerface will have negative area, being the total of its holes
	return total / 2.0;
}

template <class TDCEL, class THalfedge>
inline THalfedge merge_with_prev(TDCEL& dcel, THalfedge edge) {
	return dcel.merge_edge(edge->prev(), edge,
	                       Segment(edge->prev()->source()->point(), edge->target()->point()));
}

template <class TDCEL, class THalfedge>
inline THalfedge merge_with_next(TDCEL& dcel, THalfedge edge) {
	return dcel.merge_edge(edge, edge->next(),
	                       Segment<Exact>(edge->source()->point(), edge->next()->target()->point()));
}

template <class TDCEL, class TVertex> inline void shift(TDCEL& dcel, TVertex vertex, Point<Exact> pt) {
	CGAL::Arr_accessor<TDCEL> acc(dcel);
	acc.modify_vertex_ex(vertex, pt);
}

template <class TDCEL, class THalfedge>
inline void shift(TDCEL& dcel, THalfedge edge, Point<Exact> pt_source, Point<Exact> pt_target) {
	CGAL::Arr_accessor<TDCEL> acc(dcel);
	acc.modify_edge_ex(edge, Segment<Exact>(pt_source, pt_target));
}

template <class TDCEL, class THalfedge>
inline void shift_source(TDCEL& dcel, THalfedge edge, Point<Exact> pt) {
	CGAL::Arr_accessor<TDCEL> acc(dcel);
	acc.modify_edge_ex(edge, Segment<Exact>(pt, edge->target()->point()));
}

template <class TDCEL, class THalfedge>
inline void shift_target(TDCEL& dcel, THalfedge edge, Point<Exact> pt) {
	CGAL::Arr_accessor<TDCEL> acc(dcel);
	acc.modify_edge_ex(edge, Segment<Exact>(edge->source()->point(), pt));
}

// NB: returns the incoming edge of the new point in the same direction as the halfedge
template <class TDCEL, class THalfedge>
inline THalfedge split(TDCEL& dcel, THalfedge edge, Point<Exact> pt) {
	CGAL::Arr_accessor<TDCEL> acc(dcel);
	return acc.split_edge_ex(edge, pt, Segment(edge->source()->point(), pt),
	                         Segment(pt, edge->target()->point()));
}

template <class TDCEL, class THalfedge> inline Vector<Exact> direction(THalfedge edge) {
	return edge->target()->point() - edge->source()->point();
}

template <class THalfedge> inline Vector<Exact> normalized_direction(THalfedge edge) {
	return normalized(edge->target()->point() - edge->source()->point());
}

template <class THalfedge>
inline Vector<Exact> normalized_direction(THalfedge edge, Number<Exact>* dist) {
	return normalized(edge->target()->point() - edge->source()->point(), dist);
}

} // namespace cartocrow::simplification