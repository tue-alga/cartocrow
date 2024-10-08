#ifndef CARTOCROW_TYPES_H
#define CARTOCROW_TYPES_H

#include "../core/core.h"
#include <CGAL/Cartesian.h>
#include <CGAL/Arr_circle_segment_traits_2.h>
#include <CGAL/Arr_conic_traits_2.h>
#include <CGAL/CORE_algebraic_number_traits.h>
#include <CGAL/Gps_traits_2.h>
#include <CGAL/Gps_circle_segment_traits_2.h>
#include <CGAL/Arr_polycurve_traits_2.h>

#include "general_polyline.h"

namespace cartocrow::simplesets {
typedef CGAL::Arr_circle_segment_traits_2<Exact> CSTraits;
typedef CGAL::Gps_circle_segment_traits_2<Exact> CSTraitsBoolean;
typedef CGAL::Arr_polycurve_traits_2<CSTraits> PolyCSTraits;
typedef CSTraitsBoolean::Polygon_2 CSPolygon;
typedef CSTraitsBoolean::Polygon_with_holes_2 CSPolygonWithHoles;
typedef CGAL::General_polygon_set_2<CSTraitsBoolean> CSPolygonSet;
typedef General_polyline_2<CSTraits> CSPolyline;
typedef PolyCSTraits::Curve_2 CSPolycurve;
typedef PolyCSTraits::X_monotone_curve_2 CSPolycurveXM;
typedef CGAL::Arrangement_2<CSTraits> CSArrangement;
typedef CSTraits::X_monotone_curve_2 X_monotone_curve_2;
typedef CSTraits::Curve_2 Curve_2;
typedef CSTraits::CoordNT OneRootNumber;
typedef CSTraits::Point_2 OneRootPoint;

Point<Exact> makeExact(const Point<Inexact>& point);
Circle<Exact> makeExact(const Circle<Inexact>& circle);
std::vector<Point<Exact>> makeExact(const std::vector<Point<Inexact>>& points);
Polygon<Exact> makeExact(const Polygon<Inexact>& polygon);
Point<Inexact> approximateAlgebraic(const CSTraits::Point_2& algebraic_point);

template <class T>
T squared(T x) {
	return x * x;
}
}

#endif //CARTOCROW_TYPES_H
