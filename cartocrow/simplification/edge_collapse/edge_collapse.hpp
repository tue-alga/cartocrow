// -----------------------------------------------------------------------------
// IMPLEMENTATION OF TEMPLATE FUNCTIONS
// Do not include this file, but the .h file instead
// -----------------------------------------------------------------------------

namespace cartocrow::simplification {

namespace detail {

template <EdgeCollapseTraits ECT>
ECT::Map::Halfedge_handle getMain(typename ECT::Map::Halfedge_handle e) {
	if (ECT::ecGetEdgeMark(e) == ECEdgeMark::MAIN) {
		assert(ECT::ecGetEdgeMark(e->twin()) == ECEdgeMark::OTHER);
		return e;
	} else {
		assert(ECT::ecGetEdgeMark(e) == ECEdgeMark::OTHER);
		assert(ECT::ecGetEdgeMark(e->twin()) == ECEdgeMark::MAIN);
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

	Point<Exact> ref = s.start();

	for (Polygon<Exact> poly : polygons) {
		if (!poly.has_on_unbounded_side(ref)) {
			// ref point lies on boundary or inside, so intersection guaranteed
			return true;
		}

		// since ref point lies outside, it must intersect an edge,
		// if it is to intersect at all
		for (typename Polygon<Exact>::Edge_const_iterator it = poly.edges_begin();
		     it != poly.edges_end(); it++) {
			Segment<Exact> e = *it;
			auto intersect = CGAL::intersection(e, s);
			if (intersect) {
				return true;
			}
		}
	}

	return false;
}

inline bool intersects(Segment<Exact> s, PolygonVector polygons, Point<Exact> ignore) {

	Point<Exact> ref = s.start() + (s.end() - s.start()) / 2.0; // midpoint

	for (Polygon<Exact> poly : polygons) {
		if (!poly.has_on_unbounded_side(ref)) {
			// mid point lies on boundary or inside, so intersection guaranteed
			return true;
		}

		// since mid point lies outside, it must intersect an edge,
		// if it is to intersect at all
		for (typename Polygon<Exact>::Edge_const_iterator it = poly.edges_begin();
		     it != poly.edges_end(); it++) {
			Segment<Exact> e = *it;
			auto intersect = CGAL::intersection(e, s);
			if (!intersect) {
				// no intersection at all
				continue;
			} else if (boost::get<Segment<Exact>>(&*intersect)) {
				// proper overlap, must include nonendpoint part
				return true;
			} else {
				// only other option: point intersection, make sure it's not the
				// excluded point
				Point<Exact> p = boost::get<Point<Exact>>(intersect.get());
				if (p != ignore) {
					return true;
				}
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
                   typename ECT::Map::Halfedge_handle collapsing) {

	assert(ECT::ecGetEdgeMark(collapsing) == ECEdgeMark::MAIN);

	if (collapsing->face() == collapsing->twin()->face()) {
		// same face on both sides, need to check more extensively
		assert(blocking->face() == collapsing->face());

		// this is effectively the same as below, but we need to account for the
		// blocking edge possibly interacting with the collapse from both sides
		// NB: we could just always run this bit of code, also for the other cases
		// but it is rather redundant in its checks
		if (blocking == collapsing || blocking == collapsing->prev() ||
		    blocking == collapsing->next() || blocking == collapsing->twin() ||
		    blocking == collapsing->twin()->prev() || blocking == collapsing->twin()->next()) {
			// one of the collapsing edges
			return false;
		} else {
			// NB: collapsing's vertices are degree 2
			// NB: pp == tnn edge, for deg-2 nodes, and different if there's a deg-3 node
			// so we need to handle only one of the following four nearby-edge cases
			// a quadrilateral isn't possible, as it would imply different faces

			if (blocking == collapsing->prev()->prev() ||
			    blocking == collapsing->twin()->prev()->prev()) {
				return detail::intersects(blocking->curve(),
				                          ECT::ecGetCollapse(collapsing).this_face_polygons,
				                          blocking->target()->point()) ||
				       detail::intersects(blocking->curve(),
				                          ECT::ecGetCollapse(collapsing).twin_face_polygons,
				                          blocking->target()->point());
			} else if (blocking == collapsing->next()->next()) {
				return detail::intersects(blocking->curve(),
				                          ECT::ecGetCollapse(collapsing).this_face_polygons,
				                          blocking->source()->point()) ||
				       detail::intersects(blocking->curve(),
				                          ECT::ecGetCollapse(collapsing).twin_face_polygons,
				                          blocking->source()->point());
			} else {
				// independent edge
				return detail::intersects(blocking->curve(),
				                          ECT::ecGetCollapse(collapsing).this_face_polygons) ||
				       detail::intersects(blocking->curve(),
				                          ECT::ecGetCollapse(collapsing).twin_face_polygons);
			}
		}

	} else {
		// different faces on both sides of the collapse

		PolygonVector polygons;
		typename ECT::Map::Halfedge_handle collapse_test;

		if (blocking->face() == collapsing->face()) {
			// same face
			polygons = ECT::ecGetCollapse(collapsing).this_face_polygons;
			collapse_test = collapsing;
		} else {
			// other face
			assert(blocking->face() == collapsing->twin()->face());
			polygons = ECT::ecGetCollapse(collapsing).twin_face_polygons;
			collapse_test = collapsing->twin();
		}

		if (blocking == collapse_test || blocking == collapse_test->prev() ||
		    blocking == collapse_test->next()) {
			// one of the collapsing edges
			return false;
		} else {
			bool prev_adj = blocking == collapse_test->prev()->prev();
			bool next_adj = blocking == collapse_test->next()->next();
			if (prev_adj) {
				if (next_adj) {
					// quadrilateral, cannot block
					return false;
				} else {
					return detail::intersects(blocking->curve(), polygons,
					                          blocking->target()->point());
				}
			} else if (next_adj) {
				return detail::intersects(blocking->curve(), polygons, blocking->source()->point());
			} else {
				// independent edge
				return detail::intersects(blocking->curve(), polygons);
			}
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
				if (detail::blocks<ECT>(&*curr, e)) {
					b++;
				}
			} while (++curr != circ);
		}

		// holes
		for (typename Map::Hole_iterator h = face->holes_begin(); h != face->holes_end(); ++h) {
			typename Map::Ccb_halfedge_circulator circ = *h;
			typename Map::Ccb_halfedge_circulator curr = circ;
			do {
				if (detail::blocks<ECT>(&*curr, e)) {
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
				if (detail::blocks<ECT>(&*curr, e)) {
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
				if (detail::blocks<ECT>(&*curr, e)) {
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

	if (!face->is_unbounded()) {
		// the outer rim
		typename Map::Ccb_halfedge_circulator circ = face->outer_ccb();
		typename Map::Ccb_halfedge_circulator curr = circ;
		do {
			typename Map::Halfedge_handle collapsing = detail::getMain<ECT>(&*curr);
			int b = ECT::ecGetBlockingNumber(collapsing);
			if (b >= 0 && detail::blocks<ECT>(e, collapsing)) {
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
			if (b >= 0 && detail::blocks<ECT>(e, collapsing)) {
				ECT::ecSetBlockingNumber(collapsing, b + adj);
			}

		} while (++curr != circ);
	}
}

} // namespace cartocrow::simplification
