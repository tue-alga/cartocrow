#include "painting.h"
#include "cartocrow/core/cgal_types.h"
#include "cartocrow/necklace_map/bezier_necklace.h"
#include "cartocrow/necklace_map/circle_necklace.h"
#include "cartocrow/renderer/geometry_renderer.h"

namespace cartocrow {
namespace necklace_map {

DrawNecklaceShapeVisitor::DrawNecklaceShapeVisitor(renderer::GeometryRenderer& renderer)
    : m_renderer(renderer) {}

void DrawNecklaceShapeVisitor::Visit(CircleNecklace& shape) {
	m_renderer.draw(shape.shape_);
}

void DrawNecklaceShapeVisitor::Visit(BezierNecklace& shape) {
	m_renderer.draw(shape.spline());
}

Painting::Painting(const std::vector<MapElement::Ptr>& elements,
                   const std::vector<Necklace::Ptr>& necklaces, Number scaleFactor, Options options)
    : m_elements(elements), m_necklaces(necklaces), m_scaleFactor(scaleFactor), m_options(options) {}

void Painting::paint(renderer::GeometryRenderer& renderer) {
	paintRegions(renderer);
	paintNecklaces(renderer);
	paintConnectors(renderer);
	paintBeads(renderer, true);
	paintBeads(renderer, false);
}

void Painting::paintRegions(renderer::GeometryRenderer& renderer) {
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
}

void Painting::paintNecklaces(renderer::GeometryRenderer& renderer) {
	renderer.setMode(renderer::GeometryRenderer::stroke);
	DrawNecklaceShapeVisitor visitor(renderer);
	for (const Necklace::Ptr& necklace : m_necklaces) {
		if (m_options.m_drawNecklaceCurve) {
			necklace->shape->Accept(visitor);
		}
		if (m_options.m_drawNecklaceKernel) {
			renderer.draw(necklace->shape->kernel());
		}
	}
}

void Painting::paintConnectors(renderer::GeometryRenderer& renderer) {
	if (!m_options.m_drawConnectors) {
		return;
	}

	renderer.setMode(renderer::GeometryRenderer::stroke);

	for (const MapElement::Ptr& element : m_elements) {
		if (!element->bead || !element->bead->valid) {
			continue;
		}
		Polygon simpleRegion;
		element->region.MakeSimple(simpleRegion);
		Point centroid = ComputeCentroid()(simpleRegion);
		Point bead_position;
		CHECK(element->necklace->shape->IntersectRay(element->bead->angle_rad, bead_position));
		renderer.draw(centroid);
		renderer.draw(Segment(centroid, bead_position));
	}
}

void Painting::paintBeads(renderer::GeometryRenderer& renderer, bool shadow) {
	if (shadow) {
		renderer.setMode(renderer::GeometryRenderer::fill);
		renderer.setFillOpacity(80);
		renderer.setFill(Color{0, 0, 0});
	} else {
		renderer.setMode(renderer::GeometryRenderer::fill | renderer::GeometryRenderer::stroke);
		renderer.setFillOpacity(static_cast<int>(m_options.m_beadOpacity * 255));
	}
	for (const MapElement::Ptr& element : m_elements) {
		if (!element->bead || !element->bead->valid) {
			continue;
		}
		Point position;
		CHECK(element->necklace->shape->IntersectRay(element->bead->angle_rad, position));
		Number radius = m_scaleFactor * element->bead->radius_base;
		if (shadow) {
			renderer.draw(Circle(position + Vector(5, -5), radius * radius));
		} else {
			renderer.setFill(element->color);
			renderer.draw(Circle(position, radius * radius));
			renderer.drawText(position, element->region.id);
		}
	}
	renderer.setFillOpacity(255);
}

} // namespace necklace_map
} // namespace cartocrow
