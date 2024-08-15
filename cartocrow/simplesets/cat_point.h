#ifndef CARTOCROW_CAT_POINT_H
#define CARTOCROW_CAT_POINT_H

#include "types.h"

namespace cartocrow::simplesets {
/// Categorical point
struct CatPoint {
	unsigned int category;
	Point<Inexact> point;
	auto operator<=>(const CatPoint&) const = default;
};
}

#endif //CARTOCROW_CAT_POINT_H
