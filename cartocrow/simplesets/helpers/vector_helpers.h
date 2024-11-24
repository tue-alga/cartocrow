#ifndef CARTOCROW_VECTOR_HELPERS_H
#define CARTOCROW_VECTOR_HELPERS_H

#include "../../core/core.h"

namespace cartocrow::simplesets {
double orientedAngleBetween(Vector<Inexact> v, Vector<Inexact> w, CGAL::Orientation orientation);
double smallestAngleBetween(const Vector<Inexact>& v, const Vector<Inexact>& w);
}

#endif //CARTOCROW_VECTOR_HELPERS_H
