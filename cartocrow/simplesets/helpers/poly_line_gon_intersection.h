#ifndef CARTOCROW_POLY_LINE_GON_INTERSECTION_H
#define CARTOCROW_POLY_LINE_GON_INTERSECTION_H

#include "cartocrow/simplesets/types.h"
#include "cs_polyline_helpers.h"
#include "cs_polygon_helpers.h"
#include <CGAL/Arrangement_with_history_2.h>

namespace cartocrow::simplesets {
// todo: edge case where edges of dilated patterns overlap, so a half-edge may have multiple origins.
struct HalfEdgePolylineData {
	bool of_polyline = false;
};

std::vector<CSPolyline> poly_line_gon_intersection(const CSPolygon& gon, const CSPolyline& line, bool keepOverlap);
std::vector<CSPolyline> poly_line_gon_difference(const CSPolygon& gon, const CSPolyline& line, bool keepOverlap);
std::vector<CSPolyline> poly_line_gon_intersection(const CSPolygonWithHoles& gon, const CSPolyline& line, bool keepOverlap);
std::vector<CSPolyline> poly_line_gon_difference(const CSPolygonWithHoles& gon, const CSPolyline& line, bool keepOverlap);

template <class OutputIterator>
void poly_line_gon_intersection(const CSPolygonWithHoles& gon, const CSPolyline& line, OutputIterator out, bool difference, bool keepOverlap) {
	using Arr = CGAL::Arrangement_with_history_2<PolyCSTraits, CGAL::Arr_extended_dcel<CSTraits, std::monostate, HalfEdgePolylineData, std::monostate>>;
	Arr arr;
	auto linePolycurve = arrPolycurveFromCSPolyline(line);
	auto outerGonPolycurve = arrPolycurveFromCSPolygon(gon.outer_boundary());

	auto lch = CGAL::insert(arr, linePolycurve);
	auto ogch = CGAL::insert(arr, outerGonPolycurve);
	std::vector<Arr::Curve_handle> hgchs;

	std::vector<CSPolycurve> holesGonPolycurves;
	for (auto hit = gon.holes_begin(); hit != gon.holes_end(); ++hit) {
		hgchs.push_back(CGAL::insert(arr, arrPolycurveFromCSPolygon(*hit)));
	}

	std::vector<Arr::Halfedge_handle> line_edges_keep;
	for (auto eit = arr.induced_edges_begin(lch); eit != arr.induced_edges_end(lch); ++eit) {
		Arr::Halfedge_handle edge = *eit;

		bool onGonEdge = false;
		for (auto curveIt = arr.originating_curves_begin(edge); curveIt != arr.originating_curves_end(edge); ++curveIt) {
			auto curve = *curveIt;
			Arr::Curve_handle ch = curveIt;
			if (ch == ogch) {
				onGonEdge = true;
			}
			for (const auto& hgch : hgchs) {
				if (ch == hgch) {
					onGonEdge = true;
				}
			}
		}
		if (onGonEdge) {
			if (keepOverlap) {
				line_edges_keep.push_back(eit->ptr());
			}
			continue;
		}

		bool liesInGon = false;
		for (auto fh : {edge->face(), edge->twin()->face()}) {
			if (!fh->has_outer_ccb()) {
				continue;
			}
			auto ccb = fh->outer_ccb();
			auto ccbIt = ccb;
			do {
				// if *ccbIt lies on outer face.
				for (auto curveIt = arr.originating_curves_begin(ccbIt);
				     curveIt != arr.originating_curves_end(ccbIt); ++curveIt) {
					Arr::Curve_handle ch = curveIt;
					if (ch == ogch) {
						liesInGon = true;
						break;
					}
				}
			} while (++ccbIt != ccb);
			if (liesInGon) break;
		}

		if (!difference && liesInGon) {
			line_edges_keep.push_back(eit->ptr());
		}
		if (difference && !liesInGon) {
			line_edges_keep.push_back(eit->ptr());
		}
	}

	while (!line_edges_keep.empty()) {
		// Find first edge on connected component of polyline (in the intersection with polygon)
		auto start = line_edges_keep.front();
		auto curr = start;
		while (curr->prev()->data().of_polyline) {
			curr = curr->prev();

			// The polyline and polygon do not intersect.
			if (curr == start) {
				return;
			}
		}
		std::vector<X_monotone_curve_2> xmcs;
		auto last_it = line_edges_keep.end();
		do {
			last_it = std::remove(line_edges_keep.begin(), last_it, curr);
			std::copy(curr->curve().subcurves_begin(), curr->curve().subcurves_end(), std::back_inserter(xmcs));
			curr = curr->next();
		} while (curr->data().of_polyline);
		line_edges_keep.erase(last_it, line_edges_keep.end());
		++out = CSPolyline(xmcs.begin(), xmcs.end());
	}
}
}

#endif //CARTOCROW_POLY_LINE_GON_INTERSECTION_H
