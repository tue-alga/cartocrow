#ifndef CARTOCROW_CS_POLYGON_HELPERS_H
#define CARTOCROW_CS_POLYGON_HELPERS_H

#include "../types.h"

// Thanks to: https://stackoverflow.com/questions/69399922/how-does-one-obtain-the-area-of-a-general-polygon-set-in-cgal

namespace cartocrow::simplesets {
//For two circles of radii R and r and centered at (0,0) and (d,0) intersecting
//in a region shaped like an asymmetric lens.
constexpr double lens_area(const double r, const double R, const double d);

// ------ return signed area under the linear segment (P1, P2)
Number<Inexact> area(const CSTraits::Point_2& P1, const CSTraits::Point_2& P2);

// ------ return signed area under the circular segment (P1, P2, C)
Number<Inexact> area(const CSTraits::Point_2& P1, const CSTraits::Point_2& P2, const CSTraits::Rational_circle_2& C);

// ------ return signed area under the X-monotone curve
Number<Inexact> area(const CSTraits::X_monotone_curve_2& XCV);

// ------ return area of the simple polygon
Number<Inexact> area(const CSPolygon P);

// ------ return area of the polygon with (optional) holes
Number<Inexact> area(const CSPolygonWithHoles& P);
}

#endif //CARTOCROW_CS_POLYGON_HELPERS_H