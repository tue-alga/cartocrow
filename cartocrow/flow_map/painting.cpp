#include "painting.h"

#include "../core/core.h"
#include "../renderer/geometry_renderer.h"
#include <CGAL/Origin.h>

namespace cartocrow::flow_map {

Painting::Painting(std::shared_ptr<RegionMap> map, std::shared_ptr<SpiralTree> tree,
                   const Options options)
    : m_tree(tree), m_map(map), m_options(options) {}

void Painting::paint(renderer::GeometryRenderer& renderer) const {
	if (m_map) {
		paintRegions(renderer);
	}
	paintObstacles(renderer);
	paintFlow(renderer);
	paintNodes(renderer);
}

void Painting::paintRegions(renderer::GeometryRenderer& renderer) const {
	//renderer.setMode(renderer::GeometryRenderer::fill | renderer::GeometryRenderer::stroke);
	//renderer.setStroke(Color{0, 0, 0}, 2);
	//renderer.setFill(Color{230, 230, 230});
	renderer.setMode(renderer::GeometryRenderer::stroke);
	renderer.setStroke(Color{200, 200, 200}, 1.25);
	for (auto& [key, region] : *m_map) {
		renderer.draw(region.shape);
	}
}

void Painting::paintObstacles(renderer::GeometryRenderer& renderer) const {
	renderer.setMode(/*renderer::GeometryRenderer::fill |*/ renderer::GeometryRenderer::stroke);
	renderer.setStroke(Color{170, 50, 20}, 2);
	renderer.setFill(Color{230, 190, 170});
	renderer.setFillOpacity(60);

	for (const SpiralTree::Obstacle& obstacle : m_tree->obstacles()) {
		std::vector<Point<Inexact>> vertices;
		vertices.reserve(obstacle.size());
		for (auto e : obstacle) {
			vertices.push_back(e->shape().start().toCartesian() +
			                   (m_tree->rootPosition() - CGAL::ORIGIN));
		}
		Polygon<Inexact> polygon(vertices.begin(), vertices.end());
		renderer.draw(polygon);
		/*for (size_t i = 0; i < vertices.size(); i++) {
			//renderer.draw(vertices[i]);
			renderer.drawText(vertices[i], std::to_string(i));
		}*/
	}

	renderer.setFillOpacity(255);
}

void Painting::paintFlow(renderer::GeometryRenderer& renderer) const {
	renderer.setMode(renderer::GeometryRenderer::stroke);
	renderer.setStroke(Color{100, 100, 100}, 4);
	for (const auto& node : m_tree->nodes()) {
		if (node->m_parent == nullptr) {
			continue;
		}
		const Spiral spiral(node->m_position, node->m_parent->m_position);
		paintSpiral(renderer, spiral, m_tree->rootPosition(), node->m_parent->m_position);
	}
}

void Painting::paintNodes(renderer::GeometryRenderer& renderer) const {
	renderer.setMode(renderer::GeometryRenderer::vertices);
	renderer.setStroke(Color{100, 100, 100}, 4);
	for (const auto& node : m_tree->nodes()) {
		if (node->isSteiner()) {
			renderer.draw(node->m_position.toCartesian() + (m_tree->rootPosition() - CGAL::ORIGIN));
		}
	}
	renderer.setStroke(Color{0, 0, 0}, 4);
	for (const auto& node : m_tree->nodes()) {
		if (!node->isSteiner()) {
			renderer.draw(node->m_position.toCartesian() + (m_tree->rootPosition() - CGAL::ORIGIN));
		}
	}
	renderer.setStroke(Color{0, 50, 150}, 4);
	renderer.draw(m_tree->rootPosition());
}

void Painting::paintSpiral(renderer::GeometryRenderer& renderer, const Spiral& spiral,
                           const Point<Inexact>& root, const PolarPoint& parent) const {
	std::vector<PolarPoint> points;
	points.push_back(spiral.evaluate(0));
	if (spiral.angle() != 0) {
		for (double t = m_options.spiralStep; t < m_options.spiralMax; t += m_options.spiralStep) {
			const PolarPoint polar = spiral.evaluate(t);
			if (polar.r() <= parent.r()) {
				break;
			}
			points.push_back(polar);
		}
	}
	points.push_back(parent);

	Vector<Inexact> offset = root - CGAL::ORIGIN;
	//std::cout << points.size() << "spiral coordinates ";
	//for (int i = 0; i < points.size(); ++i) {
	//	std::cout << points[i] << " ";
	//}
	//std::cout << std::endl;
	for (int i = 0; i + 1 < points.size(); ++i) {
		renderer.draw(Segment<Inexact>(points[i].toCartesian() + offset,
		                               points[i + 1].toCartesian() + offset));
	}
}

} // namespace cartocrow::flow_map
