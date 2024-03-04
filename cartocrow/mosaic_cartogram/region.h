#ifndef CARTOCROW_MOSAIC_CARTOGRAM_REGION_H
#define CARTOCROW_MOSAIC_CARTOGRAM_REGION_H

#include "../core/ellipse.h"
#include "../core/region_map.h"

namespace cartocrow::mosaic_cartogram {

/// A contiguous land region in the mosaic cartogram.
/// Note that the input map may contain non-contiguous regions. These "superregions" are partitioned
/// into "subregions", which are henceforth processed separately. The data value of the superregion
/// is split among the subregions by area.
struct LandRegion {
	/// The unique, non-negative number identifying this region.
	int id;
	/// The region's name.
	std::string name;
	/// The superregion's name, if this region has one. In that case, the region's name has the
	/// following format: "<superregion name>_<index>".
	std::optional<std::string> superName;
	/// The data value of this region.
	Number<Inexact> dataValue;
	/// The desired number of tiles to represent this region. This is computed from the data value
	/// and unit value.
	int targetTileCount;
	/// The color of this region as specified by the input map. This is only used for visualization.
	/// Note that all subregions have the same color.
	Color color;
	/// The shape of this region as specified by the input map.
	PolygonWithHoles<Exact> shape;
	/// An approximation of the desired final shape of this region. It is centered at the origin,
	/// scaled according to the desired number of tiles, and its contour lines are normalized such
	/// that the additional area is equal to one tile.
	EllipseAtOrigin guidingShape;
	/// The adjacent regions in clockwise order.
	std::vector<std::reference_wrapper<LandRegion>> neighbors;

	/// Returns the corresponding \c Region.
	Region basic() const {
		return { name, color, PolygonSet<Exact>(shape) };
	}
};

} // namespace cartocrow::mosaic_cartogram

#endif //CARTOCROW_MOSAIC_CARTOGRAM_REGION_H
