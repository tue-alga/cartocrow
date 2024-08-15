#ifndef CARTOCROW_TYPES_H
#define CARTOCROW_TYPES_H

#include "../core/core.h"
#include <CGAL/Cartesian.h>
#include <CGAL/Arr_circle_segment_traits_2.h>
#include <CGAL/Arr_conic_traits_2.h>
#include <CGAL/CORE_algebraic_number_traits.h>
#include <CGAL/Gps_traits_2.h>
#include <CGAL/Gps_circle_segment_traits_2.h>

namespace cartocrow::simplesets {
//typedef Exact K;
typedef CGAL::Arr_circle_segment_traits_2<Exact> CSTraits;
typedef CGAL::Gps_circle_segment_traits_2<Exact> CSTraitsBoolean;
typedef CSTraitsBoolean::Polygon_2 CSPolygon;
typedef CSTraitsBoolean::Polygon_with_holes_2 CSPolygonWithHoles;
typedef CGAL::Arrangement_2<CSTraits> CSArrangement;
//typedef CGAL::CORE_algebraic_number_traits Nt_traits;
//typedef CGAL::Cartesian<Nt_traits::Rational> Rat_kernel;
//typedef Nt_traits::Algebraic Algebraic;
//typedef CGAL::Cartesian<Algebraic> Alg_kernel;
//typedef CGAL::Arr_conic_traits_2<Rat_kernel, Alg_kernel, Nt_traits> ConicTraits;
//typedef CGAL::Gps_traits_2<ConicTraits> Gps_traits;
//typedef Alg_kernel K;

Point<Exact> makeExact(const Point<Inexact>& point);
std::vector<Point<Exact>> makeExact(const std::vector<Point<Inexact>>& points);
Polygon<Exact> makeExact(const Polygon<Inexact>& polygon);
Point<Inexact> approximateAlgebraic(const CSTraits::Point_2& algebraic_point);
}

#endif //CARTOCROW_TYPES_H
