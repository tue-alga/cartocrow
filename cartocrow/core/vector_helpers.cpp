#include "vector_helpers.h"

namespace cartocrow {
double orientedAngleBetween(Vector<Inexact> v, Vector<Inexact> w, CGAL::Orientation orientation) {
	if (orientation == CGAL::CLOCKWISE) return orientedAngleBetween(w, v, CGAL::COUNTERCLOCKWISE);
	return atan2(v.x() * w.y() - v.y() * w.x(), v.x() * w.x() + v.y() * w.y());
}

Number<Inexact> smallestAngleBetween(const Vector<Inexact>& v, const Vector<Inexact>& w) {
	auto x = (v * w) / (sqrt(v.squared_length()) * sqrt(w.squared_length()));
	return abs(x - 1.0) < M_EPSILON ? 0.0 : acos((v * w) / (sqrt(v.squared_length()) * sqrt(w.squared_length())));
}
}