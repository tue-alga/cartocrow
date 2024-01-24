//
// Created by steven on 1/23/24.
//

#ifndef CARTOCROW_COLLAPSE_H
#define CARTOCROW_COLLAPSE_H

#include "types.h"

namespace cartocrow::isoline_simplification {

Gt::Line_2 area_preservation_line(Gt::Point_2 s, Gt::Point_2 t, Gt::Point_2 u, Gt::Point_2 v);
std::vector<Gt::Point_2> collapse(SlopeLadder& ladder, const PointToPoint& p_prev,
								  const PointToPoint& p_next);
}
#endif //CARTOCROW_COLLAPSE_H
