#ifndef CARTOCROW_SIMPLESETS_TYPES_H
#define CARTOCROW_SIMPLESETS_TYPES_H

#include "../core/core.h"
#include "cartocrow/circle_segment_helpers/cs_types.h"
#include <CGAL/Arr_circle_segment_traits_2.h>
#include <CGAL/Arr_conic_traits_2.h>
#include <CGAL/Arr_polycurve_traits_2.h>
#include <CGAL/CORE_algebraic_number_traits.h>
#include <CGAL/Cartesian.h>
#include <CGAL/Gps_circle_segment_traits_2.h>
#include <CGAL/Gps_traits_2.h>

#include "cartocrow/core/general_polyline.h"

namespace cartocrow::simplesets {
template <class T>
T squared(T x) {
	return x * x;
}
}

#endif //CARTOCROW_SIMPLESETS_TYPES_H
