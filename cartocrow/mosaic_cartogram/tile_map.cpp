#include "tile_map.h"

#include <cmath>
#include <numbers>
#include <stdexcept>
#include <CGAL/Aff_transformation_2.h>
#include <CGAL/aff_transformation_tags.h>
#include <CGAL/Origin.h>

namespace cartocrow::mosaic_cartogram {

std::array<HexagonalMap::Coordinate, 6> HexagonalMap::Coordinate::neighbors() const {
	// we must use double outer braces to indicate that the inner braces initialize a Coordinate
	return {{
		{ m_x + 1, m_y + 0 },
		{ m_x + 1, m_y + 1 },
		{ m_x + 0, m_y + 1 },
		{ m_x - 1, m_y + 0 },
		{ m_x - 1, m_y - 1 },
		{ m_x + 0, m_y - 1 }
	}};
}

std::ostream& operator<<(std::ostream &os, const HexagonalMap::Coordinate &c) {
	return os << '(' << c.m_x << ", " << c.m_y << ')';
}

HexagonalMap::HexagonalMap(const VisibilityDrawing &initial,
                           const std::vector<LandRegion> &landRegions, int seaRegionCount,
                           const Parameters &params)
    : guidingPairs(landRegions.size() - 1), configurations(landRegions.size() + seaRegionCount) {
	// compute painting constants
	const double sqrt3 = std::sqrt(3);
	tileArea = params.tileArea();
	inradius = std::sqrt(tileArea / (2*sqrt3));
	exradius = inradius * 2/sqrt3;
	for (int i = 0; i < 6; i++) {
		const auto a = (1 + 2*i) * std::numbers::pi / 6;
		tileShape.push_back({
			exradius * std::cos(a),
			exradius * std::sin(a)
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
	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			int i = initial.grid[x+1][h-y];  // vertical flip
			if (i < configurations.size())  // don't add tiles for outer sea regions
				configurations[i].tiles.insert({ x+y, y });  // convert grid coordinate to barycentric coordinate
		}
	}
	for (int i = 0; i < configurations.size(); i++)
		for (const Coordinate &c : configurations[i].tiles)
			tiles[c] = i;
}

Point<Inexact> HexagonalMap::getCentroid(const Configuration &config) const {
	Vector<Inexact> v(0, 0);
	for (auto c : config.tiles) v += getCentroid(c) - CGAL::ORIGIN;
	return CGAL::ORIGIN + v / config.tiles.size();
}

Point<Inexact> HexagonalMap::getCentroid(const Coordinate &c) const {
	return {
		inradius * (2*c.x() - c.y()),
		exradius * 3/2 * c.y()
	};
}

std::pair<Ellipse, Ellipse> HexagonalMap::getGuidingPair(const Configuration &c1, const Configuration &c2) const {
	if (c1.isSea() || c2.isSea())
		throw std::invalid_argument("Sea regions do not have a guiding shape");

	int id1 = c1.id();
	int id2 = c2.id();

	if (id1 == id2)
		throw std::invalid_argument("A guiding pair only exists for distinct regions");

	// compute centroid of union of configurations
	const int size1 = c1.tiles.size();
	const int size2 = c2.tiles.size();
	const auto centroid = (size1 * (getCentroid(c1) - CGAL::ORIGIN)
	                     + size2 * (getCentroid(c2) - CGAL::ORIGIN)) / (size1 + size2);

	// get precomputed guiding pair and translate to `centroid`
	if (id1 > id2) std::swap(id1, id2);
	const auto p = guidingPairs[id1].at(id2).translate(centroid);  // throws exception if regions are not neighbors
	return id1 == c1.id() ? p : std::make_pair(p.second, p.first);
}

void HexagonalMap::paintTile(renderer::GeometryRenderer &renderer, const Coordinate &c) const {
	// https://stackoverflow.com/a/45452642
	CGAL::Aff_transformation_2<Inexact> t(CGAL::TRANSLATION, getCentroid(c) - CGAL::ORIGIN);
	renderer.draw(CGAL::transform(t, tileShape));
}

void HexagonalMap::paint(renderer::GeometryRenderer &renderer, ColorFunction tileColor) const {
	renderer.pushStyle();
	renderer.setMode(renderer::GeometryRenderer::fill | renderer::GeometryRenderer::stroke);
	renderer.setStroke(COLOR_BORDER, inradius / 5);

	for (const Configuration &config : configurations) {
		for (const Coordinate &c : config.tiles) {
			renderer.setFill(tileColor(c));
			paintTile(renderer, c);
		}
	}

	renderer.popStyle();  // restore style
}

} // namespace cartocrow::mosaic_cartogram
