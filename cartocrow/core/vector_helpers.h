#ifndef CARTOCROW_VECTOR_HELPERS_H
#define CARTOCROW_VECTOR_HELPERS_H

#include "core.h"

namespace cartocrow {
double orientedAngleBetween(Vector<Inexact> v, Vector<Inexact> w, CGAL::Orientation orientation);
double smallestAngleBetween(const Vector<Inexact>& v, const Vector<Inexact>& w);
}

#endif //CARTOCROW_VECTOR_HELPERS_H
