#include "cat_point.h"

namespace cartocrow::simplesets {
std::ostream& operator<<(std::ostream& os, CatPoint const& catPoint) {
	return os << catPoint.category << " " << catPoint.point;
}
}