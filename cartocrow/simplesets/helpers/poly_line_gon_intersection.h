#ifndef CARTOCROW_POLY_LINE_GON_INTERSECTION_H
#define CARTOCROW_POLY_LINE_GON_INTERSECTION_H

#include "cartocrow/simplesets/types.h"
#include "cs_curve_helpers.h"
#include "cs_polyline_helpers.h"
#include "cs_polygon_helpers.h"
#include <CGAL/Arrangement_with_history_2.h>
#include <CGAL/Arr_observer.h>
#include <CGAL/Arr_extended_dcel.h>

namespace cartocrow::simplesets {
// todo: edge case where edges of dilated patterns overlap, so a half-edge may have multiple origins.
std::vector<CSPolyline> intersection(const CSPolyline& line, const CSPolygon& gon, bool keepOverlap);
std::vector<CSPolyline> difference(const CSPolyline& line, const CSPolygon& gon, bool keepOverlap);
std::vector<CSPolyline> intersection(const CSPolyline& line, const CSPolygonWithHoles& gon, bool keepOverlap);
std::vector<CSPolyline> difference(const CSPolyline& line, const CSPolygonWithHoles& gon, bool keepOverlap);

template <class OutputIterator>
void intersection(const CSPolyline& line, const CSPolygonWithHoles& gon, OutputIterator out, bool difference, bool keepOverlap) {
	CSTraits traits;
	auto equals = traits.equal_2_object();
	auto opposite = traits.construct_opposite_2_object();
	enum Origin {
		Polyline,
		PolygonOuter,
		PolygonHole,
	};
	struct HalfEdgeData {
		std::vector<Origin> origins;
	};
	using Arr = CGAL::Arrangement_2<CSTraits, CGAL::Arr_extended_dcel<CSTraits, std::monostate, HalfEdgeData, std::monostate>>;
	using FH = typename Arr::Face_handle;
	using HeH = typename Arr::Halfedge_handle;
	using VH = typename Arr::Vertex_handle;

	Arr arr;
	class Observer : public CGAL::Arr_observer<Arr> {
	  public:
		Observer(Arr& arr) : CGAL::Arr_observer<Arr>(arr) {}

		virtual void after_create_edge(HeH e) {
			e->data().origins.push_back(currentOrigin);
			e->twin()->data().origins.push_back(currentOrigin);
		}

		virtual void before_split_edge(HeH e, VH v, const X_monotone_curve_2& c1, const X_monotone_curve_2& c2) {
			beforeSplitData = e->data();
		}

		virtual void after_split_edge(HeH e1, HeH e2) {
			e1->set_data(beforeSplitData);
			e1->twin()->set_data(beforeSplitData);
			e2->twin()->set_data(beforeSplitData);
			e2->set_data(beforeSplitData);
			CSTraits traits;
			auto opposite = traits.construct_opposite_2_object();
			if (liesOn(e1->curve(), xmCurve) || liesOn(opposite(e1->curve()), xmCurve)) {
				e1->data().origins.push_back(currentOrigin);
				e1->twin()->data().origins.push_back(currentOrigin);
			}
			if (liesOn(e2->curve(), xmCurve) || liesOn(opposite(e2->curve()), xmCurve)) {
				e2->data().origins.push_back(currentOrigin);
				e2->twin()->data().origins.push_back(currentOrigin);
			}
		}

		virtual void after_modify_edge(HeH e) {
			e->data().origins.push_back(currentOrigin);
			e->twin()->data().origins.push_back(currentOrigin);
		}

		Origin currentOrigin;
		X_monotone_curve_2 xmCurve;
	  private:
		HalfEdgeData beforeSplitData;
	};

	Observer observer(arr);

	observer.currentOrigin = Origin::Polyline;
	for (auto cit = line.curves_begin(); cit != line.curves_end(); ++cit) {
		observer.xmCurve = *cit;
		CGAL::insert(arr, *cit);
	}
	observer.currentOrigin = Origin::PolygonOuter;
	for (auto cit = gon.outer_boundary().curves_begin(); cit != gon.outer_boundary().curves_end(); ++cit) {
		observer.xmCurve = *cit;
		CGAL::insert(arr, *cit);
	}
	observer.currentOrigin = Origin::PolygonHole;
	for (auto hit = gon.holes_begin(); hit != gon.holes_end(); ++hit) {
		for (auto cit = hit->curves_begin(); cit != hit->curves_end(); ++cit) {
			observer.xmCurve = *cit;
			CGAL::insert(arr, *cit);
		}
	}

	std::vector<typename Arr::Halfedge_handle> line_edges_keep;
	for (auto eit = arr.edges_begin(); eit != arr.edges_end(); ++eit) {
		HeH edge = eit;

		bool onGonEdge = false;
		bool onPolyline = false;
		for (auto origin : edge->data().origins) {
			if (origin == Origin::Polyline) {
				onPolyline = true;
			}
			if (origin == Origin::PolygonOuter || origin == Origin::PolygonHole) {
				onGonEdge = true;
			}
		}
		if (!onPolyline) {
			continue;
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
					if (!equals(ccbIt->source()->point(), ccbIt->curve().source()))
						continue;
					// if *ccbIt lies on outer face.
					for (auto origin : ccbIt->data().origins) {
						if (origin == Origin::PolygonOuter) {
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
			if (equals(edge->source()->point(), edge->curve().source())) {
				line_edges_keep.push_back(edge);
			} else {
				line_edges_keep.push_back(edge->twin());
			}
		}
		if (difference && !liesInGon) {
			if (equals(edge->source()->point(), edge->curve().source())) {
				line_edges_keep.push_back(edge);
			} else {
				line_edges_keep.push_back(edge->twin());
			}
		}
	}

	auto originatesFromPolyline = [&arr, &equals](HeH h) {
		for (auto origin : h->data().origins) {
			if (origin == Origin::Polyline && equals(h->source()->point(), h->curve().source())) {
				return true;
			}
		}
		return false;
	};

	while (!line_edges_keep.empty()) {
		// Find first edge on connected component of polyline (in the intersection with polygon)
		auto start = line_edges_keep.front();
		auto startStart = start;
		while (originatesFromPolyline(startStart->prev())) {
			startStart = startStart->prev();

			if (startStart == start) {
				break;
			}
		}
		std::vector<X_monotone_curve_2> xmcs;
		auto last_it = line_edges_keep.end();
		auto curr = startStart;
		do {
			last_it = std::remove(line_edges_keep.begin(), last_it, curr);
			xmcs.push_back(curr->curve());
			curr = curr->next();
		} while (originatesFromPolyline(curr) && curr != startStart);
		line_edges_keep.erase(last_it, line_edges_keep.end());

		++out = CSPolyline(xmcs.begin(), xmcs.end());
	}
}

template <class OutputIterator>
void intersection(const CSPolyline& line, const CSPolygon& gon, OutputIterator out, bool difference, bool keepOverlap) {
	CSPolygonWithHoles withHoles(gon);
	return intersection(line, withHoles, out, difference, keepOverlap);
}

// May crash due to CGAL bug: https://github.com/CGAL/cgal/issues/8468
#if 0
template <class OutputIterator>
void intersectionCrashes(const CSPolyline& line, const CSPolygonWithHoles& gon, OutputIterator out, bool difference, bool keepOverlap) {
	PolyCSTraits traits;
	auto equals = traits.equal_2_object();
	using Arr = CGAL::Arrangement_with_history_2<PolyCSTraits>;
	Arr arr;
	auto linePolycurve = arrPolycurveFromCSPolyline(line);
	auto outerGonPolycurve = arrPolycurveFromCSPolygon(gon.outer_boundary());

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
#endif
}

#endif //CARTOCROW_POLY_LINE_GON_INTERSECTION_H
