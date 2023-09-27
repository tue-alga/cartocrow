#include "painting.h"

namespace cartocrow::mosaic_cartogram {

Painting::Options::Options() {}

Painting::Painting(std::shared_ptr<MosaicCartogram> mosaicCartogram, Options options)
    : m_mosaicCartogram(mosaicCartogram), m_options(options) {}

void Painting::paint(renderer::GeometryRenderer &renderer) const {
	m_mosaicCartogram->m_tileMap.paint(renderer);
}

} // namespace cartocrow::mosaic_cartogram
