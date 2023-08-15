// -----------------------------------------------------------------------------
// IMPLEMENTATION OF TEMPLATE FUNCTIONS
// Do not include this file, but the .h file instead
// -----------------------------------------------------------------------------

namespace cartocrow::simplification {

namespace detail {

template <EdgeCollapseTraits ECT>
ECT::Map::Halfedge_handle getMain(typename ECT::Map::Halfedge_handle e) {
	if (ECT::ecGetEdgeMark(e) == ECEdgeMark::MAIN) {
		// TODO: assert that e->twin is OTHER
		return e;
	} else {
		// TODO: assert that e is OTHER and e->twin is MAIN
		return e->twin();
	}
}

template <EdgeCollapseTraits ECT>
ECT::Map::Halfedge_handle decideMain(typename ECT::Map::Halfedge_handle e) {

	const ECEdgeMark e_mark = ECT::ecGetEdgeMark(e);
	const ECEdgeMark twin_mark = ECT::ecGetEdgeMark(e->twin());
	switch (e_mark) {
	default:
	case ECEdgeMark::NONE:
		switch (twin_mark) {
		default:
		case ECEdgeMark::NONE:
			// e becomes main
			ECT::ecSetEdgeMark(e, ECEdgeMark::MAIN);
			ECT::ecSetEdgeMark(e->twin(), ECEdgeMark::OTHER);
			return e;
		case ECEdgeMark::MAIN:
			// e becomes other
			ECT::ecSetEdgeMark(e, ECEdgeMark::OTHER);
			return e->twin();
		case ECEdgeMark::OTHER:
			// e becomes main
			ECT::ecSetEdgeMark(e, ECEdgeMark::MAIN);
			return e;
		}
	case ECEdgeMark::MAIN:
		switch (twin_mark) {
		default:
		case ECEdgeMark::NONE:
			ECT::ecSetEdgeMark(e->twin(), ECEdgeMark::OTHER);
			return e;
		case ECEdgeMark::MAIN:
			// override e-twin
			ECT::ecSetEdgeMark(e->twin(), ECEdgeMark::OTHER);
			return e;
		case ECEdgeMark::OTHER:
			return e;
		}
	case ECEdgeMark::OTHER:
		switch (twin_mark) {
		default:
		case ECEdgeMark::NONE:
			// e becomes main
			ECT::ecSetEdgeMark(e, ECEdgeMark::MAIN);
			ECT::ecSetEdgeMark(e->twin(), ECEdgeMark::OTHER);
			return e;
		case ECEdgeMark::MAIN:
			return e->twin();
		case ECEdgeMark::OTHER:
			// e becomes main
			ECT::ecSetEdgeMark(e, ECEdgeMark::MAIN);
			return e;
		}
	}
}

inline bool intersects(Segment<Exact> s, PolygonVector polygons) {
	for (Polygon<Exact> poly : polygons) {
		if (!poly.has_on_unbounded_side(s.start())) {
			return true;
		}
		for (typename Polygon<Exact>::Edge_const_iterator it = poly.edges_begin();
		     it != poly.edges_end(); it++) {
			Segment<Exact> e = *it;
			if (CGAL::intersection(e, s)) {
				return true;
			}
		}
	}

	return false;
}

inline bool intersects(Point<Exact> p, PolygonVector polygons) {
	for (Polygon<Exact> poly : polygons) {
		if (!poly.has_on_unbounded_side(p)) {
			return true;
		}
	}
	return false;
}

template <EdgeCollapseTraits ECT>
inline bool blocks(typename ECT::Map::Halfedge_handle blocking,
                   typename ECT::Map::Halfedge_handle collapsing, bool debug) {

	// TODO: assert that collapsing is a MAIN edge

	PolygonVector polygons;
	typename ECT::Map::Halfedge_handle collapse_test;
	if (debug) {
		std::cout << blocking->source()->point() << " > " << blocking->target()->point()
		          << " blocks? " << collapsing->source()->point() << " > "
		          << collapsing->target()->point() << " mark=" << (ECT::ecGetEdgeMark(collapsing) == ECEdgeMark::MAIN)
		          << "\n";
	}
	// TODO: this isn't going to work when the same face is on both sides of an edge
	if (blocking->face() == collapsing->face()) {
		// same face
		if (debug)
			std::cout << "  same\n";
		polygons = ECT::ecGetCollapse(collapsing).this_face_polygons;
		collapse_test = collapsing;
	} else {
		// other face
		if (debug)
			std::cout << "  other\n";
		polygons = ECT::ecGetCollapse(collapsing).twin_face_polygons;
		collapse_test = collapsing->twin();
	}

	if (blocking == collapse_test || blocking == collapse_test->prev() ||
	    blocking == collapse_test->next()) {
		if (debug)
			std::cout << "  near\n";
		// one of the collapsing edges
		return false;
	} else {
		bool prev_adj = blocking == collapse_test->prev()->prev();
		bool next_adj = blocking == collapse_test->next()->next();
		if (prev_adj) {
			if (next_adj) {
				if (debug)
					std::cout << "  quad\n";
				// quadrilateral, cannot block
				return false;
			} else {

				if (debug)
					std::cout << "  near prev\n";
				// TODO: what if the segment cuts through?
				return detail::intersects(blocking->source()->point(), polygons);
			}
		} else if (next_adj) {
			if (debug)
				std::cout << "  near next\n";
			// TODO: what if the segment cuts through?
			return detail::intersects(blocking->target()->point(), polygons);
		} else {
			if (debug) {

				std::cout << "  independent " << blocking->curve() << "\n";
				std::cout << "    result = " << detail::intersects(blocking->curve(), polygons) << "\n";
			}
			// independent edge

			return detail::intersects(blocking->curve(), polygons);
		}
	}
}
} // namespace detail

template <ModifiableArrangement MA, EdgeCollapseTraits ECT>
requires(std::same_as<typename MA::Map,
                      typename ECT::Map>) void EdgeCollapseSimplification<MA, ECT>::initialize() {

	// initialize each edge (nb: edge_handles does only one halfedge per edge!
	for (typename Map::Halfedge_handle e : map.edge_handles()) {
		initEdge(e);
	}
}

template <ModifiableArrangement MA, EdgeCollapseTraits ECT>
requires(std::same_as<typename MA::Map, typename ECT::Map>) void EdgeCollapseSimplification<
    MA, ECT>::simplify(Number<Exact> t) {
	// make sure nothing has been undone, i.e., the state of the arrangement
	// matches the data stored for the simplification
	if constexpr (ModifiableArrangementWithHistory<MA>) {
		modmap.goToPresent();
	}

	typename Map::Halfedge_handle best;
	Number<Exact> best_cost;

	// find and execute the operation with lowest cost while not exceeding cost t
	while (findBest(best, best_cost) && best_cost <= t) {
		execute(best, best_cost);
	}
}

template <ModifiableArrangement MA, EdgeCollapseTraits ECT>
requires(std::same_as<typename MA::Map,
                      typename ECT::Map>) void EdgeCollapseSimplification<MA, ECT>::simplify(int c) {

	// make sure nothing has been undone, i.e., the state of the arrangement
	// matches the data stored for the simplification
	if constexpr (ModifiableArrangementWithHistory<MA>) {
		modmap.goToPresent();
	}

	typename Map::Halfedge_handle best;
	Number<Exact> best_cost;

	// find and execute the operation with lowest cost while the map has more than
	// c edges
	while (map.number_of_edges() > c && findBest(best, best_cost)) {
		execute(best, best_cost);
	}
}

template <ModifiableArrangement MA, EdgeCollapseTraits ECT>
requires(std::same_as<typename MA::Map, typename ECT::Map>) void EdgeCollapseSimplification<
    MA, ECT>::initEdge(Map::Halfedge_handle e) {

	e = detail::decideMain<ECT>(e);

	if (e->source()->degree() != 2 || e->target()->degree() != 2) {
		// one of the endpoints is not degree-2
		// not collapsable
		ECT::ecSetBlockingNumber(e, NOOP_DEGREE);
		return;
	}

	if (e->next()->next()->next() == e || e->twin()->next()->next()->next() == e->twin()) {
		// triangle
		// not collapsable
		ECT::ecSetBlockingNumber(e, NOOP_TRIANGLE);
		return;
	}

	// both edgepoints are degree 2 and the faces are not triangles
	// collapsable

	const Collapse collapse = ECT::ecComputeCollapse(e);
	ECT::ecSetCollapse(e, collapse);
	ECT::ecSetCost(e);

	// initialize blocking number

	typename Map::Face_handle face = e->face();
	typename Map::Face_handle twin_face = e->twin()->face();

	// first, test floaters
	// if these block, then it will need reinitialization
	if (!collapse.this_face_polygons.empty())
		for (typename Map::Isolated_vertex_iterator it = face->isolated_vertices_begin();
		     it != face->isolated_vertices_end(); ++it) {
			if (detail::intersects(it->point(), collapse.this_face_polygons)) {

				ECT::ecSetBlockingNumber(e, BLOCKED_FLOATING);
				return;
			}
		}
	if (!collapse.twin_face_polygons.empty())
		for (typename Map::Isolated_vertex_iterator it = twin_face->isolated_vertices_begin();
		     it != twin_face->isolated_vertices_end(); ++it) {
			if (detail::intersects(it->point(), collapse.twin_face_polygons)) {

				ECT::ecSetBlockingNumber(e, BLOCKED_FLOATING);
				return;
			}
		}

	// then all edges

	int b = 0;

	// try face of e
	if (!collapse.this_face_polygons.empty()) {
		if (!face->is_unbounded()) {
			// the outer rim
			typename Map::Ccb_halfedge_circulator circ = face->outer_ccb();
			typename Map::Ccb_halfedge_circulator curr = circ;
			do {
				if (detail::blocks<ECT>(&*curr, e, false)) {
					b++;
				}
			} while (++curr != circ);
		}

		// holes
		for (typename Map::Hole_iterator h = face->holes_begin(); h != face->holes_end(); ++h) {
			typename Map::Ccb_halfedge_circulator circ = *h;
			typename Map::Ccb_halfedge_circulator curr = circ;
			do {
				if (detail::blocks<ECT>(&*curr, e, false)) {
					b++;
				}

			} while (++curr != circ);
		}
	}

	if (!collapse.twin_face_polygons.empty()) {
		if (!twin_face->is_unbounded()) {
			// the outer rim
			typename Map::Ccb_halfedge_circulator circ = twin_face->outer_ccb();
			typename Map::Ccb_halfedge_circulator curr = circ;
			do {
				if (detail::blocks<ECT>(&*curr, e, false)) {
					b++;
				}
			} while (++curr != circ);
		}

		// holes
		for (typename Map::Hole_iterator h = twin_face->holes_begin(); h != twin_face->holes_end();
		     ++h) {
			typename Map::Ccb_halfedge_circulator circ = *h;
			typename Map::Ccb_halfedge_circulator curr = circ;
			do {
				if (detail::blocks<ECT>(&*curr, e, false)) {
					b++;
				}

			} while (++curr != circ);
		}
	}

	ECT::ecSetBlockingNumber(e, b);
}

template <ModifiableArrangement MA, EdgeCollapseTraits ECT>
requires(std::same_as<typename MA::Map, typename ECT::Map>) bool EdgeCollapseSimplification<
    MA, ECT>::findBest(Map::Halfedge_handle& best, Number<Exact>& best_cost) {

	bool found = false;
	for (typename Map::Halfedge_handle e : map.edge_handles()) {

		e = detail::getMain<ECT>(e);

		if (ECT::ecGetBlockingNumber(e) == 0) {
			// performable operation
			Number<Exact> cost = ECT::ecGetCost(e);
			if (!found || cost < best_cost) {
				// beats current best
				best = e;
				best_cost = cost;
				found = true;
			}
		}
	}

	return found;
}

template <ModifiableArrangement MA, EdgeCollapseTraits ECT>
requires(std::same_as<typename MA::Map, typename ECT::Map>) void EdgeCollapseSimplification<MA, ECT>::execute(
    Map::Halfedge_handle e, Number<Exact> cost) {

	if (debug) {

		std::cout << "EXEC " << e->source()->point() << "\n";
		std::cout << "     " << e->target()->point() << "\n";
	}

	// walk over faces to reduce counts
	adjustCounts(e, -1);
	adjustCounts(e->prev(), -1);
	adjustCounts(e->next(), -1);
	adjustCounts(e->twin(), -1);
	adjustCounts(e->twin()->prev(), -1);
	adjustCounts(e->twin()->next(), -1);

	// execute
	if constexpr (ModifiableArrangementWithHistory<MA>) {
		modmap.startBatch(cost);
	}

	const Collapse col = ECT::ecGetCollapse(e);
	if (col.erase_both) {

		e = modmap.mergeWithNext(e->prev());
		e = modmap.mergeWithNext(e);
		if constexpr (ModifiableArrangementWithHistory<MA>) {
			modmap.endBatch();
		}

		// walk over faces to increase counts
		adjustCounts(e, 1);
		adjustCounts(e->twin(), 1);

	} else {
		e = modmap.mergeWithNext(e->prev());
		modmap.shift(e->target(), col.point);

		// e points towards new point

		if constexpr (ModifiableArrangementWithHistory<MA>) {
			modmap.endBatch();
		}

		if (debug) {

			std::cout << "e " << e->source()->point() << "\n";
			std::cout << "  " << e->target()->point() << "\n";
		}

		// walk over faces to increase counts
		adjustCounts(e, 1);
		adjustCounts(e->twin(), 1);
		adjustCounts(e->next(), 1);
		adjustCounts(e->twin()->prev(), 1);
	}

	// reinit neighborhood
	initEdge(e);
	initEdge(e->prev());
	initEdge(e->next());
	initEdge(e->next()->next());
}

template <ModifiableArrangement MA, EdgeCollapseTraits ECT>
requires(std::same_as<typename MA::Map, typename ECT::Map>) void EdgeCollapseSimplification<
    MA, ECT>::adjustCounts(Map::Halfedge_handle e, const int adj) {

	typename Map::Face_handle face = e->face();

	if (debug && adj > 0) {

		std::cout << "adj " << e->source()->point() << "\n";
		std::cout << "    " << e->target()->point() << "\n";

		std::cout << "    => " << adj << "\n";
	}

	if (!face->is_unbounded()) {
		// the outer rim
		typename Map::Ccb_halfedge_circulator circ = face->outer_ccb();
		typename Map::Ccb_halfedge_circulator curr = circ;
		do {
			typename Map::Halfedge_handle collapsing = detail::getMain<ECT>(&*curr);
			int b = ECT::ecGetBlockingNumber(collapsing);
			if (b >= 0 && detail::blocks<ECT>(e, collapsing, debug && adj > 0)) {
				ECT::ecSetBlockingNumber(collapsing, b + adj);
			}
		} while (++curr != circ);
	}

	// holes
	for (typename Map::Hole_iterator h = face->holes_begin(); h != face->holes_end(); ++h) {
		typename Map::Ccb_halfedge_circulator circ = *h;
		typename Map::Ccb_halfedge_circulator curr = circ;
		do {
			typename Map::Halfedge_handle collapsing = detail::getMain<ECT>(&*curr);
			int b = ECT::ecGetBlockingNumber(collapsing);
			if (b >= 0 && detail::blocks<ECT>(e, collapsing, debug && adj > 0)) {
				ECT::ecSetBlockingNumber(collapsing, b + adj);
			}

		} while (++curr != circ);
	}
}

} // namespace cartocrow::simplification
