#ifndef CARTOCROW_CS_POLYGON_HELPERS_H
#define CARTOCROW_CS_POLYGON_HELPERS_H

#include "cs_types.h"

namespace cartocrow {
// All area functions in this file are adapted from a Stack Overflow answer by HEKTO.
// Link: https://stackoverflow.com/questions/69399922/how-does-one-obtain-the-area-of-a-general-polygon-set-in-cgal
// License info: https://stackoverflow.com/help/licensing
// The only changes made were changing the auto return type to Number<Inexact> and using the
// typedefs for circle, point, polygon etc.

/// For two circles of radii R and r and centered at (0,0) and (d,0) intersecting
/// in a region shaped like an asymmetric lens.
constexpr double lens_area(const double r, const double R, const double d);

/// Return signed area under the linear segment (P1, P2)
Number<Inexact> area(const ArrCSTraits::Point_2& P1, const ArrCSTraits::Point_2& P2);

/// Return signed area under the circular segment (P1, P2, C)
Number<Inexact> area(const ArrCSTraits::Point_2& P1, const ArrCSTraits::Point_2& P2, const ArrCSTraits::Rational_circle_2& C);

/// Return signed area under the X-monotone curve
Number<Inexact> area(const CSXMCurve& XCV);

/// Return area of the simple polygon
Number<Inexact> area(const CSPolygon& P);

/// Return area of the polygon with (optional) holes
Number<Inexact> area(const CSPolygonWithHoles& P);

/// Convert circle to a CSPolygon
/// Be careful: circles seem to be clockwise by default, so if you are going to compute
/// intersections you probably want to reverse its orientation!
CSPolygon circleToCSPolygon(const Circle<Exact>& circle);
/// Convert a linear polygon to a CSPolygon
CSPolygon polygonToCSPolygon(const Polygon<Exact>& polygon);
/// Convert a linear polygon with holes to a CSPolygonWithHoles
CSPolygonWithHoles polygonToCSPolygon(const PolygonWithHoles<Exact>& polygon);
/// Return the curve in the CSPolygon that point p lies on (if any).
std::optional<CSPolygon::Curve_const_iterator> liesOn(const Point<Exact>& p, const CSPolygon& polygon);
/// Return the curve in the CSPolygon that point p lies on (if any).
std::optional<CSPolygon::Curve_const_iterator> liesOn(const OneRootPoint& p, const CSPolygon& polygon);
/// Return whether the curve is a subset of the boundary of the CSPolygon.
bool liesOn(const CSXMCurve& c, const CSPolygon& polygon);
/// Return whether the point lies on or inside the polygon.
/// The CGAL 2D Regularized Boolean Set-Operations package provides the function bounded_side with similar functionality
/// but that is slower.
bool onOrInside(const CSPolygon& polygon, const Point<Exact>& point);
/// Return whether the point lies within the polygon.
/// The CGAL 2D Regularized Boolean Set-Operations package provides the function bounded_side with similar functionality
/// but that is slower.
bool inside(const CSPolygon& polygon, const Point<Exact>& point);
/// Return whether the point lies on or outside the polygon.
/// The CGAL 2D Regularized Boolean Set-Operations package provides the function bounded_side with similar functionality
/// but that is slower.
bool onOrOutside(const CSPolygon& polygon, const Point<Exact>& point);
/// Return whether the point lies outside the polygon.
/// The CGAL 2D Regularized Boolean Set-Operations package provides the function bounded_side with similar functionality
/// but that is slower.
bool outside(const CSPolygon& polygon, const Point<Exact>& point);
/// Return on which side of the polygon the point lies (bounded, unbounded, or on the boundary).
/// The CGAL 2D Regularized Boolean Set-Operations package provides the function bounded_side with similar functionality
/// but that is slower.
CGAL::Bounded_side bounded_side(const CSPolygon& polygon, const Point<Exact>& point);
/// Convert CSPolygon to CSPolycurve.
CSPolycurve arrPolycurveFromCSPolygon(const CSPolygon& polygon);
/// Return whether CSPolygon is simple (no self-intersections).
bool is_simple(const CSPolygon& pgn);
/// Return the approximate length of the polygon.
Number<Inexact> approximateLength(const CSPolygon& polygon);
}

#endif //CARTOCROW_CS_POLYGON_HELPERS_H
