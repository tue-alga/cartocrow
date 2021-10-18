#ifndef CARTOCROW_NECKLACE_MAP_PAINTING
#define CARTOCROW_NECKLACE_MAP_PAINTING

#define _USE_MATH_DEFINES

#include <cartocrow/renderer/geometry_painting.h>
#include <cartocrow/renderer/geometry_renderer.h>

#include "necklace_map.h"

namespace cartocrow {
namespace necklace_map {

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
	/// Creates a new painting with the given map elements, necklaces, and scale
	/// factor.
	Painting(std::vector<MapElement::Ptr>& elements, std::vector<Necklace::Ptr>& necklaces,
	         Number scale_factor);

  protected:
	void paint(renderer::GeometryRenderer& renderer) override;

  private:
	std::vector<MapElement::Ptr>& m_elements;
	std::vector<Necklace::Ptr>& m_necklaces;
	Number m_scale_factor;
};

} // namespace necklace_map
} // namespace cartocrow

#endif //CARTOCROW_NECKLACE_MAP_PAINTING
