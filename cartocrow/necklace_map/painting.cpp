#include "painting.h"
#include "cartocrow/necklace_map/bezier_necklace.h"
#include "cartocrow/necklace_map/circle_necklace.h"

namespace cartocrow {
namespace necklace_map {

DrawNecklaceShapeVisitor::DrawNecklaceShapeVisitor(renderer::GeometryRenderer& renderer)
    : m_renderer(renderer) {}

void DrawNecklaceShapeVisitor::Visit(CircleNecklace& shape) {
	m_renderer.draw(shape.shape_);
}

void DrawNecklaceShapeVisitor::Visit(BezierNecklace& shape) {
	// TODO
	std::cout << "Cannot draw Bézier necklaces\n";
}

Painting::Painting(std::vector<MapElement::Ptr>& elements, std::vector<Necklace::Ptr>& necklaces,
                   Number scale_factor)
    : m_elements(elements), m_necklaces(necklaces), m_scale_factor(scale_factor) {}

void Painting::paint(renderer::GeometryRenderer& renderer) {

	// regions
	renderer.setMode(renderer::GeometryRenderer::fill | renderer::GeometryRenderer::stroke);
	renderer.setStroke(renderer::Color{0, 0, 0}, 2);
	renderer.setFill(renderer::Color{0, 102, 203});
	for (const MapElement::Ptr& element : m_elements) {
		const Region& region = element->region;
		//std::cout << region.style << std::endl;
		// meh! it just stores the SVG style string
		// should we parse that or what?
		// nah, let's rewrite the input system to just use Ipe files
		//renderer.setFill(renderer::Color{0, 0, 0});
		renderer.draw(region);
	}

	// necklaces
	renderer.setMode(renderer::GeometryRenderer::stroke);
	DrawNecklaceShapeVisitor visitor(renderer);
	for (const Necklace::Ptr& necklace : m_necklaces) {
		necklace->shape->Accept(visitor);
	}

	// beads
	renderer.setMode(renderer::GeometryRenderer::fill | renderer::GeometryRenderer::stroke);
	renderer.setFill(renderer::Color{150, 0, 50});
	for (const Necklace::Ptr& necklace : m_necklaces) {
		for (const Bead::Ptr& bead : necklace->beads) {
			if (!bead->valid) {
				continue;
			}
			Point position;
			CHECK(necklace->shape->IntersectRay(bead->angle_rad, position));
			Number radius = m_scale_factor * bead->radius_base;
			renderer.draw(Circle(position, radius * radius));
		}
	}
}

} // namespace necklace_map
} // namespace cartocrow
