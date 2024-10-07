#ifndef CARTOCROW_CS_POLYGON_HELPERS_H
#define CARTOCROW_CS_POLYGON_HELPERS_H

#include "../types.h"
#include "../../renderer/render_path.h"

namespace cartocrow::simplesets {
//For two circles of radii R and r and centered at (0,0) and (d,0) intersecting
//in a region shaped like an asymmetric lens.
constexpr double lens_area(const double r, const double R, const double d);

// ------ return signed area under the linear segment (P1, P2)
Number<Inexact> area(const CSTraits::Point_2& P1, const CSTraits::Point_2& P2);

// ------ return signed area under the circular segment (P1, P2, C)
Number<Inexact> area(const CSTraits::Point_2& P1, const CSTraits::Point_2& P2, const CSTraits::Rational_circle_2& C);

// ------ return signed area under the X-monotone curve
Number<Inexact> area(const X_monotone_curve_2& XCV);

// ------ return area of the simple polygon
Number<Inexact> area(const CSPolygon& P);

// ------ return area of the polygon with (optional) holes
Number<Inexact> area(const CSPolygonWithHoles& P);

CSPolygon circleToCSPolygon(const Circle<Exact>& circle);

std::optional<CSPolygon::Curve_const_iterator> liesOn(const Point<Exact>& p, const CSPolygon& polygon);
std::optional<CSPolygon::Curve_const_iterator> liesOn(const OneRootPoint& p, const CSPolygon& polygon);
bool liesOn(const X_monotone_curve_2& c, const CSPolygon& polygon);
renderer::RenderPath renderPath(const CSPolygon& polygon);
renderer::RenderPath renderPath(const CSPolygonWithHoles& withHoles);
bool on_or_inside(const CSPolygon& polygon, const Point<Exact>& point);
bool inside(const CSPolygon& polygon, const Point<Exact>& point);
CSPolycurve arrPolycurveFromCSPolygon(const CSPolygon& polygon);
Polygon<Exact> linearSample(const CSPolygon& polygon, int n);
CSPolygonWithHoles approximateDilate(const CSPolygon& polygon, double r, double eps, int n);
}

#endif //CARTOCROW_CS_POLYGON_HELPERS_H
