#ifndef CARTOCROW_NECKLACE_MAP_PAINTING
#define CARTOCROW_NECKLACE_MAP_PAINTING

#include "../renderer/geometry_painting.h"
#include "../renderer/geometry_renderer.h"

#include "necklace_map.h"

namespace cartocrow::necklace_map {

namespace detail {
class DrawNecklaceShapeVisitor : public NecklaceShapeVisitor {
  public:
	DrawNecklaceShapeVisitor(renderer::GeometryRenderer& renderer);
	void Visit(CircleNecklace& shape) override;
	void Visit(BezierNecklace& shape) override;

  private:
	renderer::GeometryRenderer& m_renderer;
};
} // namespace detail

/// The \ref renderer::GeometryPainting "GeometryPainting" for a \ref
/// NecklaceMap.
class Painting : public renderer::GeometryPainting {

  public:
	/// Options that determine what to draw in the painting.
	struct Options {
		/// Default constructor.
		Options();
		/// Opacity with which to draw the beads.
		double m_beadOpacity = 1.0;
		/// Whether to draw the necklaces themselves.
		bool m_drawNecklaceCurve = true;
		/// Whether do draw the kernel for each necklace.
		bool m_drawNecklaceKernel = false;
		/// Whether to draw a line between each region and its bead.
		bool m_drawConnectors = false;
	};

	/// Creates a new painting for the given necklace map.
	Painting(const NecklaceMap& necklaceMap, Options options = {});

  protected:
	void paint(renderer::GeometryRenderer& renderer) override;

  private:
	/// Paints the regions.
	void paintRegions(renderer::GeometryRenderer& renderer);
	/// Paints the necklaces.
	void paintNecklaces(renderer::GeometryRenderer& renderer);
	/// Paints the connectors.
	void paintConnectors(renderer::GeometryRenderer& renderer);
	/// Paints the beads. If \c shadow is \c true, this paints the bead's
	/// shadow instead of the bead itself.
	void paintBeads(renderer::GeometryRenderer& renderer, bool shadow);

	/// The necklace map we are drawing.
	const NecklaceMap& m_necklaceMap;
	/// The drawing options.
	Options m_options;
};

} // namespace cartocrow::necklace_map

#endif //CARTOCROW_NECKLACE_MAP_PAINTING
