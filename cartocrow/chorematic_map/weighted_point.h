#ifndef CARTOCROW_WEIGHTED_POINT_H
#define CARTOCROW_WEIGHTED_POINT_H

#include "../core/core.h"

namespace cartocrow::chorematic_map {
struct WeightedPoint {
	Point<Inexact> point;
	Number<Inexact> weight;
};

using InducedDisk = std::tuple<std::optional<Point<Inexact>>, std::optional<Point<Inexact>>, std::optional<Point<Inexact>>>;
using InducedDiskW = std::tuple<std::optional<WeightedPoint>, std::optional<WeightedPoint>, std::optional<WeightedPoint>>;
}

#endif //CARTOCROW_WEIGHTED_POINT_H
