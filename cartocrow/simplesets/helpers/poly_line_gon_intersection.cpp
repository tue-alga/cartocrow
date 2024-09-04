#include "poly_line_gon_intersection.h"

namespace cartocrow::simplesets {
std::vector<CSPolyline> poly_line_gon_intersection(CSPolygon gon, CSPolyline line) {
	assert(!gon.is_empty());

	using Arr = CGAL::Arrangement_2<CSTraits, CGAL::Arr_extended_dcel<CSTraits, std::monostate, HalfEdgePolylineData, std::monostate>>;
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

	// todo: fix. Use Arr_polycurve_traits_2 class.
	// 1. line_edges is incorrect because the insertion of polygon edges changes the edge data.
	// use arrangement with history and polycurve triats.
	// 2. below I don't check whether a line_edge actually lies inside an interior face of a polygon -_-.
	// check whether the outer_ccb of one of the adjacent faces of the edge originates from the outer boundary of the
	// polygon of from a hole.
	// 3. make it work for CSPolygonWithHoles


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
