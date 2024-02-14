//
// Created by steven on 2/12/24.
//

#ifndef CARTOCROW_SYMMETRIC_DIFFERENCE_H
#define CARTOCROW_SYMMETRIC_DIFFERENCE_H

#include "types.h"
#include "isoline.h"

namespace cartocrow::isoline_simplification {
double symmetric_difference(const Isoline<K>& original, const Isoline<K>& simplified);
}
#endif //CARTOCROW_SYMMETRIC_DIFFERENCE_H
