#include "smooth_tree_painting.h"

#include "../core/core.h"
#include "../renderer/geometry_renderer.h"
#include <CGAL/Origin.h>

namespace cartocrow::flow_map {

SmoothTreePainting::SmoothTreePainting(std::shared_ptr<SmoothTree> tree, const Options options)
    : m_tree(tree), m_options(options) {}

void SmoothTreePainting::paint(renderer::GeometryRenderer& renderer) const {
	paintFlow(renderer);
	paintNodes(renderer);
}

void SmoothTreePainting::paintFlow(renderer::GeometryRenderer& renderer) const {
	renderer.setMode(renderer::GeometryRenderer::stroke);
	renderer.setStroke(Color{255, 84, 32}, 4);
	for (const auto& node : m_tree->nodes()) {
		if (node->m_parent == nullptr) {
			continue;
		}
		renderer.draw(Segment<Inexact>(node->m_parent->m_position.toCartesian(),
		                               node->m_position.toCartesian()));
	}
}

void SmoothTreePainting::paintNodes(renderer::GeometryRenderer& renderer) const {
	renderer.setMode(renderer::GeometryRenderer::vertices);
	renderer.setStroke(Color{100, 100, 100}, 4);
	for (const auto& node : m_tree->nodes()) {
		renderer.draw(node->m_position.toCartesian());
	}
}

} // namespace cartocrow::flow_map
