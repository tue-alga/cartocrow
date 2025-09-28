#include "painting.h"

#include "../core/core.h"
#include "../renderer/geometry_renderer.h"
#include "bezier_necklace.h"
#include "circle_necklace.h"
#include <stdexcept>

namespace cartocrow::necklace_map {

namespace detail {
DrawNecklaceShapeVisitor::DrawNecklaceShapeVisitor(renderer::GeometryRenderer& renderer)
    : m_renderer(renderer) {}

void DrawNecklaceShapeVisitor::Visit(CircleNecklace& shape) {
	m_renderer.draw(shape.shape_);
}

void DrawNecklaceShapeVisitor::Visit(BezierNecklace& shape) {
	// TODO
	//m_renderer.draw(shape.spline());
}
} // namespace detail

Painting::Options::Options() {}

Painting::Painting(std::shared_ptr<NecklaceMap> necklaceMap, Options options)
    : m_necklaceMap(necklaceMap), m_options(options) {}

void Painting::paint(renderer::GeometryRenderer& renderer) const {
	paintRegions(renderer);
	paintNecklaces(renderer);
	paintConnectors(renderer);
	paintBeads(renderer, true);
	paintBeads(renderer, false);
}

void Painting::paintRegions(renderer::GeometryRenderer& renderer) const {
	renderer.setMode(renderer::GeometryRenderer::fill | renderer::GeometryRenderer::stroke);
	renderer.setStroke(Color{0, 0, 0}, 2);
	for (const Necklace& necklace : m_necklaceMap->m_necklaces) {
		for (const std::shared_ptr<Bead>& bead : necklace.beads) {
			const Region* region = bead->region;
			/*if (!region->necklace) {
				renderer.setFill(Color{255, 255, 255});
			} else if (element->value <= 0) {
				renderer.setFill(Color{230, 230, 230});
			} else {*/
			renderer.setFill(region->color);
			/*}*/
			renderer.draw(region->shape);
		}
	}
}

void Painting::paintNecklaces(renderer::GeometryRenderer& renderer) const {
	renderer.setMode(renderer::GeometryRenderer::stroke);
	detail::DrawNecklaceShapeVisitor visitor(renderer);
	for (const Necklace& necklace : m_necklaceMap->m_necklaces) {
		if (m_options.m_drawNecklaceCurve) {
			necklace.shape->accept(visitor);
		}
		if (m_options.m_drawNecklaceKernel) {
			renderer.draw(necklace.shape->kernel());
		}
	}
}

void Painting::paintConnectors(renderer::GeometryRenderer& renderer) const {
	// TODO

	/*if (!m_options.m_drawConnectors) {
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
		assert(element->necklace->shape->IntersectRay(element->bead->angle_rad, bead_position));
		renderer.draw(centroid);
		renderer.draw(Segment(centroid, bead_position));
	}*/
}

void Painting::paintBeads(renderer::GeometryRenderer& renderer, bool shadow) const {
	if (shadow) {
		renderer.setMode(renderer::GeometryRenderer::fill);
		renderer.setFillOpacity(80);
		renderer.setFill(Color{0, 0, 0});
	} else {
		renderer.setMode(renderer::GeometryRenderer::fill | renderer::GeometryRenderer::stroke);
		renderer.setFillOpacity(static_cast<int>(m_options.m_beadOpacity * 255));
	}
	for (const Necklace& necklace : m_necklaceMap->m_necklaces) {
		for (const std::shared_ptr<Bead>& bead : necklace.beads) {
			Point<Inexact> position;
			if (!necklace.shape->intersectRay(bead->angle_rad, position)) {
				throw std::runtime_error("Ray to bead \"" + bead->region->name +
				                         "\" does not intersect necklace");
			}
			Number<Inexact> radius = m_necklaceMap->m_scaleFactor * bead->radius_base;
			if (shadow) {
				renderer.draw(Circle<Inexact>(position + Vector<Inexact>(2, -2), radius * radius));
			} else {
				renderer.setFill(bead->region->color);
				renderer.draw(Circle<Inexact>(position, radius * radius));
				renderer.drawText(position, bead->region->name);
			}
		}
		renderer.setFillOpacity(255);
	}
}

} // namespace cartocrow::necklace_map
