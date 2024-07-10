#ifndef CARTOCROW_MOSAIC_CARTOGRAM_GUIDING_SHAPE_H
#define CARTOCROW_MOSAIC_CARTOGRAM_GUIDING_SHAPE_H

#include "../core/core.h"
#include "../core/ellipse.h"
#include "region.h"

namespace cartocrow::mosaic_cartogram {

/// A pair of <em>guiding shapes</em> for two adjacent land regions.
///
/// To determine which tile should be transferred between two regions we use a cost function based
/// on guiding shapes. In particular, for two land regions, we use <em>guiding pairs</em>. These
/// are special, precomputed pairs of guiding shapes that have the correct relative position to each
/// other, based on the input map. Furthermore, their joint centroid is at zero, so they can be
/// easily translated to the joint centroid of the configurations in the tile map.
class GuidingPair {
  public:
	Ellipse ellipse1, ellipse2;

	GuidingPair(const LandRegion &region1, const LandRegion &region2);

	std::pair<Ellipse, Ellipse> translate(const Vector<Inexact> &v) const;
};

} // namespace cartocrow::mosaic_cartogram

#endif // CARTOCROW_MOSAIC_CARTOGRAM_GUIDING_SHAPE_H
