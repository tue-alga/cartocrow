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
		for (auto &v : map.vertex_handles()) {
		
			if (v->data().block == 0 && (!found || best->data().cost < v->data().cost)) {
				best = v;
				found = true;
			}
		}

		if (found && best->data().cost <= t) {
			// execute
			if (best->data().cost > max_cost) {
				max_cost = best->data().cost;
			}

			auto e = merge_with_next(map, best->data().inc);

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
		}

	} else {
		// degree != 2
		// not removable
		v->data().block = -1;
	}
}

} // namespace cartocrow::simplification
