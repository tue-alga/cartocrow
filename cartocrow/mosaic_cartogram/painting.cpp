#include "painting.h"

#include <algorithm>
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
	renderer.pushStyle();

	paintMap(renderer, std::bind(&Painting::getColorDefault, this, std::placeholders::_1));

	/*
	for (const auto &config : map().configurations)
		for (const auto c : config.boundary)
			paintMark(renderer, c);
	*/

	renderer.popStyle();  // restore style
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

void Painting::paintMark(Renderer &renderer, Coordinate c) const {
	const Point<Inexact> p = getCentroid(c);
	const Number<Inexact> x = p.x(), y = p.y();
	const Number<Inexact> w = tileScale / 5;

	renderer.setMode(Renderer::stroke);
	renderer.setStroke(m_options.colorBorder, w);

	renderer.draw(Segment<Inexact>(Point<Inexact>(x-w, y-w), Point<Inexact>(x+w, y+w)));
	renderer.draw(Segment<Inexact>(Point<Inexact>(x-w, y+w), Point<Inexact>(x+w, y-w)));
}

void Painting::paintTile(Renderer &renderer, Coordinate c) const {
	// https://stackoverflow.com/a/45452642
	CGAL::Aff_transformation_2<Inexact> t(CGAL::TRANSLATION, getCentroid(c) - CGAL::ORIGIN);
	renderer.draw(CGAL::transform(t, tileShape));
}

void Painting::paintMap(Renderer &renderer, ColorFunction tileColor) const {
	renderer.setMode(Renderer::fill | Renderer::stroke);
	renderer.setStroke(m_options.colorBorder, tileScale / 10);
	for (const auto &config : map().configurations) {
		for (const Coordinate c : config) {
			renderer.setFill(tileColor(c));
			paintTile(renderer, c);
		}
	}
}

void Painting::paintGuidingPair(Renderer &renderer, const std::string &sourceName, const std::string &targetName) const {
	const auto &source = map().configurations[m_mosaicCartogram->getRegionIndex(sourceName)];
	const auto &target = map().configurations[m_mosaicCartogram->getRegionIndex(targetName)];

	auto [guideSource, guideTarget] = map().getGuidingShapes(source, target);
	if (guideSource) guideSource = guideSource->stretch(1 / tileScale, 1 / tileScale);
	if (guideTarget) guideTarget = guideTarget->stretch(1 / tileScale, 1 / tileScale);

	auto transfers = map().computeAllTransfers(source, target);
	std::sort(transfers.begin(), transfers.end());  // best is first

	// normalize all scores to [0.5, 1.5]
	HexagonalMap::CoordinateMap<double> scores;
	double minScore =  1e10,
	       maxScore = -1e10;
	for (const auto &t : transfers) {
		minScore = std::min(minScore, t.score);
		maxScore = std::max(maxScore, t.score);
	}
	for (const auto &t : transfers) {
		scores.insert({ t.tile, 1.5 - (t.score - minScore) / (maxScore - minScore) });
	}

	paintMap(renderer, [&](const Coordinate c) {
		if (scores.contains(c))
			return Color{255, 140, 0}.shaded(scores[c]);  // dark orange
		else if (source.contains(c))
			return m_options.colorLand.shaded(.6);
		else if (target.contains(c))
			return Color{50, 205, 50};  // lime green
		else
			return getColorUniform(c);
	});

	// draw mark to indicate best transfer
	paintMark(renderer, transfers[0].tile);

	// draw guiding shapes
	renderer.setMode(renderer::GeometryRenderer::stroke);
	renderer.setStroke({139, 69, 19}, tileScale / 5);  // saddle brown
	if (guideSource) renderer.draw(*guideSource);
	if (guideTarget) renderer.draw(*guideTarget);
}

} // namespace cartocrow::mosaic_cartogram
