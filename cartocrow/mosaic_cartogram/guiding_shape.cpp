#include "guiding_shape.h"

#include <numbers>
#include <CGAL/number_utils.h>

#include "../core/centroid.h"

namespace cartocrow::mosaic_cartogram {

GuidingPair::GuidingPair(const LandRegion &region1, const LandRegion &region2) {
	// internally, tiles have unit area
	// here, we approximate tiles by circles, and those have radius sqrt(1/pi)
	static constexpr double tileRadius = std::numbers::inv_sqrtpi_v<double>;

	const Point<Exact> centroid1 = centroid(region1.shape);
	const Point<Exact> centroid2 = centroid(region2.shape);

	const EllipseAtOrigin &guide1 = region1.guidingShape;
	const EllipseAtOrigin &guide2 = region2.guidingShape;

	const double area1 = guide1.area();
	const double area2 = guide2.area();


	// slope of line through the centroids  (TODO: what if x1 = x2?)
	const double m = CGAL::to_double((centroid1.y() - centroid2.y()) / (centroid1.x() - centroid2.x()));

	// Δx per unit of distance between the centroids
	const double dx = 1 / std::sqrt(1 + m*m);

	// -1 if `region1` lies left of `region2`, else +1  (w.r.t. their centroids)
	const int direction = centroid1.x() < centroid2.x() ? -1 : +1;

	const double dx1 = dx * (guide1.radius(m) - tileRadius) *  direction;
	const double dx2 = dx * (guide2.radius(m) - tileRadius) * -direction;

	// new (relative) centers for guiding shapes, equivalent to:
	// moving them along  y = m x  such that they overlap the diameter of one tile w.r.t. this line
	// TODO: yields undesirable results if guiding shapes are small compared to a tile
	const Vector<Inexact> center1 = { dx1, m * dx1 };
	const Vector<Inexact> center2 = { dx2, m * dx2 };

	// together, the guiding shapes should be centered around 0
	// so we add the opposite of their "joint" center  (note: overlap counted twice)
	const Vector<Inexact> correction = -(area1 * center1 + area2 * center2) / (area1 + area2);

	// apply translation
	ellipse1 = guide1.translate(center1 + correction);
	ellipse2 = guide2.translate(center2 + correction);
}

std::pair<Ellipse, Ellipse> GuidingPair::translate(const Vector<Inexact> &v) const {
	return {
		ellipse1.translate(v).normalizeSign(),
		ellipse2.translate(v).normalizeSign()
	};
}

} // namespace cartocrow::mosaic_cartogram
