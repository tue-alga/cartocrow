#ifndef CARTOCROW_MOSAIC_CARTOGRAM_GUIDING_SHAPE_H
#define CARTOCROW_MOSAIC_CARTOGRAM_GUIDING_SHAPE_H

#include "../core/core.h"
#include "../core/ellipse.h"
#include "region.h"

namespace cartocrow::mosaic_cartogram {

class GuidingPair {
  public:
	Ellipse ellipse1, ellipse2;

	GuidingPair(const LandRegion &region1, const LandRegion &region2);

	std::pair<Ellipse, Ellipse> translate(const Vector<Inexact> &v) const;
};

} // namespace cartocrow::mosaic_cartogram

#endif // CARTOCROW_MOSAIC_CARTOGRAM_GUIDING_SHAPE_H
