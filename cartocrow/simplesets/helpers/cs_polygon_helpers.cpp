#include "cs_polygon_helpers.h"
#include "cs_curve_helpers.h"
#include <CGAL/approximated_offset_2.h>

// Code adapted from a Stack Overflow answer by HEKTO.
// Link: https://stackoverflow.com/questions/69399922/how-does-one-obtain-the-area-of-a-general-polygon-set-in-cgal
// License info: https://stackoverflow.com/help/licensing
// The only changes made were changing the auto return type to Number<Inexact> and using the
// typedefs for circle, point, polygon etc.

namespace cartocrow::simplesets {
//For two circles of radii R and r and centered at (0,0) and (d,0) intersecting
//in a region shaped like an asymmetric lens.
constexpr double lens_area(const double r, const double R, const double d) {
	return r * r * std::acos((d * d + r * r - R * R) / 2 / d / r) +
	       R * R * std::acos((d * d + R * R - r * r) / 2 / d / R) -
	       0.5 * std::sqrt((-d + r + R) * (d + r - R) * (d - r + R) * (d + r + R));
}

// ------ return signed area under the linear segment (P1, P2)
Number<Inexact> area(const CSTraits::Point_2& P1, const CSTraits::Point_2& P2) {
	auto const dx = CGAL::to_double(P1.x()) - CGAL::to_double(P2.x());
	auto const sy = CGAL::to_double(P1.y()) + CGAL::to_double(P2.y());
	return dx * sy / 2;
}

// ------ return signed area under the circular segment (P1, P2, C)
Number<Inexact> area(const CSTraits::Point_2& P1, const CSTraits::Point_2& P2, const CSTraits::Rational_circle_2& C) {
	auto const dx = CGAL::to_double(P1.x()) - CGAL::to_double(P2.x());
	auto const dy = CGAL::to_double(P1.y()) - CGAL::to_double(P2.y());
	auto const squaredChord = dx * dx + dy * dy;
	auto const chord = std::sqrt(squaredChord);
	auto const squaredRadius = CGAL::to_double(C.squared_radius());
	auto const areaSector =
	    squaredRadius * std::asin(std::min(1.0, chord / (std::sqrt(squaredRadius) * 2)));
	auto const areaTriangle = chord * std::sqrt(std::max(0.0, squaredRadius * 4 - squaredChord)) / 4;
	auto const areaCircularSegment = areaSector - areaTriangle;
	int sign;
	if (C.orientation() == CGAL::Sign::NEGATIVE) {
		sign = -1;
	} else if (C.orientation() == CGAL::Sign::POSITIVE) {
		sign = 1;
	} else {
		assert(C.orientation() == CGAL::Sign::ZERO);
		sign = 0;
	}
	return area(P1, P2) + sign * areaCircularSegment;
}

// ------ return signed area under the X-monotone curve
Number<Inexact> area(const X_monotone_curve_2& XCV) {
	if (XCV.is_linear()) {
		return area(XCV.source(), XCV.target());
	} else if (XCV.is_circular()) {
		return area(XCV.source(), XCV.target(), XCV.supporting_circle());
	} else {
		return 0;
	}
}

// ------ return area of the simple polygon
Number<Inexact> area(const CSPolygon& P) {
	Number<Inexact> res = 0;
	for (auto it = P.curves_begin(); it != P.curves_end(); ++it) {
		res += area(*it);
	}
	return res;
}

// ------ return area of the polygon with (optional) holes
Number<Inexact> area(const CSPolygonWithHoles& P) {
	auto res = area(P.outer_boundary());
	for (auto it = P.holes_begin(); it != P.holes_end(); ++it) {
		res += area(*it);
	}
	return res;
}

CSPolygon circleToCSPolygon(const Circle<Exact>& circle) {
	std::vector<X_monotone_curve_2> xm_curves;
	curveToXMonotoneCurves(circle, std::back_inserter(xm_curves));
	return {xm_curves.begin(), xm_curves.end()};
}

std::optional<CSPolygon::Curve_const_iterator> liesOn(const Point<Exact>& p, const CSPolygon& polygon) {
	for (auto cit = polygon.curves_begin(); cit != polygon.curves_end(); ++cit) {
		if (liesOn(p, *cit)) {
			return cit;
		}
	}
	return std::nullopt;
}

std::optional<CSPolygon::Curve_const_iterator> liesOn(const OneRootPoint& p, const CSPolygon& polygon) {
	for (auto cit = polygon.curves_begin(); cit != polygon.curves_end(); ++cit) {
		if (liesOn(p, *cit)) {
			return cit;
		}
	}
	return std::nullopt;
}

renderer::RenderPath operator<<(renderer::RenderPath& path, const CSPolygon& polygon) {
	bool first = true;
	std::vector<Curve_2> mergedCurves;
	toCurves(polygon.curves_begin(), polygon.curves_end(), std::back_inserter(mergedCurves));
	for (const auto& c : mergedCurves) {
		addToRenderPath(c, path, first);
	}
	if (!holds_alternative<renderer::RenderPath::Close>(path.commands().back())) {
		path.close();
	}
	return path;
}

renderer::RenderPath renderPath(const CSPolygon& polygon) {
	renderer::RenderPath path;
	path << polygon;
	return path;
}

renderer::RenderPath renderPath(const CSPolygonWithHoles& withHoles) {
	renderer::RenderPath path;
	path << withHoles.outer_boundary();
	for (auto hit = withHoles.holes_begin(); hit != withHoles.holes_end(); ++hit) {
		path << *hit;
	}
	return path;
}

bool on_or_inside(const CSPolygon& polygon, const Point<Exact>& point) {
	Ray<Exact> ray(point, Vector<Exact>(1, 0));

	Rectangle<Exact> bbox = polygon.bbox();
	Rectangle<Exact> rect({bbox.xmin() - 1, bbox.ymin() - 1}, {bbox.xmax() + 1, bbox.ymax() + 1});

	auto inter = CGAL::intersection(ray, rect);
	if (!inter.has_value()) return false;
	if (inter->type() == typeid(Point<Exact>)) return true;
	auto seg = boost::get<Segment<Exact>>(*inter);
	X_monotone_curve_2 seg_xm_curve(seg.source(), seg.target());

	typedef std::pair<CSTraits::Point_2, unsigned int> Intersection_point;
	typedef boost::variant<Intersection_point, X_monotone_curve_2> Intersection_result;
	std::vector<Intersection_result> intersection_results;

	for (auto cit = polygon.curves_begin(); cit != polygon.curves_end(); ++cit) {
		const auto& curve = *cit;
		curve.intersect(seg_xm_curve, std::back_inserter(intersection_results));
	}

	int count = 0;
	for (const auto& ir : intersection_results) {
		if (ir.which() == 0) {
			auto ip = get<Intersection_point>(ir);
			//(ir) Intersection points are double-counted, so increase count by half.
			bool found = false;
			for (auto cit = polygon.curves_begin(); cit != polygon.curves_end(); ++cit) {
				if (cit->source() == ip.first) {
					found = true;
					break;
				}
			}
			if (found) {
				count += 1;
				continue;
			}
		}
		count += 2;
	}

	return count % 4 != 0;
}

bool liesOn(const X_monotone_curve_2& c, const CSPolygon& polygon) {
	auto sc = liesOn(c.source(), polygon);
	auto tc = liesOn(c.target(), polygon);
	if (!sc.has_value() || !tc.has_value()) {
		return false;
	}
	do {
		auto next = *sc;
		++next;
		if (next == polygon.curves_end()) {
			next = polygon.curves_begin();
		}
		if (liesOn(c.source(), *next)) {
			sc = next;
		} else {
			break;
		}
	} while (true);
	do {
		auto next = *tc;
		++next;
		if (next == polygon.curves_end()) {
			next = polygon.curves_begin();
		}
		if (liesOn(c.source(), *next)) {
			tc = next;
		} else {
			break;
		}
	} while (true);
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

bool inside(const CSPolygon& polygon, const Point<Exact>& point) {
    return on_or_inside(polygon, point) && !liesOn(point, polygon);
}

CSPolycurve arrPolycurveFromCSPolygon(const CSPolygon& polygon) {
	return arrPolycurveFromXMCurves(polygon.curves_begin(), polygon.curves_end());
}

Polygon<Exact> linearSample(const CSPolygon& polygon, int n) {
    std::vector<std::pair<double, double>> coords;
	polygon.approximate(std::back_inserter(coords), n);
	std::vector<Point<Exact>> points;

	// polygon.approximate returns duplicate points.
	for (int i = 0; i < coords.size(); ++i) {
		auto p = coords[i];
		if (i > 0) {
			auto prev = coords[i - 1];
			if (p == prev) continue;
		}
		if (i == coords.size() - 1 && p == coords[0]) {
			continue;
		}
		points.emplace_back(p.first, p.second);
	}
	return {points.begin(), points.end()};
}

CSPolygonWithHoles approximateDilate(const CSPolygon& polygon, double r, double eps, int n) {
	auto poly = linearSample(polygon, n);
	return CGAL::approximated_offset_2(poly, r, eps);
}
}