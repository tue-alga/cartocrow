#include "cs_polyline_helpers.h"
#include "cs_curve_helpers.h"
#include "rectangle_helpers.h"
#include "vector_helpers.h"
#include <CGAL/Bbox_2.h>

namespace cartocrow {
OneRootPoint nearest(const CSPolyline& polyline, const Point<Exact>& point) {
	std::optional<OneRootNumber> minSqrdDist;
	std::optional<OneRootPoint> closest;
	for (auto cit = polyline.curves_begin(); cit != polyline.curves_end(); ++cit) {
		const auto& curve = *cit;
		auto n = nearest(curve, point);
		auto sqrdDist = CGAL::square(n.x() - point.x()) + CGAL::square(n.y() - point.y());
		if (!minSqrdDist.has_value() || sqrdDist < *minSqrdDist) {
			minSqrdDist = sqrdDist;
			closest = n;
		}
	}

	if (!closest.has_value()) {
		throw std::runtime_error("Cannot find closest point to empty polyline.");
	}

	return *closest;
}

std::optional<CSPolyline::Curve_const_iterator> liesOn(const Point<Exact>& p, const CSPolyline& polyline) {
	for (auto cit = polyline.curves_begin(); cit != polyline.curves_end(); ++cit) {
		if (liesOn(p, *cit)) {
			return cit;
		}
	}
	return std::nullopt;
}

std::optional<CSPolyline::Curve_const_iterator> liesOn(const OneRootPoint& p, const CSPolyline& polyline) {
	for (auto cit = polyline.curves_begin(); cit != polyline.curves_end(); ++cit) {
		if (liesOn(p, *cit)) {
			return cit;
		}
	}
	return std::nullopt;
}

bool liesOn(const CSXMCurve& c, const CSPolyline& polyline) {
	auto sc = liesOn(c.source(), polyline);
	auto tc = liesOn(c.target(), polyline);
	if (!sc.has_value() || !tc.has_value()) {
		return false;
	}
	auto sit = *sc;
	auto tit = *sc;
	auto curr = sit;
	do {
		if (curr->is_linear()) {
			if (c.is_circular()) return false;
			if (curr->supporting_line() != c.supporting_line()) return false;
		} else {
			if (c.is_linear()) return false;
			if (curr->supporting_circle() != c.supporting_circle()) return false;
		}
	} while (curr++ != tit);

	return true;
}

CSPolycurve arrPolycurveFromCSPolyline(const CSPolyline& polyline) {
	return arrPolycurveFromXMCurves(polyline.curves_begin(), polyline.curves_end());
}

CSPolyline polylineToCSPolyline(const Polyline<Exact>& polyline) {
	std::vector<CSXMCurve> xm_curves;
	for (auto eit = polyline.edges_begin(); eit != polyline.edges_end(); ++eit) {
		xm_curves.emplace_back(eit->source(), eit->target());
	}
	return {xm_curves.begin(), xm_curves.end()};
}

struct RationalRadiusCircle {
	Point<Exact> center;
	Number<Exact> radius;
};

Segment<Exact> doStuff(const OneRootPoint& p1, const Point<Exact>& p2, const RationalRadiusCircle& c1, bool idk) {
	const auto& x1 = p1.x();
	const auto& y1 = p1.y();
	const auto& x2 = p2.x();
	const auto& y2 = p2.y();

	auto delta_x = x2 - x1;
	auto delta_y = y2 - y1;
	auto sqr_d = CGAL::square(delta_x) + CGAL::square(delta_y);
	// This is hacky
	Number<Exact> app_delta_x = CGAL::to_double(x2-x1);
	Number<Exact> app_delta_y = CGAL::to_double(y2-y1);
	Number<Exact> app_d = sqrt(CGAL::to_double(sqr_d));

	auto d_app_err = sqr_d - CGAL::square(app_d);
	auto dx_app_err = app_delta_x - delta_x;
	auto dy_app_err = app_delta_y - delta_y;
	auto sign_d_app_err = CGAL::sign(d_app_err);
	auto sign_dx_app_err = CGAL::sign(dx_app_err);
	auto sign_dy_app_err = CGAL::sign(dy_app_err);

	if (sign_d_app_err == CGAL::ZERO && sign_dx_app_err == CGAL::ZERO && sign_dy_app_err == CGAL::ZERO) {
		auto tp1 = Point<Exact> (c1.center.x() + c1.radius * app_delta_y / app_d, c1.center.y() + c1.radius * (-app_delta_x) / app_d);

		auto seg1 = Segment<Exact> (tp1, p2);
		return seg1;
	} else {
		// This is hacky
		if (CGAL::sign(app_delta_x) == CGAL::ZERO) {
			app_delta_x += M_EPSILON;
		}
		if (CGAL::sign(app_delta_y) == CGAL::ZERO) {
			app_delta_y += M_EPSILON;
		}

		bool rotate_pi2 = false;
		if (CGAL::compare (CGAL::abs(delta_x),
		                  CGAL::abs(delta_y)) == CGAL::SMALLER)
		{
			rotate_pi2 = true;

			// Swap the delta_x and delta_y values.
			auto tmp_app = app_delta_x;
			app_delta_x = -app_delta_y;
			app_delta_y = tmp_app;
		}

		Number<Exact> lower_tan_half_phi = (app_d - app_delta_y) / (-app_delta_x);
		Number<Exact> upper_tan_half_phi = (-app_delta_x) / (app_d + app_delta_y);
		if (upper_tan_half_phi < lower_tan_half_phi) {
			auto temp = lower_tan_half_phi;
			lower_tan_half_phi = upper_tan_half_phi;
			upper_tan_half_phi = temp;
		}

		// This is hacky
		lower_tan_half_phi -= M_EPSILON;

		Number<Exact> sqr_tan_half_phi = CGAL::square (lower_tan_half_phi);
		Number<Exact> sin_phi = 2 * lower_tan_half_phi / (1 + sqr_tan_half_phi);
		Number<Exact> cos_phi = (1 - sqr_tan_half_phi) / (1 + sqr_tan_half_phi);

		Point<Exact> tp1;
		if (! rotate_pi2)
		{
			tp1 = Point<Exact> (c1.center.x() + c1.radius*cos_phi, c1.center.y() + c1.radius*sin_phi);
		}
		else
		{
			tp1 = Point<Exact> (c1.center.x() + c1.radius*sin_phi, c1.center.y() - c1.radius*cos_phi);
		}

		if (idk) {
			tp1 = c1.center + (c1.center - tp1);
		}

		auto l = Line<Exact>(c1.center, tp1).perpendicular(tp1);
		Segment<Exact> seg(tp1, l.projection(p2));

		return seg;
	}
}

std::tuple<CSPolyline, Point<Exact>, Point<Exact>> approximateExtend(const CSPolyline& polyline, Number<Inexact> amount, Number<Exact> circleRadius) {

	auto startCurve = *polyline.curves_begin();
	auto endCurve = *(--polyline.curves_end());

	auto sTan = startTangent(startCurve);
	Point<Exact> approxNewSource =
	    pretendExact(approximateAlgebraic(startCurve.source())) - pretendExact(sTan * amount);

	auto tTan = endTangent(*(--polyline.curves_end()));
	Point<Exact> approxNewTarget =
	    pretendExact(approximateAlgebraic(endCurve.target())) + pretendExact(tTan * amount);

	std::vector<CSXMCurve> xmCurves;

	// Single curve
	if (++polyline.curves_begin() == polyline.curves_end()) {
		auto mc = *polyline.curves_begin();
		if (mc.is_linear()) {
			auto l = mc.supporting_line();
			auto newSource = l.projection(approxNewSource);
			OneRootPoint newSourceA(newSource.x(), newSource.y());
			auto newTarget = l.projection(approxNewTarget);
			OneRootPoint newTargetA(newTarget.x(), newTarget.y());
			CSXMCurve extCurve(l, newSourceA, newTargetA);
			xmCurves.push_back(extCurve);
			return {{xmCurves.begin(), xmCurves.end()}, newSource, newTarget};
		} else {
			auto c = startCurve.supporting_circle();
			RationalRadiusCircle ratC(c.center(), circleRadius);
			auto seg1 = doStuff(startCurve.source(), approxNewSource, ratC, true).opposite();
			auto seg2 = doStuff(endCurve.target(), approxNewTarget, ratC, false);
			auto newSource = seg1.source();
			auto newTarget = seg2.target();
			CSXMCurve extStartCurve(seg1.source(), seg1.target());
			xmCurves.push_back(extStartCurve);
			OneRootPoint s1tA(seg1.target().x(), seg1.target().y());
			OneRootPoint s2sA(seg2.source().x(), seg2.source().y());
			CSXMCurve newMc(c, s1tA, s2sA, startCurve.orientation());
			xmCurves.push_back(newMc);
			CSXMCurve extEndCurve(seg2.source(), seg2.target());
			xmCurves.push_back(extEndCurve);
			return {{xmCurves.begin(), xmCurves.end()}, newSource, newTarget};
		}
	}


	Point<Exact> newSource;
	bool skipFirst = false;

	if (startCurve.is_linear()) {
		auto l = startCurve.supporting_line();
		newSource = l.projection(approxNewSource);
		OneRootPoint newSourceA(newSource.x(), newSource.y());
		CSXMCurve extStartCurve(l, newSourceA, startCurve.source());
		xmCurves.push_back(extStartCurve);
	} else {
		auto c = startCurve.supporting_circle();
		RationalRadiusCircle ratC(c.center(), circleRadius);
		auto seg = doStuff(startCurve.source(), approxNewSource, ratC, true).opposite();
		newSource = seg.source();
		CSXMCurve extStartCurve(seg.source(), seg.target());
		xmCurves.push_back(extStartCurve);
		OneRootPoint stA(seg.target().x(), seg.target().y());
		CSXMCurve newStartCurve(c, stA, startCurve.target(), startCurve.orientation());
		xmCurves.push_back(newStartCurve);
		skipFirst = true;
	}

	auto startIt = polyline.curves_begin();
	if (skipFirst) {
		++startIt;
	}
	for (auto cit = startIt; cit != --(polyline.curves_end()); ++cit) {
		xmCurves.push_back(*cit);
	}

	Point<Exact> newTarget;

	if (endCurve.is_linear()) {
		xmCurves.push_back(endCurve);
		auto l = endCurve.supporting_line();
		newTarget = l.projection(approxNewTarget);
		OneRootPoint newTargetA(newTarget.x(), newTarget.y());
		CSXMCurve extEndCurve(l, endCurve.target(), newTargetA);
		xmCurves.push_back(extEndCurve);
	} else {
		auto c = endCurve.supporting_circle();
		RationalRadiusCircle ratC(c.center(), circleRadius);
		auto seg = doStuff(endCurve.target(), approxNewTarget, ratC, false);
		newTarget = seg.target();
		CSXMCurve extEndCurve(seg.source(), seg.target());
		OneRootPoint ssA(seg.source().x(), seg.source().y());
		CSXMCurve newEndCurve(c, endCurve.source(), ssA, endCurve.orientation());
		xmCurves.push_back(newEndCurve);
		xmCurves.push_back(extEndCurve);
	}

	OneRootPoint nsA(newSource.x(), newSource.y());
	OneRootPoint ntA(newTarget.x(), newTarget.y());
	return {{xmCurves.begin(), xmCurves.end()}, newSource, newTarget};
}

CSPolygon closeAroundBB(CSPolyline polyline, CGAL::Orientation orientation, Number<Inexact> offset, const Point<Exact>& source, const Point<Exact>& target) {
	Rectangle<Exact> bb = CGAL::bbox_2(polyline.curves_begin(), polyline.curves_end());
	auto sSide = closest_side(source, bb);
	auto tSide = closest_side(target, bb);
	auto sSideI = static_cast<int>(sSide);
	auto tSideI = static_cast<int>(tSide);

	int dist = abs(sSideI - tSideI);
	if (dist > 2) {
		dist -= 2;
	}

	auto sDir = side_direction<Exact>(sSide);
	auto tDir = side_direction<Exact>(tSide);

	auto sOut = proj_on_side(source, sSide, bb) + sDir * offset;
	auto tOut = proj_on_side(target, tSide, bb) + tDir * offset;

	// The points of the new part that closes the polyline around its bounding box, except source and target.
	std::vector<Point<Exact>> pts({target, tOut});

	switch (dist) {
		case 0: {
		    break;
	    };
		case 1: {
		    std::vector<Point<Exact>> opt1({tOut, get_corner(bb, sSide, tSide) + (sDir + tDir) * offset, sOut});
		    if (CGAL::orientation(opt1[0], opt1[1], opt1[2]) == orientation) {
			    pts.push_back(opt1[1]);
		    } else {
			    // 0 1 -> 1 2 3 0
			    // 1 2 -> 2 3 0 1
			    // 2 3 -> 3 0 1 2
			    // 3 0 -> 0 1 2 3
			    //     3
			    //   -----
			    // 0 |   | 2
			    //   -----
			    //     1

			    auto smaller = static_cast<int>(sSide) < static_cast<int>(tSide) ? sSide : tSide;
			    auto larger = smaller == sSide ? tSide : sSide;
				Side s1;
			    if (larger == next_side(smaller)) {
				    s1 = larger;
			    } else {
				    s1 = smaller;
			    }
				auto s2 = next_side(s1);
				auto s3 = next_side(s2);
				auto s4 = next_side(s3);
				std::vector<Point<Exact>> opt2({
					get_corner(bb, s1, s2) + (side_direction<Exact>(s1) + side_direction<Exact>(s2)) * offset,
					get_corner(bb, s2, s3) + (side_direction<Exact>(s2) + side_direction<Exact>(s3)) * offset,
					get_corner(bb, s3, s4) + (side_direction<Exact>(s3) + side_direction<Exact>(s4)) * offset,
				});
			    if (s1 == tSide) {
				    std::copy(opt2.begin(), opt2.end(), std::back_inserter(pts));
			    } else {
					std::copy(opt2.rbegin(), opt2.rend(), std::back_inserter(pts));
			    }
		    }
		    break;
		}
		case 2: {
			auto betweenSide = orientation == CGAL::RIGHT_TURN ? next_side(sSide) : next_side(tSide);
		    auto betweenDir = side_direction<Exact>(betweenSide);
		    pts.push_back(get_corner(bb, betweenSide, tSide) + (betweenDir + tDir) * offset);
			pts.push_back(get_corner(bb, sSide, betweenSide) + (sDir + betweenDir) * offset);
		    break;
		}
		default: {
			throw std::runtime_error("Impossible");
		}
	}
	pts.push_back(sOut);
	pts.push_back(source);

	std::vector<CSXMCurve> xm_curves;
	std::copy(polyline.curves_begin(), polyline.curves_end(), std::back_inserter(xm_curves));
	for (int i = 1; i < pts.size(); ++i) {
		xm_curves.emplace_back(pts[i-1], pts[i]);
	}

	return {xm_curves.begin(), xm_curves.end()};
}

double approximateAbsoluteTurningAngle(const CSPolyline& polyline) {
	auto total = abs(approximateTurningAngle(*polyline.curves_begin()));
	for (auto cit = ++polyline.curves_begin(); cit != polyline.curves_end(); ++cit) {
		auto prev = cit;
		--prev;
		auto t1 = endTangent(*prev);
		auto t2 = startTangent(*cit);
		total += smallestAngleBetween(t1, t2);
		total += abs(approximateTurningAngle(*cit));
	}
	return total;
}
}