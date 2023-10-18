#ifndef CARTOCROW_MOSAIC_CARTOGRAM_GUIDING_SHAPE_H
#define CARTOCROW_MOSAIC_CARTOGRAM_GUIDING_SHAPE_H

#include "ellipse.h"
#include "parameters.h"
#include "region.h"

namespace cartocrow::mosaic_cartogram {

class GuidingPair {
  public:
	Ellipse ellipse1, ellipse2;

	GuidingPair(const LandRegion &region1, const LandRegion &region2, const Parameters &params);

	std::pair<Ellipse, Ellipse> translate(const double dx, const double dy) const;
};

} // namespace cartocrow::mosaic_cartogram

#endif // CARTOCROW_MOSAIC_CARTOGRAM_GUIDING_SHAPE_H
