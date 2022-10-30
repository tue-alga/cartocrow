#include "vw.h"

#include "../common.h"

namespace cartocrow::simplification {

VWSimplification::VWSimplification(VWMap& inmap) : map(inmap) {
	this->max_cost = 0;

	for (VWMap::Vertex_const_iterator v = map.vertices_begin(); v != map.vertices_end(); ++v) {
		initVertex(&*v);
	}
}

VWSimplification::~VWSimplification() {}

void VWSimplification::simplify(const int c, const Number<Exact> t) {

	while (this->map.number_of_edges() > c) {

		VWMap::Vertex_handle best;
		bool found = false;
		for (auto& v : map.vertex_handles()) {

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

void VWSimplification::initVertex(VWMap::Vertex_handle v) {

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

			v->data().cost = v->data().triangle().area();
			if (v->data().cost < 0) {
				v->data().inc = inc->next()->twin();
				v->data().cost *= -1;
			}

			VWMap::Face_handle face = v->data().inc->face();

			// initialize block
			if (!face->is_unbounded()) {
				// the outer rim
				VWMap::Ccb_halfedge_circulator circ = face->outer_ccb();
				VWMap::Ccb_halfedge_circulator curr = circ;
				do {
					VWMap::Halfedge_handle curr_e = curr;
					if (curr_e != v->data().inc && curr_e != v->data().inc->next() &&
					    curr_e != v->data().inc->next()->next() &&
					    T.has_on_bounded_side(curr_e->source()->point())) {
						v->data().block++;
					}
				} while (++curr != circ);
			}

			// holes
			for (VWMap::Hole_iterator h = face->holes_begin(); h != face->holes_end(); ++h) {
				VWMap::Ccb_halfedge_circulator circ = *h;
				VWMap::Ccb_halfedge_circulator curr = circ;
				do {
					VWMap::Halfedge_handle curr_e = curr;
					if (curr_e != v->data().inc && curr_e != v->data().inc->next() &&
					    curr_e != v->data().inc->next()->next() &&
					    T.has_on_bounded_side(curr_e->source()->point())) {
						v->data().block++;
					}

				} while (++curr != circ);
			}

			// free floating vertices
			for (VWMap::Isolated_vertex_iterator it = face->isolated_vertices_begin();
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

void VWSimplification::reduceCounts(VWMap::Vertex_handle v) {
	VWMap::Face_handle face = v->data().inc->twin()->face();

	if (!face->is_unbounded()) {
		// the outer rim
		VWMap::Ccb_halfedge_circulator circ = face->outer_ccb();
		VWMap::Ccb_halfedge_circulator curr = circ;
		do {
			// NB: may also reduce v and its neighbors, but this doesn't matter
			if (curr->target()->data().triangle().has_on_bounded_side(v->point())) {
				curr->target()->data().block--;
			}
		} while (++curr != circ);
	}

	// holes
	for (VWMap::Hole_iterator h = face->holes_begin(); h != face->holes_end(); ++h) {
		VWMap::Ccb_halfedge_circulator circ = *h;
		VWMap::Ccb_halfedge_circulator curr = circ;
		do {
			// NB: may also reduce v and its neighbors, but this doesn't matter
			if (curr->target()->data().triangle().has_on_bounded_side(v->point())) {
				curr->target()->data().block--;
			}

		} while (++curr != circ);
	}
}

} // namespace cartocrow::simplification
