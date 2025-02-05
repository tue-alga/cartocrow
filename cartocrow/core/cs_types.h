#ifndef CARTOCROW_CS_TYPES_H
#define CARTOCROW_CS_TYPES_H

#include "core.h"
#include "general_polyline.h"
#include <CGAL/Cartesian.h>
#include <CGAL/Arr_circle_segment_traits_2.h>
#include <CGAL/CORE_algebraic_number_traits.h>
#include <CGAL/Gps_traits_2.h>
#include <CGAL/Gps_circle_segment_traits_2.h>
#include <CGAL/Arr_polycurve_traits_2.h>

namespace cartocrow {
typedef CGAL::Arr_circle_segment_traits_2<Exact> ArrCSTraits;
typedef CGAL::Gps_circle_segment_traits_2<Exact> GpsCSTraits;
typedef CGAL::Arr_polycurve_traits_2<ArrCSTraits> PolycurveCSTraits;
typedef GpsCSTraits::Polygon_2 CSPolygon;
typedef GpsCSTraits::Polygon_with_holes_2 CSPolygonWithHoles;
typedef CGAL::General_polygon_set_2<GpsCSTraits> CSPolygonSet;
typedef General_polyline_2<ArrCSTraits> CSPolyline;
typedef PolycurveCSTraits::Curve_2 CSPolycurve;
typedef PolycurveCSTraits::X_monotone_curve_2 CSXMPolycurve;
typedef CGAL::Arrangement_2<ArrCSTraits> CSArrangement;

typedef ArrCSTraits::X_monotone_curve_2 CSXMCurve;
typedef ArrCSTraits::Curve_2 CSCurve;
typedef ArrCSTraits::CoordNT OneRootNumber;
typedef ArrCSTraits::Point_2 OneRootPoint;

Point<Inexact> approximateAlgebraic(const OneRootPoint& algebraic_point);
}

#endif //CARTOCROW_CS_TYPES_H
