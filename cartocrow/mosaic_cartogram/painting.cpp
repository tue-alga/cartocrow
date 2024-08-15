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
    : m_mosaicCartogram(mosaicCartogram), m_options(options),
      tileScale(std::sqrt(m_options.tileArea)) {
	for (int i = -1; i < 5; i++) {  // such that right edge is first
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

Polygon<Inexact> Painting::getTile(const Coordinate c) const {
	// https://stackoverflow.com/a/45452642
	CGAL::Aff_transformation_2<Inexact> t(CGAL::TRANSLATION, getCentroid(c) - CGAL::ORIGIN);
	return CGAL::transform(t, tileShape);
}

Color Painting::getColorDefault(Coordinate c) const {
	const auto config = map().getConfiguration(c);
	return config ? config->region->color() : Color{255, 255, 255};
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

void Painting::paintBorders(Renderer &renderer, const Configuration &config) const {
	for (const Coordinate c0 : config) {
		const auto tile = getTile(c0);
		auto edge = tile.edges_begin();
		for (const Coordinate c1 : c0.neighbors()) {  // edges are in same order as neighbors!
			if (!config.contains(c1)) renderer.draw(*edge);
			++edge;
		}
	}
}

void Painting::paintMap(Renderer &renderer, ColorFunction tileColor) const {
	// draw tiles
	renderer.setMode(Renderer::fill | Renderer::stroke);
	renderer.setStroke(m_options.colorBorder, tileScale / 10);
	for (const auto &config : map().configurations) {
		for (const Coordinate c : config) {
			renderer.setFill(tileColor(c));
			renderer.draw(getTile(c));
		}
	}

	// optionally, draw borders between regions
	if (m_options.drawBorders) {
		renderer.setStroke(m_options.colorBorder, tileScale / 4);
		for (const auto &config : map().configurations) paintBorders(renderer, config);
	}
}

void Painting::paintGuidingPair(Renderer &renderer, const std::string &sourceName, const std::string &targetName) const {
	const auto &source = map().configurations[m_mosaicCartogram->m_regionIndices.at(sourceName)];
	const auto &target = map().configurations[m_mosaicCartogram->m_regionIndices.at(targetName)];

	auto [guideSource, guideTarget] = map().getGuidingShapes(source, target);
	guideSource = guideSource.stretch(1 / tileScale);
	guideTarget = guideTarget.stretch(1 / tileScale);

	auto transfers = map().computeAllTransfers(source, target);
	std::sort(transfers.begin(), transfers.end());  // best is first

	// normalize all costs to [0.5, 1.5]
	HexagonalMap::CoordinateMap<double> costs;
	double minCost =  1e10,
	       maxCost = -1e10;
	for (const auto &t : transfers) {
		minCost = std::min(minCost, t.cost);
		maxCost = std::max(maxCost, t.cost);
	}
	for (const auto &t : transfers) {
		costs.insert({ t.tile, 1.5 - (t.cost - minCost) / (maxCost - minCost) });
	}

	paintMap(renderer, [&](const Coordinate c) {
		if (costs.contains(c))
			return Color{255, 140, 0}.shaded(costs[c]);  // dark orange
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
	renderer.draw(guideSource);
	renderer.draw(guideTarget);
}

void Painting::paintTransfers(Renderer &renderer) const {
	renderer.setMode(renderer::GeometryRenderer::stroke);
	renderer.setStroke({139, 69, 19}, tileScale / 5);  // saddle brown
	for (const auto [i, j] : map().configGraph.getEdges()) {
		const auto &source = map().configurations[i];
		const auto &target = map().configurations[j];

		const auto t = map().computeBestTransfer(source, target);
		if (t) {
			renderer.setStroke(
				t->cost < 0 ? Color{0, 255, 0} : Color{255, 0, 0},
				std::sqrt(std::abs(t->cost)) / 10
			);

			auto sourceCentroid = map().getCentroid(source) - CGAL::ORIGIN;
			auto targetCentroid = map().getCentroid(target) - CGAL::ORIGIN;

			sourceCentroid *= tileScale;
			targetCentroid *= tileScale;

			auto normal = targetCentroid - sourceCentroid;
			normal = Vector<Inexact>(normal.y(), -normal.x());
			normal /= std::sqrt(normal.squared_length());
			normal *= 2;

			renderer.draw(Segment<Inexact>(
				CGAL::ORIGIN + sourceCentroid + normal,
				CGAL::ORIGIN + targetCentroid + normal
			));
		}
	}
}

} // namespace cartocrow::mosaic_cartogram
