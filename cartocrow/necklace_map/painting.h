#ifndef CARTOCROW_NECKLACE_MAP_PAINTING
#define CARTOCROW_NECKLACE_MAP_PAINTING

#include "../renderer/geometry_painting.h"
#include "../renderer/geometry_renderer.h"

#include "necklace_map.h"

namespace cartocrow::necklace_map {

class DrawNecklaceShapeVisitor : public NecklaceShapeVisitor {
  public:
	DrawNecklaceShapeVisitor(renderer::GeometryRenderer& renderer);
	void Visit(CircleNecklace& shape) override;
	void Visit(BezierNecklace& shape) override;

  private:
	renderer::GeometryRenderer& m_renderer;
};

/// The \ref renderer::GeometryPainting "GeometryPainting" for a necklace map.
class Painting : public renderer::GeometryPainting {

  public:
	/// Options that determine what to draw in the painting.
	struct Options {
		/// Opacity with which to draw the beads.
		double m_beadOpacity = 1.0;
		/// Whether to draw the necklaces themselves.
		bool m_drawNecklaceCurve = true;
		/// Whether do draw the kernel for each necklace.
		bool m_drawNecklaceKernel = false;
		/// Whether to draw a line between each region and its bead.
		bool m_drawConnectors = false;
	};

	/// Creates a new painting with the given map elements, necklaces, and scale
	/// factor.
	Painting(const std::vector<MapElement::Ptr>& elements,
	         const std::vector<Necklace::Ptr>& necklaces, Number scaleFactor, Options options);

  protected:
	void paint(renderer::GeometryRenderer& renderer) override;

  private:
	/// Paints the regions.
	void paintRegions(renderer::GeometryRenderer& renderer);
	/// Paints the necklaces.
	void paintNecklaces(renderer::GeometryRenderer& renderer);
	/// Paints the connectors.
	void paintConnectors(renderer::GeometryRenderer& renderer);
	/// Paints the beads.
	/**
	 * If \c shadow is \c true, this paints the bead's shadow instead of the
	 * bead itself.
	 */
	void paintBeads(renderer::GeometryRenderer& renderer, bool shadow);

	const std::vector<MapElement::Ptr>& m_elements;
	const std::vector<Necklace::Ptr>& m_necklaces;
	Number m_scaleFactor;
	Options m_options;
};

} // namespace cartocrow::necklace_map

#endif //CARTOCROW_NECKLACE_MAP_PAINTING
