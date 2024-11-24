#ifndef CARTOCROW_CS_POLYLINE_HELPERS_H
#define CARTOCROW_CS_POLYLINE_HELPERS_H

#include "../types.h"
#include "cartocrow/renderer/render_path.h"
#include <CGAL/Arr_polycurve_traits_2.h>

namespace cartocrow::simplesets {
OneRootPoint nearest(const CSPolyline& polyline, const Point<Exact>& point);
std::optional<CSPolyline::Curve_const_iterator> liesOn(const Point<Exact>& p, const CSPolyline& polyline);
std::optional<CSPolyline::Curve_const_iterator> liesOn(const OneRootPoint& p, const CSPolyline& polyline);
bool liesOn(const X_monotone_curve_2& c, const CSPolyline& polyline);
renderer::RenderPath renderPath(const CSPolyline& polyline);
CSPolycurve arrPolycurveFromCSPolyline(const CSPolyline& polyline);
CSPolyline polylineToCSPolyline(const Polyline<Exact>& polyline);
std::tuple<CSPolyline, Point<Exact>, Point<Exact>> extend(const CSPolyline& polyline, Number<Inexact> amount, Number<Exact> dilationRadius);
CSPolygon closeAroundBB(CSPolyline polyline, CGAL::Orientation orientation, Number<Inexact> offset, const Point<Exact>& source, const Point<Exact>& target);
double approximateAbsoluteTurningAngle(const CSPolyline& polyline);
}

#endif //CARTOCROW_CS_POLYLINE_HELPERS_H
