#pragma once

#include "../../core/core.h"
#include "../../core/region_arrangement.h"
#include "../common.h"

namespace cartocrow::simplification {

template <class T> concept VertexRemovalTraits = requires {

	typename T::Map;
};

template <VertexRemovalTraits VRT> class VertexRemovalSimplification {

  public:
	VertexRemovalSimplification(VRT::Map& inmap) : map(inmap), max_cost(0){};

	void initialize();
	void simplify(const int c, const Number<Exact> t);

  private:
	void initVertex(VRT::Map::Vertex_handle v);
	void reduceCounts(VRT::Map::Vertex_handle v);

	Triangle<Exact> triangle(VRT::Map::Vertex_handle v) {
		typename VRT::Map::Halfedge_handle inc = VRT::vrGetHalfedge(v);
		return Triangle<Exact>(inc->source()->point(), v->point(), inc->next()->target()->point());
	}

	const short BLOCKED_FLOATING = -1;
	const short NOOP_TRIANGLE = -2;
	const short NOOP_DEGREE = -3;

	Number<Exact> max_cost;
	VRT::Map& map;
};

template <VertexRemovalTraits VRT> void VertexRemovalSimplification<VRT>::initialize() {
	this->max_cost = 0;

	for (typename VRT::Map::Vertex_const_iterator v = this->map.vertices_begin();
	     v != this->map.vertices_end(); ++v) {
		initVertex(&*v);
	}
}

template <VertexRemovalTraits VRT>
void VertexRemovalSimplification<VRT>::simplify(const int c, const Number<Exact> t) {

	while (this->map.number_of_edges() > c) {

		typename VRT::Map::Vertex_handle best;
		Number<Exact> best_cost;
		bool found = false;
		for (auto& v : this->map.vertex_handles()) {

			if (VRT::vrGetBlockingNumber(v) == 0) {
				Number<Exact> cost = VRT::vrGetCost(v);
				if (!found || cost < best_cost) {
					best = v;
					best_cost = cost;
					found = true;
				}
			}
		}

		if (found && best_cost <= t) {
			// walk over twin face to reduce counts
			reduceCounts(best);

			// execute
			if (best_cost > max_cost) {
				max_cost = best_cost;
			}

			//std::cout << "removing " << best->point() << ", degree = " << best->degree()
			//          << ", prev = " << VRT::vrGetHalfedge(best)->source()->point()
			//          << ", next = " << VRT::vrGetHalfedge(best)->next()->target()->point() << "\n";

			auto e = merge_with_next(map, VRT::vrGetHalfedge(best));

			//std::cout << "   reached " << this->map.number_of_edges() << "\n";

			// reinit neighbors
			initVertex(e->source());
			initVertex(e->target());

			// NB: we only get NOOP_TRIANGLE if the vertex is actually degree 2
			// we need to test both endpoints in case the other is of high degree
			// but if both would be NOOP_TRIANGLE, then they identify the same third point (disconnected triangle)
			if (VRT::vrGetBlockingNumber(e->source()) == NOOP_TRIANGLE) {
				// we just created a triangle
				// also update the third point
				initVertex(e->prev()->source());
			} else if (VRT::vrGetBlockingNumber(e->target()) == NOOP_TRIANGLE) {
				// we just created a triangle
				// also update the third point
				initVertex(e->next()->target());
			}
		} else {
			// nothing more to do
			break;
		}
	}
}

template <VertexRemovalTraits VRT>
void VertexRemovalSimplification<VRT>::initVertex(VRT::Map::Vertex_handle v) {

	if (v->degree() == 2) {
		auto inc = v->incident_halfedges();

		typename VRT::Map::Halfedge_handle realinc;
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

			typename VRT::Map::Face_handle face = realinc->face();

			typename VRT::Map::Vertex_handle p = realinc->source();
			typename VRT::Map::Vertex_handle n = realinc->next()->target();

			// initialize block
			int b = 0;

			// free floating vertices: if this blocks, then it will need reinitialization
			for (typename VRT::Map::Isolated_vertex_iterator it = face->isolated_vertices_begin();
			     it != face->isolated_vertices_end(); ++it) {
				if (T.has_on_bounded_side(it->point())) {
					b = BLOCKED_FLOATING;
				}
			}

			if (b >= 0) {
				// otherwise, count blocking points

				if (!face->is_unbounded()) {
					// the outer rim
					typename VRT::Map::Ccb_halfedge_circulator circ = face->outer_ccb();
					typename VRT::Map::Ccb_halfedge_circulator curr = circ;
					do {
						typename VRT::Map::Vertex_handle e_src = curr->source();
						if (e_src != p && e_src != v && e_src != n &&
						    T.has_on_bounded_side(e_src->point())) {
							b++;
						}
					} while (++curr != circ);
				}

				// holes
				for (typename VRT::Map::Hole_iterator h = face->holes_begin();
				     h != face->holes_end(); ++h) {
					typename VRT::Map::Ccb_halfedge_circulator circ = *h;
					typename VRT::Map::Ccb_halfedge_circulator curr = circ;
					do {
						typename VRT::Map::Vertex_handle e_src = curr->source();
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

template <VertexRemovalTraits VRT>
void VertexRemovalSimplification<VRT>::reduceCounts(VRT::Map::Vertex_handle v) {
	typename VRT::Map::Face_handle face = VRT::vrGetHalfedge(v)->twin()->face();

	Point<Exact> pt = v->point();

	if (!face->is_unbounded()) {
		// the outer rim
		typename VRT::Map::Ccb_halfedge_circulator circ = face->outer_ccb();
		typename VRT::Map::Ccb_halfedge_circulator curr = circ;
		do {
			// NB: may also reduce v and its neighbors, but this doesn't matter
			typename VRT::Map::Vertex_handle w = curr->target();
			if (VRT::vrGetBlockingNumber(w) >= 0 && triangle(w).has_on_bounded_side(pt)) {
				VRT::vrSetBlockingNumber(w, VRT::vrGetBlockingNumber(w) - 1);
			}
		} while (++curr != circ);
	}

	// holes
	for (typename VRT::Map::Hole_iterator h = face->holes_begin(); h != face->holes_end(); ++h) {
		typename VRT::Map::Ccb_halfedge_circulator circ = *h;
		typename VRT::Map::Ccb_halfedge_circulator curr = circ;
		do {
			// NB: may also reduce v and its neighbors, but this doesn't matter
			typename VRT::Map::Vertex_handle w = curr->target();
			if (VRT::vrGetBlockingNumber(w) >= 0 && triangle(w).has_on_bounded_side(pt)) {
				VRT::vrSetBlockingNumber(w, VRT::vrGetBlockingNumber(w) - 1);
			}

		} while (++curr != circ);
	}
}

} // namespace cartocrow::simplification
