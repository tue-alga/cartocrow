#ifndef CARTOCROW_DILATED_POLY_H
#define CARTOCROW_DILATED_POLY_H

#include "../patterns/pattern.h"

namespace cartocrow::simplesets {
CSPolygon dilatePattern(const Pattern& pattern, const Number<Inexact>& dilationRadius);
}
#endif //CARTOCROW_DILATED_POLY_H
