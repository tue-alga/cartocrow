#include "poly_line_gon_intersection.h"

namespace cartocrow::simplesets {
std::vector<CSPolyline> poly_line_gon_intersection(CSPolygon gon, CSPolyline line) {
	assert(!gon.is_empty());

	Arr arr;
	CGAL::insert_non_intersecting_curves(arr, line.curves_begin(), line.curves_end());
	for (auto eit = arr.edges_begin(); eit != arr.edges_end(); ++eit) {
		eit->set_data({true});
		eit->twin()->set_data({true});
	}
	CGAL::insert(arr, gon.curves_begin(), gon.curves_end());

	std::vector<Arr::Halfedge_handle> line_edges;
	for (auto eit = arr.edges_begin(); eit != arr.edges_end(); ++eit) {
		if (eit->data().of_polyline) {
			if (eit->source()->point() == eit->curve().source()) {
				line_edges.push_back(eit.ptr());
			} else {
				assert(eit->twin()->source()->point() == eit->twin()->curve().source());
				line_edges.push_back(eit->twin().ptr());
			}
		}
	}

	std::vector<CSPolyline> parts;
	while (!line_edges.empty()) {
		// Find first edge on connected component of polyline (in the intersection with polygon)
		auto start = line_edges.front();
		auto curr = start;
		while (curr->prev()->data().of_polyline) {
			curr = curr->prev();

			// The polyline and polygon do not intersect.
			if (curr == start) {
				return {};
			}
		}
		std::vector<X_monotone_curve_2> xmcs;
		auto last_it = line_edges.end();
		do {
			last_it = std::remove(line_edges.begin(), last_it, curr);
			xmcs.push_back(curr->curve());
			curr = curr->next();
		} while (curr->data().of_polyline);
		line_edges.erase(last_it, line_edges.end());
		parts.emplace_back(xmcs.begin(), xmcs.end());
	}

	return parts;
}
}
