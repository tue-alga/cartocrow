#include "guiding_shape.h"

#include <stdexcept>
#include <CGAL/number_utils.h>
#include <CGAL/Origin.h>

#include "../core/centroid.h"
#include "../core/core.h"

namespace cartocrow::mosaic_cartogram {

GuidingPair::GuidingPair(const LandRegion &region1, const LandRegion &region2, const Parameters &params) {
	const Point<Exact> centroid1(centroid(region1.shape));
	const Point<Exact> centroid2(centroid(region2.shape));

	// slope of line through the centroids  (TODO: what if infinite?)
	const double m = CGAL::to_double((centroid1.y() - centroid2.y()) / (centroid1.x() - centroid2.x()));
	// Δx per unit of distance between the centroids
	const double dx = 1 / std::sqrt(1 + m*m);

	const EllipseAtOrigin *guide1 = &region1.guidingShape;
	const EllipseAtOrigin *guide2 = &region2.guidingShape;

	// ensure that `guide1` corresponds to the region on the left (and `guide2` right)
	if (centroid1.x() > centroid2.x()) std::swap(guide1, guide2);

	// move guiding shapes along  y = m x
	// such that they overlap the diameter of one tile w.r.t. this line
	// TODO: yields undesirable results if guiding shapes are small compared to a tile
	const double dx1 = -(guide1->radius(m) - params.tileRadius) * dx;
	const double dx2 =  (guide2->radius(m) - params.tileRadius) * dx;
	ellipse1 = guide1->translate(dx1, m * dx1);
	ellipse2 = guide2->translate(dx2, m * dx2);

	// restore correspondence
	if (centroid1.x() > centroid2.x()) {
		std::swap(guide1, guide2);
		std::swap(ellipse1, ellipse2);
	}

	const double area1 = guide1->area();
	const double area2 = guide2->area();

	// opposite of "joint" center  (note: overlap is counted twice)
	const auto correction = -(area1 * (ellipse1.center() - CGAL::ORIGIN)
	                        + area2 * (ellipse2.center() - CGAL::ORIGIN)) / (area1 + area2);

	// move guiding shapes such that their joint center is at the origin
	ellipse1 = ellipse1.translate(correction.x(), correction.y());
	ellipse2 = ellipse2.translate(correction.x(), correction.y());
}

std::pair<Ellipse, Ellipse> GuidingPair::translate(const double dx, const double dy) const {
	return {
		ellipse1.translate(dx, dy).normalizeSign(),
		ellipse2.translate(dx, dy).normalizeSign()
	};
}

} // namespace cartocrow::mosaic_cartogram
