#ifndef CARTOCROW_POLY_LINE_GON_INTERSECTION_H
#define CARTOCROW_POLY_LINE_GON_INTERSECTION_H

#include "cartocrow/simplesets/types.h"
#include "cs_polyline_helpers.h"
#include "cs_polygon_helpers.h"
#include <CGAL/Arrangement_with_history_2.h>

namespace cartocrow::simplesets {
// todo: edge case where edges of dilated patterns overlap, so a half-edge may have multiple origins.
std::vector<CSPolyline> intersection(const CSPolyline& line, const CSPolygon& gon, bool keepOverlap);
std::vector<CSPolyline> difference(const CSPolyline& line, const CSPolygon& gon, bool keepOverlap);
std::vector<CSPolyline> intersection(const CSPolyline& line, const CSPolygonWithHoles& gon, bool keepOverlap);
std::vector<CSPolyline> difference(const CSPolyline& line, const CSPolygonWithHoles& gon, bool keepOverlap);

template <class OutputIterator>
void intersection(const CSPolyline& line, const CSPolygonWithHoles& gon, OutputIterator out, bool difference, bool keepOverlap) {
	PolyCSTraits traits;
	auto equals = traits.equal_2_object();
	using Arr = CGAL::Arrangement_with_history_2<PolyCSTraits>;
	Arr arr;
	auto linePolycurve = arrPolycurveFromCSPolyline(line);
	auto outerGonPolycurve = arrPolycurveFromCSPolygon(gon.outer_boundary());

//	std::cout << "!" << std::endl;
//	for (auto cit = outerGonPolycurve.subcurves_begin(); cit != outerGonPolycurve.subcurves_end(); ++cit) {
//		if (cit->is_circular()) {
//			std::cout << cit->supporting_circle().center() << " " << cit->supporting_circle().squared_radius() << std::endl;
//		}
//		if (cit->is_linear()) {
//			std::cout << cit->supporting_line() << std::endl;
//		}
//		std::cout << cit->source() << " -> " << cit->target() << std::endl;
//	}
//	for (auto cit = linePolycurve.subcurves_begin(); cit != linePolycurve.subcurves_end(); ++cit) {
//		if (cit->is_circular()) {
//			std::cout << cit->supporting_circle().center() << " " << cit->supporting_circle().squared_radius() << std::endl;
//		}
//		if (cit->is_linear()) {
//			std::cout << cit->supporting_line() << std::endl;
//		}
//		std::cout << cit->source() << " -> " << cit->target() << std::endl;
//	}

	auto ogch = CGAL::insert(arr, outerGonPolycurve);
	auto lch = CGAL::insert(arr, linePolycurve);
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
		bool liesInGon = false;
		if (onGonEdge) {
			if (keepOverlap) {
				liesInGon = true;
			}
		} else {
			for (auto fh : {edge->face(), edge->twin()->face()}) {
				if (!fh->has_outer_ccb()) {
					continue;
				}
				auto ccb = fh->outer_ccb();
				auto ccbIt = ccb;
				do {
					if (!equals(ccbIt->source()->point(), ccbIt->curve().subcurves_begin()->source()))
						continue;
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
				if (liesInGon)
					break;
			}
		}

		if (!difference && liesInGon) {
			if (equals(edge->source()->point(), edge->curve().subcurves_begin()->source())) {
				line_edges_keep.push_back(edge);
	 		} else {
				line_edges_keep.push_back(edge->twin());
			}
		}
		if (difference && !liesInGon) {
			if (equals(edge->source()->point(), edge->curve().subcurves_begin()->source())) {
				line_edges_keep.push_back(edge);
			} else {
				line_edges_keep.push_back(edge->twin());
			}
		}
	}

	auto originatesFromPolyline = [&arr, &lch, &equals](Arr::Halfedge_handle h) {
		for (auto cit = arr.originating_curves_begin(h); cit != arr.originating_curves_end(h); ++cit) {
			Arr::Curve_handle ch = cit;
			if (ch == lch && equals(h->source()->point(), h->curve().subcurves_begin()->source())) {
				return true;
			}
		}
		return false;
	};

	while (!line_edges_keep.empty()) {
		// Find first edge on connected component of polyline (in the intersection with polygon)
		auto start = line_edges_keep.front();
		auto curr = start;
		while (originatesFromPolyline(curr->prev())) {
			curr = curr->prev();

			if (curr == start) {
				break;
			}
		}
		std::vector<X_monotone_curve_2> xmcs;
		auto last_it = line_edges_keep.end();
		do {
			last_it = std::remove(line_edges_keep.begin(), last_it, curr);
			std::copy(curr->curve().subcurves_begin(), curr->curve().subcurves_end(), std::back_inserter(xmcs));
			curr = curr->next();
		} while (originatesFromPolyline(curr));
		line_edges_keep.erase(last_it, line_edges_keep.end());

		++out = CSPolyline(xmcs.begin(), xmcs.end());
	}
}
}

#endif //CARTOCROW_POLY_LINE_GON_INTERSECTION_H
