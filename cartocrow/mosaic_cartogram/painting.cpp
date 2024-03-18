#include "painting.h"

#include <cmath>
#include <numbers>
#include <stdexcept>
#include <CGAL/Aff_transformation_2.h>
#include <CGAL/aff_transformation_tags.h>
#include <CGAL/Origin.h>

namespace cartocrow::mosaic_cartogram {

void Painting::Options::validate() const {
	if (tileArea <= 0) {
		throw std::invalid_argument("`tileArea` must be positive");
	}
}

Painting::Painting(std::shared_ptr<MosaicCartogram> mosaicCartogram, Options &&options)
    : m_mosaicCartogram(mosaicCartogram), m_options(options) {
	// compute constants
	tileScale = std::sqrt(options.tileArea);
	for (int i = 0; i < 6; i++) {
		const auto a = (1 + 2*i) * std::numbers::pi / 6;
		tileShape.push_back({
			tileScale * HexagonalMap::tileExradius * std::cos(a),
			tileScale * HexagonalMap::tileExradius * std::sin(a)
		});
	}
}

void Painting::paint(Renderer &renderer) const {
	paint(renderer, std::bind(&Painting::getColorDefault, this, std::placeholders::_1));
}

Point<Inexact> Painting::getCentroid(Coordinate c) const {
	return CGAL::ORIGIN + tileScale * (HexagonalMap::getCentroid(c) - CGAL::ORIGIN);
}

Color Painting::getColorDefault(Coordinate c) const {
	const auto config = map().getConfiguration(c);
	return !config || config->isSea() ? Color{255, 255, 255} : config->region->get().color;
}

Color Painting::getColorUniform(Coordinate c) const {
	const auto config = map().getConfiguration(c);
	return !config || config->isSea() ? m_options.colorSea : m_options.colorLand;
}

void Painting::paintTile(Renderer &renderer, Coordinate c) const {
	// https://stackoverflow.com/a/45452642
	CGAL::Aff_transformation_2<Inexact> t(CGAL::TRANSLATION, getCentroid(c) - CGAL::ORIGIN);
	renderer.draw(CGAL::transform(t, tileShape));
}

void Painting::paint(Renderer &renderer, ColorFunction tileColor) const {
	renderer.pushStyle();
	renderer.setMode(Renderer::fill | Renderer::stroke);
	renderer.setStroke(m_options.colorBorder, tileScale / 10);

	for (const auto &config : map().configurations) {
		for (const Coordinate c : config) {
			renderer.setFill(tileColor(c));
			paintTile(renderer, c);
		}
	}

	renderer.popStyle();  // restore style
}

} // namespace cartocrow::mosaic_cartogram
