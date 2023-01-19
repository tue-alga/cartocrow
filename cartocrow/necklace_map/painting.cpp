#include "painting.h"

#include "../core/core.h"
#include "../core/centroid.h"
#include "../renderer/geometry_renderer.h"
#include "bezier_necklace.h"
#include "circle_necklace.h"
#include <stdexcept>
#include <utility>

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
	// paintRegions(renderer);
	paintNecklaces(renderer);
	paintConnectors(renderer);
	paintBeads(renderer, true);
	paintBeads(renderer, false);

	renderer.setMode(renderer::GeometryRenderer::fill);
	renderer.setFillOpacity(255);
	for (auto &region_it : *m_necklaceMap->m_map) {
		Region &region = region_it.second;
		renderer.setFill(region.color);

		PolygonSet<Exact> &poly_set = region.shape;
		// std::cout << set.number_of_polygons_with_holes() << '\n';

		std::vector<PolygonWithHoles<Exact>> polygons;
		poly_set.polygons_with_holes(std::back_inserter(polygons)); // TODO: can this be done easier?

		for (auto &p_with_holes : polygons) {
			Polygon<Exact> &p = p_with_holes.outer_boundary();
			Box b = p.bbox();
			renderer.draw(Circle<Exact>(
				centroid(p_with_holes), //Point<Inexact>((b.xmin() + b.xmax()) / 2, (b.ymin() + b.ymax()) / 2),
				p.area() / 6  // probably not 'the right way'
			));

			// std::vector<std::pair<Point<Exact>, float>> weighted_points;
			// for (Point<Exact> &v : p.vertices()) weighted_points.push_back({v, 1});
			// std::cout << weighted_points << '\n';
		}
	}
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
		CHECK(element->necklace->shape->IntersectRay(element->bead->angle_rad, bead_position));
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
