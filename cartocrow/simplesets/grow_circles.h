#ifndef CARTOCROW_GROW_CIRCLES_H
#define CARTOCROW_GROW_CIRCLES_H

#include "../core/core.h"

namespace cartocrow::simplesets {
struct GrowingCircle {
	Point<Exact> center;
	Number<Exact> squaredRadius;
	bool frozen = false;
};

std::pair<std::vector<Circle<Exact>>, std::vector<Circle<Exact>>>
approximateGrowCircles(const std::vector<Point<Exact>>& points1, const std::vector<Point<Exact>>& points2,
            const Number<Exact>& maxSquaredRadius1, const Number<Exact>& maxSquaredRadius2);
}
#endif //CARTOCROW_GROW_CIRCLES_H
