#include "partition_painting.h"

namespace cartocrow::simplesets {

void draw_poly_pattern(const PolyPattern& pattern, renderer::GeometryRenderer& renderer, const GeneralSettings& gs, const DrawSettings& ds) {
	if (std::holds_alternative<Polyline<Inexact>>(pattern.poly())) {
		renderer.setMode(renderer::GeometryRenderer::stroke);
	} else {
		renderer.setMode(renderer::GeometryRenderer::fill | renderer::GeometryRenderer::stroke);
	}
	renderer.setFill(ds.colors.at(pattern.category()));
	renderer.setFillOpacity(100);
	// todo: set absolute stroke width here.
//	renderer.setStroke(Color{0, 0, 0}, ds.contourStrokeWeight(gs));
	renderer.setStroke(Color{0, 0, 0}, 1.0);
	std::visit([&renderer](auto shape){ renderer.draw(shape); }, pattern.poly());
	for (const auto& pt : pattern.catPoints()) {
		// todo: set absolute stroke width here.
//		renderer.setStroke(Color{0, 0, 0}, ds.pointStrokeWeight(gs));
		renderer.setStroke(Color{0, 0, 0}, 1.0);
		renderer.setFillOpacity(255);
		renderer.setFill(ds.colors.at(pt.category));
		renderer.setMode(renderer::GeometryRenderer::fill | renderer::GeometryRenderer::stroke);
		renderer.draw(Circle<Inexact>{pt.point, gs.pointSize});
	}
}

PartitionPainting::PartitionPainting(const Partition& partition, const GeneralSettings& gs, const DrawSettings& ds)
    : m_partition(partition), m_gs(gs), m_ds(ds) {}

void PartitionPainting::paint(renderer::GeometryRenderer& renderer) const {
	for (const auto& pattern : m_partition) {
		draw_poly_pattern(*pattern, renderer, m_gs, m_ds);
	}
}
}