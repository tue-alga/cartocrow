#include "partition_painting.h"

namespace cartocrow::simplesets {

void draw_poly_pattern(const PolyPattern& pattern, renderer::GeometryRenderer& renderer, const GeneralSettings& gs, const DrawSettings& ds) {
	if (std::holds_alternative<Polyline<Inexact>>(pattern.poly())) {
		renderer.setMode(renderer::GeometryRenderer::stroke);
	} else {
		renderer.setMode(renderer::GeometryRenderer::fill | renderer::GeometryRenderer::stroke);
	}
	renderer.setFill(ds.getColor(pattern.category()));
	renderer.setFillOpacity(100);
	renderer.setStroke(Color{0, 0, 0}, ds.contourStrokeWeight(gs), true);
	auto& pts = pattern.catPoints();
	if (pts.size() == 1) {
//		renderer.draw(pts[0].point);
	} else {
		std::visit([&renderer](auto shape) { renderer.draw(shape); }, pattern.poly());
	}
	for (const auto& pt : pattern.catPoints()) {
		renderer.setStroke(Color{0, 0, 0}, ds.pointStrokeWeight(gs), true);
		renderer.setFillOpacity(255);
		renderer.setFill(ds.getColor(pt.category));
		renderer.setMode(renderer::GeometryRenderer::fill | renderer::GeometryRenderer::stroke);
		renderer.draw(Circle<Inexact>{pt.point, gs.pointSize * gs.pointSize});
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