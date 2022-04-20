#include "painting.h"
#include "cartocrow/core/cgal_types.h"
#include "cartocrow/renderer/geometry_renderer.h"

namespace cartocrow::flow_map {

Painting::Painting(const SpiralTree& tree, const std::vector<Region>& regions,
                   const std::vector<Region>& obstacles, const Options options)
    : m_tree(tree), m_regions(regions), m_obstacles(obstacles), m_options(options) {}

void Painting::paint(renderer::GeometryRenderer& renderer) {
	paintRegions(renderer);
	paintObstacles(renderer);
	paintFlow(renderer);
	paintNodes(renderer);
}

void Painting::paintRegions(renderer::GeometryRenderer& renderer) {
	//renderer.setMode(renderer::GeometryRenderer::fill | renderer::GeometryRenderer::stroke);
	//renderer.setStroke(Color{0, 0, 0}, 2);
	//renderer.setFill(Color{230, 230, 230});
	renderer.setMode(renderer::GeometryRenderer::stroke);
	renderer.setStroke(Color{200, 200, 200}, 1.25);
	for (const Region& region : m_regions) {
		renderer.draw(region);
	}
}

void Painting::paintObstacles(renderer::GeometryRenderer& renderer) {
	renderer.setMode(renderer::GeometryRenderer::fill | renderer::GeometryRenderer::stroke);
	renderer.setStroke(Color{170, 50, 20}, 2);
	renderer.setFill(Color{230, 190, 170});
	renderer.setFillOpacity(60);

	for (const Region& region : m_obstacles) {
		renderer.draw(region);
	}

	// TODO for debugging purposes
	for (const SpiralTree::Obstacle& obstacle : m_tree.obstacles()) {
		std::vector<Point> vertices;
		vertices.reserve(obstacle.size());
		for (const PolarPoint& p : obstacle) {
			vertices.push_back(p.to_cartesian());
		}
		Polygon polygon(vertices.begin(), vertices.end());
		renderer.draw(polygon);
	}

	renderer.setFillOpacity(255);
}

void Painting::paintFlow(renderer::GeometryRenderer& renderer) {
	renderer.setMode(renderer::GeometryRenderer::stroke);
	renderer.setStroke(Color{100, 100, 100}, 4);
	for (const auto& node : m_tree.nodes()) {
		if (node->m_parent == nullptr) {
			continue;
		}
		const PolarPoint node_relative_position(node->m_place->position,
		                                        CGAL::ORIGIN - m_tree.root());
		const PolarPoint parent_relative_position(node->m_parent->m_place->position,
		                                          CGAL::ORIGIN - m_tree.root());

		const Spiral spiral(node_relative_position, parent_relative_position);
		std::cout << "painting spiral " << node->m_place->id << " at " << node_relative_position
		          << " to " << node->m_parent->m_place->id << " at " << parent_relative_position
		          << std::endl;
		paintSpiral(renderer, spiral, m_tree.root(), parent_relative_position);
	}
}

void Painting::paintNodes(renderer::GeometryRenderer& renderer) {
	renderer.setMode(renderer::GeometryRenderer::vertices);
	renderer.setStroke(Color{100, 100, 100}, 4);
	for (const auto& node : m_tree.nodes()) {
		if (node->isSteiner()) {
			renderer.draw(node->m_place->position.to_cartesian());
		}
	}
	renderer.setStroke(Color{0, 0, 0}, 4);
	for (const auto& node : m_tree.nodes()) {
		if (!node->isSteiner()) {
			renderer.draw(node->m_place->position.to_cartesian());
		}
	}
}

void Painting::paintSpiral(renderer::GeometryRenderer& renderer, const Spiral& spiral,
                           const Point& root, const PolarPoint& parent) {
	std::vector<PolarPoint> points;
	points.push_back(spiral.Evaluate(0));
	if (spiral.angle_rad() != 0) {
		for (double t = m_options.spiralStep; t < m_options.spiralMax; t += m_options.spiralStep) {
			const PolarPoint polar = spiral.Evaluate(t);
			if (polar.R() <= parent.R()) {
				break;
			}
			points.push_back(polar);
		}
	}
	points.push_back(parent);

	Vector offset = root - CGAL::ORIGIN;
	std::cout << points.size() << "spiral coordinates ";
	for (int i = 0; i < points.size(); ++i) {
		std::cout << points[i] << " ";
	}
	std::cout << std::endl;
	for (int i = 0; i + 1 < points.size(); ++i) {
		renderer.draw(
		    Segment(points[i].to_cartesian() + offset, points[i + 1].to_cartesian() + offset));
	}
}

} // namespace cartocrow::flow_map
