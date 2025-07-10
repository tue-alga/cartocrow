#include "cs_types.h"

namespace cartocrow {
std::vector<Point<Exact>> makeExact(const std::vector<Point<Inexact>>& points) {
	std::vector<Point<Exact>> exact_points;
	std::transform(points.begin(), points.end(), std::back_inserter(exact_points),
	               [](const Point<Inexact>& pt) { return pretendExact(pt); });
	return exact_points;
}

Point<Inexact> approximateOneRootPoint(const OneRootPoint &algebraic_point) {
	return {CGAL::to_double(algebraic_point.x()), CGAL::to_double(algebraic_point.y())};
}

OneRootPoint pretendOneRootPoint(const Point<Inexact>& point) {
    OneRootNumber x = point.x();
    OneRootNumber y = point.y();
    return {x, y};
}

OneRootPoint pretendOneRootPoint(const Point<Exact>& point) {
    OneRootNumber x = point.x();
    OneRootNumber y = point.y();
    return {x, y};
}
}