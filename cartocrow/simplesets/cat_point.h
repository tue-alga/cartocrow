#ifndef CARTOCROW_CAT_POINT_H
#define CARTOCROW_CAT_POINT_H

#include "types.h"

namespace cartocrow::simplesets {
/// Categorical point
struct CatPoint {
	Point<K> point;
	int category;
};
}

#endif //CARTOCROW_CAT_POINT_H
