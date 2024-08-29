#include "smooth_tree_painting.h"

#include <CGAL/Origin.h>

#include "../core/core.h"
#include "../renderer/geometry_renderer.h"

namespace cartocrow::flow_map {

SmoothTreePainting::SmoothTreePainting(std::shared_ptr<SmoothTree> tree, const Options options)
    : m_tree(tree), m_options(options) {}

void SmoothTreePainting::paint(renderer::GeometryRenderer& renderer) const {
	paintFlow(renderer);
	paintNodes(renderer);
}

void SmoothTreePainting::paintFlow(renderer::GeometryRenderer& renderer) const {
	renderer.setMode(renderer::GeometryRenderer::stroke);

	// boundary
	for (const auto& node : m_tree->nodes()) {
		if (node->m_parent == nullptr) {
			continue;
		}
		renderer.setStroke(Color{0, 0, 0}, node->m_flow + 0.2f, true);
		renderer.draw(Segment<Inexact>(node->m_parent->m_position.toCartesian(),
		                               node->m_position.toCartesian()));
	}

	// interior
	for (const auto& node : m_tree->nodes()) {
		if (node->m_parent == nullptr) {
			continue;
		}
		renderer.setStroke(Color{255, 84, 32}, node->m_flow, true);
		renderer.draw(Segment<Inexact>(node->m_parent->m_position.toCartesian(),
		                               node->m_position.toCartesian()));
	}
}

void SmoothTreePainting::paintNodes(renderer::GeometryRenderer& renderer) const {
	renderer.setMode(renderer::GeometryRenderer::vertices | renderer::GeometryRenderer::stroke);
	renderer.setStroke(Color{100, 100, 100}, 4);
	for (int i = 0; i < m_tree->nodes().size(); i++) {
		const auto& node = m_tree->nodes()[i];
		if (node->getType() == Node::ConnectionType::kLeaf) {
			renderer.setStroke(Color{84, 160, 32}, 4);
			renderer.draw(node->m_position.toCartesian());
		} else if (node->getType() == Node::ConnectionType::kRoot) {
			renderer.setStroke(Color{0, 0, 0}, 4);
			renderer.draw(node->m_position.toCartesian());
		}
	}
}

} // namespace cartocrow::flow_map
