#ifndef CARTOCROW_CAT_POINT_H
#define CARTOCROW_CAT_POINT_H

#include "types.h"

namespace cartocrow::simplesets {
/// Categorical point
struct CatPoint {
	unsigned int category;
	Point<Inexact> point;
	CatPoint(unsigned int category, Point<Inexact> point) : category(category), point(point) {};
	bool operator==(const CatPoint&) const = default;
};

std::ostream& operator<<(std::ostream& os, CatPoint const& catPoint);
}

#endif //CARTOCROW_CAT_POINT_H
