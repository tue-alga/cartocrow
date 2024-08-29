#include "vw_simplification.h"

#include <CGAL/Constrained_Delaunay_triangulation_2.h>

namespace cartocrow::simplification {

void VWSimplification::initialize(BoundaryMap& map, GeometryStore& store) {
	this->store = store;
	complexity = 0;

	for (Boundary bound : map.boundaries) {
		Vertex_handle first = cdt.insert(bound.points[0]);
		first.removable = true;
		Vertex_handle prev = first;

		for (int i = 1; i < bound.points.size(); i++) {
			Vertex_handle vtx = cdt.insert(bound.points[i]);
			cdt.insert_constraint(prev, vtx);
			complexity++;
			prev.next = vtx;
			vtx.prev = prev;
			vtx.removable = true;

			prev = vtx;
		}

		if (bound.closed) {
			prev.next = first;
			first.prev = prev;
			cdt.insert_constraint(prev, first);
			complexity++;
		} else {
			first.removable = false;
			prev.removable = false;
		}
	}

	auto vtx_it = cdt.finite_vertex_handles();
	for (Vertex_handle vtx = vtx_it.begin(); vtx != vtx_it.end(); vtx_it++) {
		initialize(vtx);
	}
}
}
void VWSimplification::run(StopCriterion stop) {

	// iteratively remove the lowest-cost vertex
	while (!queue.is_empty()) {
		// find the best
		Vertex_handle best = queue.peek();

		// should we stop?
		if (stop.stop(best.cost, complexity)) {
			break;
		}

		// collect vertices that were blocked by this vertex

		// perform the operation
		Vertex_handle prev = best.prev;
		Vertex_handle next = best.next;
		cdt.remove_incident_constraints(best);
		cdt.remove(best);
		cdt.insert_constraint(prev, next);

		// update neighbors
		initialize(prev);
		initialize(next);

		// update blocked vertices

		complexity--;
	}
}

} // namespace cartocrow::simplification