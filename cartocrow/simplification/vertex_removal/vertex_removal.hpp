// -----------------------------------------------------------------------------
// IMPLEMENTATION OF TEMPLATE FUNCTIONS
// Do not include this file, but the .h file instead
// -----------------------------------------------------------------------------

namespace cartocrow::simplification {

template <ModifiableArrangement MA, VertexRemovalTraits VRT>
requires(std::same_as<typename MA::Map,
                      typename VRT::Map>) void VertexRemovalSimplification<MA, VRT>::initialize() {

	// initialize each vertex
	for (typename Map::Vertex_const_iterator v = map.vertices_begin(); v != map.vertices_end(); ++v) {
		initVertex(&*v);
	}
}

template <ModifiableArrangement MA, VertexRemovalTraits VRT>
requires(std::same_as<typename MA::Map, typename VRT::Map>) void VertexRemovalSimplification<
    MA, VRT>::simplify(Number<Exact> t) {
	// make sure nothing has been undone, i.e., the state of the arrangement
	// matches the data stored for the simplification
	if constexpr (ModifiableArrangementWithHistory<MA>) {
		modmap.goToPresent();
	}

	typename Map::Vertex_handle best;
	Number<Exact> best_cost;

	// find and execute the operation with lowest cost while not exceeding cost t
	while (findBest(best, best_cost) && best_cost <= t) {
		execute(best, best_cost);
	}
}

template <ModifiableArrangement MA, VertexRemovalTraits VRT>
requires(std::same_as<typename MA::Map,
                      typename VRT::Map>) void VertexRemovalSimplification<MA, VRT>::simplify(int c) {

	// make sure nothing has been undone, i.e., the state of the arrangement
	// matches the data stored for the simplification
	if constexpr (ModifiableArrangementWithHistory<MA>) {
		modmap.goToPresent();
	}

	typename Map::Vertex_handle best;
	Number<Exact> best_cost;

	// find and execute the operation with lowest cost while the map has more than
	// c edges
	while (map.number_of_edges() > c && findBest(best, best_cost)) {
		execute(best, best_cost);
	}
}

template <ModifiableArrangement MA, VertexRemovalTraits VRT>
requires(std::same_as<typename MA::Map, typename VRT::Map>) void VertexRemovalSimplification<
    MA, VRT>::initVertex(Map::Vertex_handle v) {
	if (v->degree() == 2) {
		auto inc = v->incident_halfedges();

		// make sure we get the incoming edge on the face for which this vertex is
		// convex
		typename Map::Halfedge_handle realinc;
		if (CGAL::right_turn(inc->source()->point(), v->point(), inc->next()->target()->point())) {
			realinc = inc->next()->twin();
		} else {
			realinc = inc;
		}

		if (realinc->next()->next()->next() == realinc) {
			// triangle
			// not removable
			VRT::vrSetBlockingNumber(v, NOOP_TRIANGLE);
		} else {
			// degree 2, but no triangle
			// removable

			VRT::vrSetHalfedge(v, realinc);
			Triangle<Exact> T = triangle(v);
			VRT::vrSetCost(v, T);

			typename Map::Face_handle face = realinc->face();

			typename Map::Vertex_handle p = realinc->source();
			typename Map::Vertex_handle n = realinc->next()->target();

			// initialize blocking number
			int b = 0;

			// free floating vertices: if this blocks, then it will need reinitialization
			for (typename Map::Isolated_vertex_iterator it = face->isolated_vertices_begin();
			     it != face->isolated_vertices_end(); ++it) {
				if (T.has_on_bounded_side(it->point())) {
					b = BLOCKED_FLOATING;
					break;
				}
			}

			if (b >= 0) {
				// otherwise, count blocking points

				if (!face->is_unbounded()) {
					// the outer rim
					typename Map::Ccb_halfedge_circulator circ = face->outer_ccb();
					typename Map::Ccb_halfedge_circulator curr = circ;
					do {
						typename Map::Vertex_handle e_src = curr->source();
						if (e_src != p && e_src != v && e_src != n &&
						    T.has_on_bounded_side(e_src->point())) {
							b++;
						}
					} while (++curr != circ);
				}

				// holes
				for (typename Map::Hole_iterator h = face->holes_begin(); h != face->holes_end();
				     ++h) {
					typename Map::Ccb_halfedge_circulator circ = *h;
					typename Map::Ccb_halfedge_circulator curr = circ;
					do {
						typename Map::Vertex_handle e_src = curr->source();
						if (e_src != p && e_src != v && e_src != n &&
						    T.has_on_bounded_side(e_src->point())) {
							b++;
						}

					} while (++curr != circ);
				}
			}

			VRT::vrSetBlockingNumber(v, b);
		}

	} else {
		// degree != 2
		// not removable
		VRT::vrSetBlockingNumber(v, NOOP_DEGREE);
	}
}

template <ModifiableArrangement MA, VertexRemovalTraits VRT>
requires(std::same_as<typename MA::Map, typename VRT::Map>) bool VertexRemovalSimplification<
    MA, VRT>::findBest(Map::Vertex_handle& best, Number<Exact>& best_cost) {

	bool found = false;
	for (typename Map::Vertex_handle v : map.vertex_handles()) {

		if (VRT::vrGetBlockingNumber(v) == 0) {
			// performable operation
			Number<Exact> cost = VRT::vrGetCost(v);
			if (!found || cost < best_cost) {
				// beats current best
				best = v;
				best_cost = cost;
				found = true;
			}
		}
	}
	return found;
}

template <ModifiableArrangement MA, VertexRemovalTraits VRT>
requires(std::same_as<typename MA::Map, typename VRT::Map>) void VertexRemovalSimplification<
    MA, VRT>::execute(Map::Vertex_handle v, Number<Exact> cost) {
	// walk over twin face to reduce counts
	reduceCounts(v);

	// execute
	if constexpr (ModifiableArrangementWithHistory<MA>) {
		modmap.startBatch(cost);
	}

	typename Map::Halfedge_handle e = modmap.mergeWithNext(VRT::vrGetHalfedge(v));

	if constexpr (ModifiableArrangementWithHistory<MA>) {
		modmap.endBatch();
	}

	// reinit neighbors
	initVertex(e->source());
	initVertex(e->target());

	// NB: in case of an island, both cases may trigger on the same node...
	// really doesn't matter much since it will be a triangle and thus solved in O(1) time.
	if (e->next()->target() == e->prev()->source()) {
		// constructed a triangle on this side
		initVertex(e->next()->target());
	}

	if (e->twin()->next()->target() == e->twin()->prev()->source()) {
		// constructed a triangle on the other side
		initVertex(e->twin()->next()->target());
	}
}

template <ModifiableArrangement MA, VertexRemovalTraits VRT>
requires(std::same_as<typename MA::Map, typename VRT::Map>) void VertexRemovalSimplification<
    MA, VRT>::reduceCounts(Map::Vertex_handle v) {
	typename Map::Face_handle face = VRT::vrGetHalfedge(v)->twin()->face();

	Point<Exact> pt = v->point();

	if (!face->is_unbounded()) {
		// the outer rim
		typename Map::Ccb_halfedge_circulator circ = face->outer_ccb();
		typename Map::Ccb_halfedge_circulator curr = circ;
		do {
			// NB: may also reduce v and its neighbors, but this doesn't matter
			typename Map::Vertex_handle w = curr->target();
			if (VRT::vrGetBlockingNumber(w) >= 0 && triangle(w).has_on_bounded_side(pt)) {
				VRT::vrSetBlockingNumber(w, VRT::vrGetBlockingNumber(w) - 1);
			}
		} while (++curr != circ);
	}

	// holes
	for (typename Map::Hole_iterator h = face->holes_begin(); h != face->holes_end(); ++h) {
		typename Map::Ccb_halfedge_circulator circ = *h;
		typename Map::Ccb_halfedge_circulator curr = circ;
		do {
			// NB: may also reduce v and its neighbors, but this doesn't matter
			typename Map::Vertex_handle w = curr->target();
			if (VRT::vrGetBlockingNumber(w) >= 0 && triangle(w).has_on_bounded_side(pt)) {
				VRT::vrSetBlockingNumber(w, VRT::vrGetBlockingNumber(w) - 1);
			}

		} while (++curr != circ);
	}
}

template <ModifiableArrangement MA, VertexRemovalTraits VRT>
requires(std::same_as<typename MA::Map, typename VRT::Map>)
    Triangle<Exact> VertexRemovalSimplification<MA, VRT>::triangle(Map::Vertex_handle v) {
	typename Map::Halfedge_handle inc = VRT::vrGetHalfedge(v);
	// NB: the vertex provided is always the 2nd vertex
	return Triangle<Exact>(inc->source()->point(), v->point(), inc->next()->target()->point());
}

} // namespace cartocrow::simplification
