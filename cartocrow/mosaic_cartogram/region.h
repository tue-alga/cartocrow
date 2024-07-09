#ifndef CARTOCROW_MOSAIC_CARTOGRAM_REGION_H
#define CARTOCROW_MOSAIC_CARTOGRAM_REGION_H

#include <optional>
#include <string>

#include "../core/core.h"
#include "../core/ellipse.h"
#include "../core/region_map.h"

namespace cartocrow::mosaic_cartogram {

enum class RegionType : bool {
	/// Regions with a data value (and hence a target tile count).
	Land,
	/// Regions without a data value (which, hence, are flexible in size).
	Sea
};

/// A contiguous region in a mosaic cartogram.
struct MosaicRegion {
	/// The non-negative integer identifying this region. Identifiers are unique among regions of
	/// the same type, and there are no gaps.
	int id;
	/// The shape of this region as specified by the input map.
	PolygonWithHoles<Exact> shape;
	/// An approximation of the desired final shape of this region. It is centered at the origin,
	/// scaled according to the desired number of tiles, and its contour lines are normalized such
	/// that the additional area is equal to one tile.
	EllipseAtOrigin guidingShape;

	/// The color of this region as specified by the input map, or white for sea regions. This is
	/// only used for visualization. Note that all subregions have the same color.
	virtual Color color() const = 0;
	virtual std::string name() const = 0;
	virtual RegionType type() const = 0;

	/// Returns the corresponding \c Region.
	Region basic() const {
		return { name(), color(), PolygonSet<Exact>(shape) };
	}
};

/// A contiguous land region in a mosaic cartogram.
/// Note that the input map may contain non-contiguous regions. These "superregions" are partitioned
/// into "subregions", which are henceforth processed separately. The data value of the superregion
/// is split among the subregions by area.
struct LandRegion : public MosaicRegion {
	std::string _name;
	/// The superregion's name, if this region has one. In that case, the region's name has the
	/// following format: "<superregion name>_<index>".
	std::optional<std::string> superName;
	Color _color;
	/// The data value of this region.
	Number<Inexact> dataValue;
	/// The desired number of tiles to represent this region. This is computed from the data value
	/// and unit value.
	int targetTileCount;

	Color color() const override { return _color; }
	std::string name() const override { return _name; }
	RegionType type() const override { return RegionType::Land; }
};

/// A contiguous sea region in a mosaic cartogram.
struct SeaRegion : public MosaicRegion {
	// TODO: add "original tile count" field? based on area of `shape` and avg. tile density of land regions
	//       could be used to finetune tile maps

	Color color() const override { return { 255, 255, 255 }; }
	std::string name() const override { return "_sea" + std::to_string(id); }
	RegionType type() const override { return RegionType::Sea; }
};

} // namespace cartocrow::mosaic_cartogram

#endif //CARTOCROW_MOSAIC_CARTOGRAM_REGION_H
