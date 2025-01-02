namespace cartocrow::simplification::util {

template <class TArr> Number<Exact> faceArea(typename TArr::Face_handle face) {
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
inline TArr::Halfedge_handle mergeWithPrev(TArr& dcel, typename TArr::Halfedge_handle edge) {
	return dcel.merge_edge(edge->prev(), edge,
	                       Segment<Exact>(edge->prev()->source()->point(), edge->target()->point()));
}

template <class TArr>
inline TArr::Halfedge_handle mergeWithNext(TArr& dcel, typename TArr::Halfedge_handle edge) {
    Segment<Exact> curve(edge->source()->point(), edge->next()->target()->point());
    if (edge->direction() == edge->next()->direction()) {
        return dcel.merge_edge(edge, edge->next(), curve);
    } else {
        auto fd1 = edge->face()->data();
        auto fd2 = edge->twin()->face()->data();
        dcel.remove_edge(edge->next());
        dcel.remove_edge(edge);
        auto he = CGAL::insert_non_intersecting_curve(dcel, curve);

        typename TArr::Traits_2 traits;
        auto equal = traits.equal_2_object();
        if (equal(curve.source(), he->source()->point())) {
            he->face()->data() = fd1;
            he->twin()->face()->data() = fd2;
        } else {
            he->face()->data() = fd2;
            he->twin()->face()->data() = fd1;
        }
        return he;
    }
}

template <class TArr>
inline void shift(TArr& dcel, typename TArr::Vertex_handle vertex, Point<Exact> pt) {
	CGAL::Arr_accessor<TArr> acc(dcel);
	// modify vertex location
	acc.modify_vertex_ex(vertex, pt);
	// modify segments of all incoming edges
	typename TArr::Halfedge_handle inc = vertex->inc();
	do {
		acc.modify_edge_ex(inc, Segment<Exact>(inc->source()->point(), pt));
		inc = inc->next()->twin();
	} while (inc != vertex->inc());
}

template <class TArr>
inline void shift(TArr& dcel, typename TArr::Halfedge_handle edge, Point<Exact> pt_source,
                  Point<Exact> pt_target) {

	CGAL::Arr_accessor<TArr> acc(dcel);

	// modify the edgge itself
	acc.modify_edge_ex(edge, Segment<Exact>(pt_source, pt_target));
	// modify the vertices
	acc.modify_vertex_ex(edge->source(), pt_source);
	acc.modify_vertex_ex(edge->target(), pt_target);
	// modify all other edges around source
	typename TArr::Halfedge_handle inc = edge->prev();
	while (inc != edge->twin()) {
		acc.modify_edge_ex(inc, Segment<Exact>(inc->source()->point(), pt_source));
		inc = inc->twin()->prev();
	}
	// modify all other edges around target
	typename TArr::Halfedge_handle out = edge->next();
	while (out != edge->twin()) {
		acc.modify_edge_ex(out, Segment<Exact>(pt_target, out->target()->point()));
		out = out->twin()->next();
	}
}

template <class TArr>
inline TArr::Halfedge_handle split(TArr& dcel, typename TArr::Halfedge_handle edge, Point<Exact> pt) {
	CGAL::Arr_accessor<TArr> acc(dcel);
	return acc.split_edge_ex(edge, pt, Segment<Exact>(edge->source()->point(), pt),
	                         Segment<Exact>(pt, edge->target()->point()));
}

} // namespace cartocrow::simplification::util