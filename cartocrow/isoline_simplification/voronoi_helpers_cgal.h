#ifndef CARTOCROW_VORONOI_HELPERS_CGAL_H
#define CARTOCROW_VORONOI_HELPERS_CGAL_H

#include "types.h"

namespace cartocrow::isoline_simplification {
CGAL::Sign incircle(const SDG2& sdg, const SDG2::Face_handle& f, const SDG2::Site_2& q);

Gt::Arrangement_type_2::result_type arrangement_type(const SDG2& sdg, const SDG2::Site_2& p,
                                                     const SDG2::Site_2& q);
}
#endif //CARTOCROW_VORONOI_HELPERS_CGAL_H
