#include "choropleth.h"
#include "../core/arrangement_helpers.h"
#include "../core/centroid.h"

namespace cartocrow::chorematic_map {
using namespace renderer;
void ChoroplethPainting::paint(GeometryRenderer& renderer) const {
	const auto& arr = *m_choropleth.m_arr;
	for (auto fit = arr.faces_begin(); fit != arr.faces_end(); ++fit) {
		if (!fit->has_outer_ccb()) continue;
		auto region = fit->data();
		if (region.empty() || region == "#") {
            renderer.setMode(0);
            continue;
		} else {
            renderer.setMode(GeometryRenderer::fill);
		}
		std::optional<int> bin = m_choropleth.regionToBin(region);
		Color color = m_noDataColor;
		if (bin.has_value()) {
			if (m_colors.size() <= *bin) {
				std::cerr << "No color specified for bin " << *bin;
			} else {
				color = m_colors.at(*bin);
			}
		}
		renderer.setFill(color);
		auto poly = approximate(face_to_polygon_with_holes<Exact>(fit));
		renderer.draw(poly);
		if (m_drawLabels) {
			auto c = centroid(poly);
			renderer.setMode(GeometryRenderer::stroke);
			renderer.setStroke(Color{0, 0, 0}, 1.0);
            auto label = fit->data().empty() ? "empty" : fit->data();
			renderer.drawText(c, label);
		}
	}
	renderer.setMode(GeometryRenderer::stroke);
	renderer.setStroke(Color{0, 0, 0}, 1.0);
	for (auto eit = arr.edges_begin(); eit != arr.edges_end(); ++eit) {
		renderer.draw(Segment<Exact>(eit->source()->point(), eit->target()->point()));
	}
}
}