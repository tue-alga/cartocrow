#ifndef CARTOCROW_APPROXIMATE_CONVEX_HULL_H
#define CARTOCROW_APPROXIMATE_CONVEX_HULL_H

#include "cs_types.h"

namespace cartocrow {
CSPolygon approximateConvexHull(const std::vector<Circle<Exact>>& circles);
}

#endif //CARTOCROW_APPROXIMATE_CONVEX_HULL_H
