#include "cavc_helpers.h"
#include "cavc_helpers.hpp"
#include "cs_curve_helpers.h"
#include "cs_polygon_helpers.h"
#include <cavc/polylineoffset.hpp>
#include <cavc/polylineoffsetislands.hpp>

namespace cartocrow {
std::vector<CSXMCurve> xmCurves(const cavc::Polyline<double>& polyline) {
	std::vector<CSXMCurve> xmCurves;

	auto vs = polyline.vertexes();
	if (polyline.isClosed()) {
		vs.push_back(vs.front());
	}

	for (int i = 0; i < vs.size(); ++i) {
		const auto& v = vs[i];
		if (i != 0) {
			const auto& pv = vs[i - 1];
			if (pv.bulgeIsZero()) {
				xmCurves.emplace_back(Point<Exact>{pv.x(), pv.y()}, Point<Exact>{v.x(), v.y()});
			} else {
				bool clockwise = pv.bulgeIsNeg();
				Point<Exact> source(pv.x(), pv.y());
				Point<Exact> target(v.x(), v.y());
				Point<Exact> m = CGAL::midpoint(source, target);
				auto dir = target - source;
				dir /= 2;
				Vector<Exact> perp = dir.perpendicular(CGAL::CLOCKWISE);
				auto third = m + perp * pv.bulge();
				auto center = CGAL::circumcenter(source, target, third);
				auto r2 = CGAL::squared_distance(center, target);
				CGAL::Orientation orientation = clockwise ? CGAL::CLOCKWISE : CGAL::COUNTERCLOCKWISE;
				Circle<Exact> circle(center, r2, orientation);
				OneRootPoint sourceA(source.x(), source.y());
				OneRootPoint targetA(target.x(), target.y());
				CSCurve curve(circle, sourceA, targetA);
				curveToXMonotoneCurves(curve, std::back_inserter(xmCurves));
			}
		}
	}

	return xmCurves;
}

std::variant<CSPolyline, CSPolygon> toCSPoly(const cavc::Polyline<double>& polyline) {
	auto curves = xmCurves(polyline);

	if (polyline.isClosed()) {
		return CSPolygon(curves.begin(), curves.end());
	} else {
		return CSPolyline(curves.begin(), curves.end());
	}
}

CSPolygon remove_degeneracies(const CSPolygon& polygon) {
	std::vector<CSXMCurve> xmCurves;
	for (auto cit = polygon.curves_begin(); cit != polygon.curves_end(); ++cit) {
		auto as = approximateAlgebraic(cit->source());
		auto at = approximateAlgebraic(cit->target());
		if (CGAL::squared_distance(as, at) > M_EPSILON) {
			xmCurves.push_back(*cit);
		}
	}
	return {xmCurves.begin(), xmCurves.end()};
}

cavc::Polyline<double> cavcPolyline(const CSPolygon& polygon) {
	auto clean = remove_degeneracies(polygon);
	return cavcPolyline(clean.curves_begin(), clean.curves_end(), true);
}

cavc::Polyline<double> cavcPolyline(const CSPolyline& polyline) {
	return cavcPolyline(polyline.curves_begin(), polyline.curves_end(), false);
}

cavc::OffsetLoopSet<double> offsetLoopSet(const CSPolygonSet& csPolygonSet, CGAL::Orientation outerOrientation) {
	std::vector<CSPolygonWithHoles> withHoless;
	csPolygonSet.polygons_with_holes(std::back_inserter(withHoless));

	std::vector<CSPolygon> ccw;
	std::vector<CSPolygon> cw;
	for (const auto& withHoles : withHoless) {
		if (!withHoles.is_unbounded()) {
			auto outer = withHoles.outer_boundary();
			if (outer.orientation() != outerOrientation) {
				outer.reverse_orientation();
			}
			if (outerOrientation == CGAL::CLOCKWISE) {
				cw.push_back(outer);
			} else {
				ccw.push_back(outer);
			}
		}
		for (auto hit = withHoles.holes_begin(); hit != withHoles.holes_end(); ++hit) {
			auto hole = *hit;
			if (hole.orientation() == outerOrientation) {
				hole.reverse_orientation();
			}
			if (outerOrientation == CGAL::CLOCKWISE) {
				ccw.push_back(hole);
			} else {
				cw.push_back(hole);
			}
		}
	}

	cavc::OffsetLoopSet<double> loopSet;
	for (const auto& polygon : cw) {
		auto pl = cavcPolyline(polygon);
		loopSet.cwLoops.push_back({0, pl, cavc::createApproxSpatialIndex(pl)});
	}
	for (const auto& polygon : ccw) {
		auto pl = cavcPolyline(polygon);
		loopSet.ccwLoops.push_back({0, pl, cavc::createApproxSpatialIndex(pl)});
	}

	return loopSet;
}


cavc::OffsetLoopSet<double> reverseLoopSet(const cavc::OffsetLoopSet<double>& loopSet) {
	cavc::OffsetLoopSet<double> reversed;
	for (const auto& cw : loopSet.cwLoops) {
		auto pl = cw.polyline;
		cavc::invertDirection(pl);
		reversed.ccwLoops.push_back({0, pl, cavc::createApproxSpatialIndex(pl)});
	}
	for (const auto& ccw : loopSet.ccwLoops) {
		auto pl = ccw.polyline;
		cavc::invertDirection(pl);
		reversed.cwLoops.push_back({0, pl, cavc::createApproxSpatialIndex(pl)});
	}
	return reversed;
}

CSPolygonSet csPolygonSet(const cavc::OffsetLoopSet<double>& loopSet, bool reverse) {
	// todo check
	GpsCSTraits traits;
	CSPolygonSet polygonSet;
	if (reverse) {
		for (const auto& loop : loopSet.cwLoops) {
			auto poly = toCSPoly(loop.polyline);
			auto polygon = std::get<CSPolygon>(poly);
			if (is_simple(polygon)) {
				polygon.reverse_orientation();
				polygonSet.join(polygon);
			}
		}
		for (const auto& loop : loopSet.ccwLoops) {
			auto poly = toCSPoly(loop.polyline);
			auto polygon = std::get<CSPolygon>(poly);
			if (is_simple(polygon)) {
				polygon.reverse_orientation();
				polygonSet.difference(polygon);
			}
		}
	} else {
		for (const auto& loop : loopSet.ccwLoops) {
			auto poly = toCSPoly(loop.polyline);
			auto polygon = std::get<CSPolygon>(poly);
			if (is_simple(polygon)) {
				polygonSet.join(polygon);
			}
		}
		for (const auto& loop : loopSet.cwLoops) {
			auto poly = toCSPoly(loop.polyline);
			auto polygon = std::get<CSPolygon>(poly);
			// this is_simple_polygon causes CGAL warnings to be streamed to
			if (is_simple(polygon)) {
				polygonSet.difference(polygon);
			}
		}
	}
	return polygonSet;
}

CSPolygonSet approximateDilateOrErode(const CSPolygonSet& polygonSet, double radius, bool dilate) {
	auto loopSet = offsetLoopSet(polygonSet, dilate ? CGAL::CLOCKWISE : CGAL::COUNTERCLOCKWISE);

	cavc::ParallelOffsetIslands<double> alg;
	cavc::OffsetLoopSet<double> offsetResult = alg.compute(loopSet, radius);

	return csPolygonSet(offsetResult, dilate);
}

CSPolygonSet approximateDilate(const CSPolygonSet& csPolygonSet, double radius) {
	return approximateDilateOrErode(csPolygonSet, radius, true);
}

CSPolygonSet approximateErode(const CSPolygonSet& csPolygonSet, double radius) {
	return approximateDilateOrErode(csPolygonSet, radius, false);
}

CSPolygonSet approximateSmoothCO(const CSPolygonSet& polygonSet, double radius) {
	auto loopSet = offsetLoopSet(polygonSet, CGAL::CLOCKWISE);
	cavc::ParallelOffsetIslands<double> alg;
	auto dilated = alg.compute(loopSet, radius);
	auto dilatedReady = reverseLoopSet(dilated);
	auto eroded = alg.compute(dilatedReady, 2 * radius);
	auto erodedReady = reverseLoopSet(eroded);
	auto final = alg.compute(erodedReady, radius);
	return csPolygonSet(final, true);
}

CSPolygonSet approximateSmoothOC(const CSPolygonSet& polygonSet, double radius) {
	auto loopSet = offsetLoopSet(polygonSet, CGAL::COUNTERCLOCKWISE);
	cavc::ParallelOffsetIslands<double> alg;
	auto eroded = alg.compute(loopSet, radius);
	auto erodedReady = reverseLoopSet(eroded);
	auto dilated = alg.compute(erodedReady, 2 * radius);
	auto dilatedReady = reverseLoopSet(dilated);
	auto final = alg.compute(dilatedReady, radius);
	return csPolygonSet(final, false);
}

CSPolygonSet approximateClosing(const CSPolygonSet& polygonSet, double radius) {
    auto loopSet = offsetLoopSet(polygonSet, CGAL::CLOCKWISE);
    cavc::ParallelOffsetIslands<double> alg;
    auto dilated = alg.compute(loopSet, radius);
    auto dilatedReady = reverseLoopSet(dilated);
    auto eroded = alg.compute(dilatedReady, radius);
    return csPolygonSet(eroded, false);
}

CSPolygonSet approximateOpening(const CSPolygonSet& polygonSet, double radius) {
    auto loopSet = offsetLoopSet(polygonSet, CGAL::COUNTERCLOCKWISE);
    cavc::ParallelOffsetIslands<double> alg;
    auto eroded = alg.compute(loopSet, radius);
    auto erodedReady = reverseLoopSet(eroded);
    auto dilated = alg.compute(erodedReady, radius);
    return csPolygonSet(dilated, true);
}

CSPolygonSet approximateDilate(const CSPolygonSet& csPolygonSet, Number<Exact> radius) {
	return approximateDilate(csPolygonSet, CGAL::to_double(radius));
}

CSPolygonSet approximateErode(const CSPolygonSet& csPolygonSet, Number<Exact> radius) {
	return approximateErode(csPolygonSet, CGAL::to_double(radius));
}

CSPolygonSet approximateSmoothCO(const CSPolygonSet& csPolygonSet, Number<Exact> radius) {
	return approximateSmoothCO(csPolygonSet, CGAL::to_double(radius));
}

CSPolygonSet approximateSmoothOC(const CSPolygonSet& csPolygonSet, Number<Exact> radius) {
	return approximateSmoothOC(csPolygonSet, CGAL::to_double(radius));
}

CSPolygonSet approximateClosing(const CSPolygonSet& csPolygonSet, Number<Exact> radius) {
    return approximateClosing(csPolygonSet, CGAL::to_double(radius));
}

CSPolygonSet approximateOpening(const CSPolygonSet& csPolygonSet, Number<Exact> radius) {
    return approximateOpening(csPolygonSet, CGAL::to_double(radius));
}
}
