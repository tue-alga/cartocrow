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
	renderer.setStroke(Color{0, 0, 0}, 2);
	for (const MapElement::Ptr& element : m_elements) {
		const Region& region = element->region;
		if (!element->necklace) {
			renderer.setFill(Color{255, 255, 255});
		} else if (element->value <= 0) {
			renderer.setFill(Color{230, 230, 230});
		} else {
			renderer.setFill(element->color);
		}
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
	for (const MapElement::Ptr& element : m_elements) {
		if (!element->bead || !element->bead->valid) {
			continue;
		}
		Point position;
		CHECK(element->necklace->shape->IntersectRay(element->bead->angle_rad, position));
		Number radius = m_scale_factor * element->bead->radius_base;
		renderer.setFill(element->color);
		renderer.draw(Circle(position, radius * radius));
		renderer.drawText(position, element->region.id);
	}
}

} // namespace necklace_map
} // namespace cartocrow
