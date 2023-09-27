#include "tile_map.h"

#include <cmath>
#include <numbers>
#include <CGAL/Aff_transformation_2.h>
#include <CGAL/aff_transformation_tags.h>

namespace cartocrow::mosaic_cartogram {

HexagonalMap::HexagonalMap(const VisibilityDrawing &initial,
                           const std::vector<LandRegion> &landRegions, int seaRegionCount,
                           const Parameters &params) {
	// compute painting constants
	const double sqrt3 = std::sqrt(3);
	tileArea = std::numbers::pi * params.tileRadius * params.tileRadius;
	inradius = std::sqrt(tileArea / (2*sqrt3));
	exradius = inradius * 2/sqrt3;
	for (int i = 0; i < 6; i++) {
		const auto a = (1 + 2*i) * std::numbers::pi / 6;
		tileShape.push_back({
			inradius + exradius * std::cos(a),
			exradius + exradius * std::sin(a)
		});
	}

	// initialize tiles
	for (const LandRegion &r : landRegions) configurations.push_back({ r });
	for (int i = 0; i < seaRegionCount; i++) configurations.push_back({});
	const int w = initial.grid.size()    - 2;  // the left and right columns contain only outer sea regions, which are ignored
	const int h = initial.grid[0].size() - 2;  // the bottom and top rows ...
	for (int x = 0; x < w; x++)
		for (int y = 0; y < h; y++) {
			int i = initial.grid[x+1][h-y];  // vertical flip
			if (i < configurations.size())  // don't add tiles for outer sea regions
				configurations[i].tiles.insert({ x+y, y });  // convert grid coordinate to barycentric coordinate
		}
	for (int i = 0; i < configurations.size(); i++)
		for (const Coordinate &c : configurations[i].tiles)
			tiles[c] = i;
}

void HexagonalMap::paint(renderer::GeometryRenderer &renderer) const {
	renderer.pushStyle();
	renderer.setMode(renderer::GeometryRenderer::fill | renderer::GeometryRenderer::stroke);
	renderer.setStroke(Color{0, 0, 0}, inradius / 5);

	for (const Configuration &config : configurations) {
		renderer.setFill(config.region ? config.region->get().color : Color{255, 255, 255});  // white for sea regions
		for (const Coordinate &c : config.tiles) paintTile(renderer, c);
	}

	renderer.popStyle();  // restore style
}

void HexagonalMap::paintTile(renderer::GeometryRenderer &renderer, const Coordinate &c) const {
	// https://stackoverflow.com/a/45452642
	CGAL::Aff_transformation_2<Inexact> t(CGAL::TRANSLATION, {
		inradius * (2*c.x() - c.y()),
		exradius * 3/2 * c.y()
	});
	Polygon<Inexact> p(CGAL::transform(t, tileShape));
	renderer.draw(p);
}

} // namespace cartocrow::mosaic_cartogram
