#pragma once

#include "../../core/core.h"
#include "../../core/region_arrangement.h"
#include "../common_arrangement.h"
#include "../common_traits.h"
#include "../historic_arrangement.h"

/// Algorithm to iteratively remove vertices from a map.
///
/// Implementation notes
///	  Runs in O(n^2) time
///	  This is a topologically-safe variant, using the principle of blocking numbers in https://doi.org/10.1145/2818373
///   It abstracts from the way on how to store the cost of removing a vertex, its relevant edge, and the blocking number, via the \ref VertexRemovalTraits concept.
///   Each iteration, all vertices are polled for their cost: there is no specific need to store these via the vrSetCost function

namespace cartocrow::simplification {

template <class T>
concept VertexRemovalTraits = requires(typename T::Map::Vertex_handle v, int b, Triangle<Exact> t,
                                       typename T::Map::Halfedge_handle e) {

	requires MapType<T>;

	// can get and set the cost
	// NB: this does not have to be stored necessarily, can also be derived each time GetCost is called
	{T::vrSetCost(v, t)};
	{ T::vrGetCost(v) } -> std::same_as<Number<Exact>>;

	// can store and retrieve an integer, this should be stored
	{T::vrSetBlockingNumber(v, b)};
	{ T::vrGetBlockingNumber(v) } -> std::same_as<int>;

	// can get and set incoming halfedge on the convex side
	// NB: this does not have to be stored necessarily, can also be derived each time GetHalfedge is called
	{T::vrSetHalfedge(v, e)};
	{ T::vrGetHalfedge(v) } -> std::same_as<typename T::Map::Halfedge_handle>;
};

template <ModifiableArrangement MA, VertexRemovalTraits VRT>
requires(std::same_as<typename MA::Map, typename VRT::Map>) class VertexRemovalSimplification {

  public:
	using Map = MA::Map;

	// Constructs the algorithm for a given \ref ModifiableArrangement
	VertexRemovalSimplification(MA& ma) : map(ma.getMap()), modmap(ma){};

	// Initializes the algorithm
	void initialize();

	// Simplifies while operations cost at most threshold t
	void simplify(Number<Exact> t);
	// Simplifies until the number of edges in the arrangement is at most c
	void simplify(int c);

  private:
	// Reinitializes the simplification data for the vertex
	void initVertex(Map::Vertex_handle v);
	// Finds the vertex to remove with the lowest cost. Returns true iff removable vertex was found
	// NB: assigns best and best_cost
	bool findBest(Map::Vertex_handle& best, Number<Exact>& best_cost);
	// Executes the removal of the given vertex
	void execute(Map::Vertex_handle v, Number<Exact> cost);
	// Removes v from all blocking counters
	void reduceCounts(Map::Vertex_handle v);

	// Computes the triangle spanned by v and its two neighbors
	Triangle<Exact> triangle(Map::Vertex_handle v);

	// Flags for indicating why a vertex cannot be removed
	const short BLOCKED_FLOATING = -1;
	const short NOOP_TRIANGLE = -2;
	const short NOOP_DEGREE = -3;

	// Fields
	Map& map;
	MA& modmap;
};

template <ModifiableArrangement MA, VertexRemovalTraits VRT>
requires(std::same_as<typename MA::Map,
                      typename VRT::Map>) void VertexRemovalSimplification<MA, VRT>::initialize() {
	for (typename Map::Vertex_const_iterator v = map.vertices_begin(); v != map.vertices_end(); ++v) {
		initVertex(&*v);
	}
}

template <ModifiableArrangement MA, VertexRemovalTraits VRT>
requires(std::same_as<typename MA::Map, typename VRT::Map>) void VertexRemovalSimplification<
    MA, VRT>::simplify(Number<Exact> t) {
	if constexpr (ModifiableArrangementWithHistory<MA>) {
		modmap.recallThreshold(t);
	}

	typename Map::Vertex_handle best;
	Number<Exact> best_cost;

	while (findBest(best, best_cost) && best_cost <= t) {
		execute(best, best_cost);
	}
}

template <ModifiableArrangement MA, VertexRemovalTraits VRT>
requires(std::same_as<typename MA::Map,
                      typename VRT::Map>) void VertexRemovalSimplification<MA, VRT>::simplify(int c) {

	if constexpr (ModifiableArrangementWithHistory<MA>) {
		modmap.recallComplexity(c);
	}

	typename Map::Vertex_handle best;
	Number<Exact> best_cost;

	while (map.number_of_edges() > c && findBest(best, best_cost)) {
		execute(best, best_cost);
	}
}

template <ModifiableArrangement MA, VertexRemovalTraits VRT>
requires(std::same_as<typename MA::Map, typename VRT::Map>) void VertexRemovalSimplification<
    MA, VRT>::initVertex(Map::Vertex_handle v) {
	if (v->degree() == 2) {
		auto inc = v->incident_halfedges();

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

			// initialize block
			int b = 0;

			// free floating vertices: if this blocks, then it will need reinitialization
			for (typename Map::Isolated_vertex_iterator it = face->isolated_vertices_begin();
			     it != face->isolated_vertices_end(); ++it) {
				if (T.has_on_bounded_side(it->point())) {
					b = BLOCKED_FLOATING;
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
			Number<Exact> cost = VRT::vrGetCost(v);
			if (!found || cost < best_cost) {
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
	typename Map::Halfedge_handle e = modmap.mergeWithNext(VRT::vrGetHalfedge(v), cost);

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
	return Triangle<Exact>(inc->source()->point(), v->point(), inc->next()->target()->point());
}

} // namespace cartocrow::simplification
