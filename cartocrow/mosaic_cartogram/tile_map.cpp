#include "tile_map.h"

#include <cmath>
#include <numbers>
#include <stdexcept>
#include <CGAL/Aff_transformation_2.h>
#include <CGAL/aff_transformation_tags.h>

namespace cartocrow::mosaic_cartogram {

HexagonalMap::HexagonalMap(const VisibilityDrawing &initial,
                           const std::vector<LandRegion> &landRegions, int seaRegionCount,
                           const Parameters &params)
    : guidingPairs(landRegions.size() - 1), configurations(landRegions.size() + seaRegionCount) {
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

	// precompute all guiding pairs
	for (const LandRegion &r1 : landRegions)
		for (const LandRegion &r2 : r1.neighbors)
			if (r1.id < r2.id)
				guidingPairs[r1.id].insert({ r2.id, GuidingPair(r1, r2, params) });

	// initialize tiles
	for (const LandRegion &r : landRegions) configurations[r.id].region = r;
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

Vector<Inexact> HexagonalMap::getCentroid(const Configuration &config) const {
	Vector<Inexact> v;
	for (auto c : config.tiles) v += getCentroid(c);
	return v /= config.tiles.size();
}

Vector<Inexact> HexagonalMap::getCentroid(const Coordinate &c) const {
	return {
		inradius * (2*c.x() - c.y()),
		exradius * 3/2 * c.y()
	};
}

std::pair<Ellipse, Ellipse> HexagonalMap::getGuidingPair(const Configuration &c1, const Configuration &c2) const {
	if (c1.isSea() || c2.isSea())
		throw std::invalid_argument("Sea regions do not have a guiding shape");
	if (c1.id() == c2.id())
		throw std::invalid_argument("A guiding pair only exists for distinct regions");

	// compute centroid of union of configurations
	const int size1 = c1.tiles.size();
	const int size2 = c2.tiles.size();
	const auto centroid = (size1 * getCentroid(c1) + size2 * getCentroid(c2)) / (size1 + size2);

	// translate precomputed guiding pair to centroid
	int id1 = c1.id(), id2 = c2.id();
	if (id1 > id2) std::swap(id1, id2);
	const auto p = guidingPairs[id1].at(id2).translate(centroid.x(), centroid.y());

	return c1.id() < c2.id() ? p : std::make_pair(p.second, p.first);
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
	CGAL::Aff_transformation_2<Inexact> t(CGAL::TRANSLATION, getCentroid(c));
	renderer.draw(CGAL::transform(t, tileShape));
}

} // namespace cartocrow::mosaic_cartogram
