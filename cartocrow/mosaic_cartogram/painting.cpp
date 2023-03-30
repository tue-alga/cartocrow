#include "painting.h"

#include "../core/core.h"
#include "triangulation.h"

namespace cartocrow::mosaic_cartogram {

Painting::Options::Options() {}

Painting::Painting(std::shared_ptr<MosaicCartogram> mosaicCartogram, Options options)
    : m_mosaicCartogram(mosaicCartogram), m_options(options) {}

void Painting::paint(renderer::GeometryRenderer& renderer) const {
	renderer.setMode(renderer::GeometryRenderer::fill);
	for (const auto &[id, region] : *m_mosaicCartogram->m_origMap) {
		renderer.setFill(region.color);
		renderer.draw(region.shape);
	}

	RegionArrangement arr = triangulate(m_mosaicCartogram->m_arr, m_mosaicCartogram->m_salient);
	renderer.setMode(renderer::GeometryRenderer::stroke);
	renderer.setStroke(Color{0, 0, 0}, .5);
	for (const auto &e : arr.edge_handles()) {
		renderer.draw(Segment<Exact>(e->source()->point(), e->target()->point()));
	}
	for (const auto &p : m_mosaicCartogram->m_salient) {
		renderer.draw(p);
	}
}

} // namespace cartocrow::mosaic_cartogram
