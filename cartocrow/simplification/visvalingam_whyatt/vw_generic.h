#pragma once

#include "../../core/core.h"
#include "../../core/region_arrangement.h"
#include "../common.h"

namespace cartocrow::simplification {

template <class T> concept VWTraits = requires {

	typename T::Map;
};

template <VWTraits VW> class VWSimplification {

  public:
	VWSimplification(){};
	~VWSimplification(){};

	void initialize(VW::Map& inmap);
	void simplify(const int c, const Number<Exact> t);

  private:
	void initVertex(VW::Map::Vertex_handle v);
	void reduceCounts(VW::Map::Vertex_handle v);

	Number<Exact> max_cost;
	VW::Map& map;
};

template <VWTraits VW> void VWSimplification<VW>::initialize(VW::Map& inmap) {
	this->max_cost = 0;
	this->map = inmap;

	for (typename VW::Map::Vertex_const_iterator v = this->map.vertices_begin();
	     v != this->map.vertices_end(); ++v) {
		initVertex(&*v);
	}
}

template <VWTraits VW> void VWSimplification<VW>::simplify(const int c, const Number<Exact> t) {

	while (this->map.number_of_edges() > c) {

		typename VW::Map::Vertex_handle best;
		bool found = false;
		for (auto& v : this->map.vertex_handles()) {

			if (v->data().block == 0 && (!found || best->data().cost < v->data().cost)) {
				best = v;
				found = true;
			}
		}

		if (found && best->data().cost <= t) {
			// walk over twin face to reduce counts
			reduceCounts(best);

			// execute
			if (best->data().cost > max_cost) {
				max_cost = best->data().cost;
			}

			auto e = merge_with_next(map, best->data().inc);

			// reinit neighbors
			initVertex(e->source());
			initVertex(e->target());
		} else {
			// nothing more to do
			break;
		}
	}
}

template <VWTraits VW> void VWSimplification<VW>::initVertex(VW::Map::Vertex_handle v) {

	if (v->degree() == 2) {
		auto inc = v->incident_halfedges();
		if (inc->next()->next()->next() == inc) {
			// triangle
			// not removable
			v->data().block = -1;
		} else {
			// degree 2, but no triangle
			// removable
			v->data().block = 0;
			v->data().inc = inc;

			Triangle<Exact> T = v->data().triangle();

			Number<Exact> area = T.area();
			if (area < 0) {
				v->data().inc = inc->next()->twin();
			}
			v->data().cost = VW::computeCost(v);

			typename VW::Map::Face_handle face = v->data().inc->face();

			// initialize block
			// TODO: check vertex-based instead of halfedge based
			if (!face->is_unbounded()) {
				// the outer rim
				typename VW::Map::Ccb_halfedge_circulator circ = face->outer_ccb();
				typename VW::Map::Ccb_halfedge_circulator curr = circ;
				do {
					typename VW::Map::Halfedge_handle curr_e = curr;
					if (curr_e != v->data().inc && curr_e != v->data().inc->next() &&
					    curr_e != v->data().inc->next()->next() &&
					    T.has_on_bounded_side(curr_e->source()->point())) {
						v->data().block++;
					}
				} while (++curr != circ);
			}

			// holes
			for (typename VW::Map::Hole_iterator h = face->holes_begin(); h != face->holes_end();
			     ++h) {
				typename VW::Map::Ccb_halfedge_circulator circ = *h;
				typename VW::Map::Ccb_halfedge_circulator curr = circ;
				do {
					typename VW::Map::Halfedge_handle curr_e = curr;
					if (curr_e != v->data().inc && curr_e != v->data().inc->next() &&
					    curr_e != v->data().inc->next()->next() &&
					    T.has_on_bounded_side(curr_e->source()->point())) {
						v->data().block++;
					}

				} while (++curr != circ);
			}

			// free floating vertices
			for (typename VW::Map::Isolated_vertex_iterator it = face->isolated_vertices_begin();
			     it != face->isolated_vertices_end(); ++it) {
				if (T.has_on_bounded_side(it->point())) {
					v->data().block++;
				}
			}
		}

	} else {
		// degree != 2
		// not removable
		v->data().block = -1;
	}
}

template <VWTraits VW> void VWSimplification<VW>::reduceCounts(VW::Map::Vertex_handle v) {
	typename VW::Map::Face_handle face = v->data().inc->twin()->face();

	if (!face->is_unbounded()) {
		// the outer rim
		typename VW::Map::Ccb_halfedge_circulator circ = face->outer_ccb();
		typename VW::Map::Ccb_halfedge_circulator curr = circ;
		do {
			// NB: may also reduce v and its neighbors, but this doesn't matter
			if (curr->target()->data().triangle().has_on_bounded_side(v->point())) {
				curr->target()->data().block--;
			}
		} while (++curr != circ);
	}

	// holes
	for (typename VW::Map::Hole_iterator h = face->holes_begin(); h != face->holes_end(); ++h) {
		typename VW::Map::Ccb_halfedge_circulator circ = *h;
		typename VW::Map::Ccb_halfedge_circulator curr = circ;
		do {
			// NB: may also reduce v and its neighbors, but this doesn't matter
			if (curr->target()->data().triangle().has_on_bounded_side(v->point())) {
				curr->target()->data().block--;
			}

		} while (++curr != circ);
	}
}

} // namespace cartocrow::simplification
