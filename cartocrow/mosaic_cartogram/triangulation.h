#ifndef CARTOCROW_MOSAIC_CARTOGRAM_TRIANGULATION_H
#define CARTOCROW_MOSAIC_CARTOGRAM_TRIANGULATION_H

#include <vector>

#include "../core/region_arrangement.h"

namespace cartocrow::mosaic_cartogram {

/// "Triangulates" \c arr such that its dual is maximal planar. This is mainly achieved by dividing
/// the "ocean" into sea regions. The number of sea regions (excluding the three outer ones) is
/// returned.
int triangulate(RegionArrangement &arr, const std::vector<Point<Exact>> &salientPoints);

/// TODO
bool dualIsTriangular(const RegionArrangement &arr);

} // namespace cartocrow::mosaic_cartogram

#endif // CARTOCROW_MOSAIC_CARTOGRAM_TRIANGULATION_H
