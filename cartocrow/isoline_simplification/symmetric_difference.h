//
// Created by steven on 2/12/24.
//

#ifndef CARTOCROW_SYMMETRIC_DIFFERENCE_H
#define CARTOCROW_SYMMETRIC_DIFFERENCE_H

#include "types.h"
#include "isoline.h"

namespace cartocrow::isoline_simplification {
double symmetric_difference(const Isoline<K>& isoline1, const Isoline<K>& isoline2);
}
#endif //CARTOCROW_SYMMETRIC_DIFFERENCE_H
