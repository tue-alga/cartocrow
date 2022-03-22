#include "painting.h"
#include "cartocrow/core/cgal_types.h"
#include "cartocrow/renderer/geometry_renderer.h"

namespace cartocrow::flow_map {

Painting::Painting(const FlowTree::Ptr& tree, const std::vector<Region>& regions,
                   const std::vector<Region>& obstacles, const Options options)
    : m_tree(tree), m_regions(regions), m_obstacles(obstacles), m_options(options) {}

void Painting::paint(renderer::GeometryRenderer& renderer) {
	paintRegions(renderer);
	paintObstacles(renderer);
	paintFlow(renderer);
	paintNodes(renderer);
}

void Painting::paintRegions(renderer::GeometryRenderer& renderer) {
	renderer.setMode(renderer::GeometryRenderer::fill | renderer::GeometryRenderer::stroke);
	renderer.setStroke(Color{0, 0, 0}, 2);
	renderer.setFill(Color{230, 230, 230});
	for (const Region& region : m_regions) {
		renderer.draw(region);
	}
}

void Painting::paintObstacles(renderer::GeometryRenderer& renderer) {
	renderer.setMode(renderer::GeometryRenderer::fill | renderer::GeometryRenderer::stroke);
	renderer.setStroke(Color{170, 50, 20}, 2);
	renderer.setFill(Color{230, 190, 170});

	// TODO we draw the tree obstacles for now for debugging purposes
	// (replace by m_obstacles when done)
	for (const Region& region : m_tree->obstacles_) {
		renderer.draw(region);
	}
}

void Painting::paintFlow(renderer::GeometryRenderer& renderer) {
	renderer.setMode(renderer::GeometryRenderer::stroke);
	renderer.setStroke(Color{0, 0, 0}, 4);
	for (const FlowTree::FlowArc& arc : m_tree->arcs_) {
		const Spiral& spiral = arc.first;
		const PolarPoint& parent = arc.second;
		paintSpiral(renderer, spiral, -m_tree->root_translation_, parent);
	}
}

void Painting::paintNodes(renderer::GeometryRenderer& renderer) {
	renderer.setMode(renderer::GeometryRenderer::vertices);
	for (const Node::Ptr& node : m_tree->nodes_) {
		renderer.draw(node->place->position.to_cartesian());
	}
}

void Painting::paintSpiral(renderer::GeometryRenderer& renderer, const Spiral& spiral,
                           const Vector& offset, const PolarPoint& parent) {
	std::vector<Point> points;
	const Point anchor = spiral.Evaluate(0).to_cartesian() + offset;
	points.push_back(anchor);
	if (spiral.angle_rad() != 0) {
		for (double t = m_options.spiralStep; t < m_options.spiralMax; t += m_options.spiralStep) {
			const PolarPoint polar = spiral.Evaluate(t);
			if (polar.R() <= parent.R()) {
				break;
			}

			points.emplace_back(polar.to_cartesian() + offset);
		}
	}
	const Point parent_c = parent.to_cartesian() + offset;
	points.push_back(parent_c);

	for (int i = 0; i + 1 < points.size(); ++i) {
		renderer.draw(Segment(points[i], points[i + 1]));
	}
}

} // namespace cartocrow::flow_map
