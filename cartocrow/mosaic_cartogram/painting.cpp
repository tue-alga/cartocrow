#include "painting.h"

#include "../core/core.h"
#include "../renderer/geometry_renderer.h"

namespace cartocrow::mosaic_cartogram {

Painting::Options::Options() {}

Painting::Painting(std::shared_ptr<MosaicCartogram> mosaicCartogram, Options options)
    : m_mosaicCartogram(mosaicCartogram), m_options(options) {}

void Painting::paint(renderer::GeometryRenderer& renderer) const {
	// TODO
    renderer.draw(Circle<Exact>(Point<Exact>(10, 20), 10));
}

} // namespace cartocrow::mosaic_cartogram
