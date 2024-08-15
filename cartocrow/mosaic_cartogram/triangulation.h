#ifndef CARTOCROW_MOSAIC_CARTOGRAM_TRIANGULATION_H
#define CARTOCROW_MOSAIC_CARTOGRAM_TRIANGULATION_H

#include <vector>

#include "../core/region_arrangement.h"

namespace cartocrow::mosaic_cartogram {

/// "Triangulates" \c arr such that its dual is maximal planar. This is achieved by dividing the
/// ocean into sea regions. To this end, we compute a straight skeleton in the ocean, and use the
/// bisectors ending in salient points as sea boundaries.
///
/// The number of sea regions (excluding the three outer ones) is returned.
///
/// The current implementation is not particularly efficient. Furthermore, the flow algorithm in
/// \ref HexagonalMap likes to deal with convex-ish regions, but this currently produces many sea
/// regions with sharp angles etc.
int triangulate(RegionArrangement &arr, const std::vector<Point<Exact>> &salientPoints);

/// TODO
bool dualIsTriangular(const RegionArrangement &arr);

} // namespace cartocrow::mosaic_cartogram

#endif // CARTOCROW_MOSAIC_CARTOGRAM_TRIANGULATION_H
