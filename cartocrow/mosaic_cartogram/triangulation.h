#ifndef CARTOCROW_MOSAIC_CARTOGRAM_TRIANGULATION_H
#define CARTOCROW_MOSAIC_CARTOGRAM_TRIANGULATION_H

#include <vector>

#include "../core/region_arrangement.h"

namespace cartocrow::mosaic_cartogram {

/// "Triangulates" \c arrOrig such that its dual is maximal planar.
RegionArrangement triangulate(const RegionArrangement &arrOrig, const std::vector<Point<Exact>> &salientPoints);

/// TODO
bool dualIsTriangular(const RegionArrangement &arr);

} // namespace cartocrow::mosaic_cartogram

#endif // CARTOCROW_MOSAIC_CARTOGRAM_TRIANGULATION_H
