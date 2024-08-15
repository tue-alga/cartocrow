#ifndef CARTOCROW_MOSAIC_CARTOGRAM_PAINTING
#define CARTOCROW_MOSAIC_CARTOGRAM_PAINTING

#include <functional>
#include <memory>

#include "../core/core.h"
#include "../renderer/geometry_painting.h"
#include "../renderer/geometry_renderer.h"
#include "mosaic_cartogram.h"
#include "tile_map.h"

namespace cartocrow::mosaic_cartogram {

/// The \ref renderer::GeometryPainting "GeometryPainting" for a \ref MosaicCartogram.
class Painting : public renderer::GeometryPainting {

	// TODO: generalize to also support square tiles

	using Configuration = HexagonalMap::Configuration;
	using Coordinate = HexagonalMap::Coordinate;
	using Renderer = renderer::GeometryRenderer;

  public:
	/// Options that determine what to draw in the painting and how.
	struct Options {

		Color colorBorder = {  32,  32,  32 };
		Color colorLand   = {  34, 139,  34 };  // forest green
		Color colorSea    = { 175, 238, 238 };  // pale turquoise

		/// Whether to draw borders between regions with thicker lines. This parameter is optional
		/// and \c false by default.
		bool drawBorders;

		/// The area of one tile. This essentially controls the scale of the painting (compared to
		/// the internal representation, which uses unit areas).
		Number<Inexact> tileArea;

		/// Checks whether the parameters are valid. If not, an exception is thrown.
		void validate() const;

	};

  private:
	/// A function that maps a tile coordinate to a color. Used as a parameter for painting, so
	/// different "views" can be drawn with the same function.
	using ColorFunction = std::function<Color(Coordinate)>;

	std::shared_ptr<MosaicCartogram> m_mosaicCartogram;
	Options m_options;

	/// The factor with which to scale (the inradius and exradius of) the unit hexagon to attain the
	/// user-supplied area.
	Number<Inexact> tileScale;
	/// The hexagon used for painting. Its centroid is at the origin.
	Polygon<Inexact> tileShape;

  public:
	Painting(std::shared_ptr<MosaicCartogram> mosaicCartogram, Options &&options);

	void paint(Renderer &renderer) const override;

  private:
	const HexagonalMap& map() const { return m_mosaicCartogram->m_tileMap; }

	Point<Inexact> getCentroid(Coordinate c) const;
	Polygon<Inexact> getTile(Coordinate c) const;

	// implementations of `ColorFunction`
	Color getColorDefault(Coordinate c) const;
	Color getColorUniform(Coordinate c) const;

	void paintMark(Renderer &renderer, Coordinate c) const;
	void paintBorders(Renderer &renderer, const Configuration &config) const;

	void paintMap(Renderer &renderer, ColorFunction tileColor) const;

	/// Debug function to visualize the cost computation between two regions.
	void paintGuidingPair(Renderer &renderer, const std::string &sourceName, const std::string &targetName) const;

	void paintTransfers(Renderer &renderer) const;

};

} // namespace cartocrow::mosaic_cartogram

#endif //CARTOCROW_MOSAIC_CARTOGRAM_PAINTING
