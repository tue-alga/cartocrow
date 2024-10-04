#include "cs_polyline_helpers.h"
#include "cs_curve_helpers.h"

namespace cartocrow::simplesets {
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

renderer::RenderPath renderPath(const CSPolyline& polyline) {
	renderer::RenderPath path;
	bool first = true;
	for (auto cit = polyline.curves_begin(); cit != polyline.curves_end(); ++cit) {
		addToRenderPath(*cit, path, first);
	}
	return path;
}

bool liesOn(const X_monotone_curve_2& c, const CSPolyline& polyline) {
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
	std::vector<X_monotone_curve_2> xm_curves;
	for (auto eit = polyline.edges_begin(); eit != polyline.edges_end(); ++eit) {
		xm_curves.emplace_back(eit->source(), eit->target());
	}
	return {xm_curves.begin(), xm_curves.end()};
}
}