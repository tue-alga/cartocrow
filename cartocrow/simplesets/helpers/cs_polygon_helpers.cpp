#include "cs_polygon_helpers.h"

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
	return area(P1, P2) + C.orientation() * areaCircularSegment;
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
Number<Inexact> area(const CSPolygon P) {
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
}