#include "mosaic_cartogram.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <stdexcept>
#include <unordered_set>
#include <CGAL/number_utils.h>

#include "../core/centroid.h"
#include "triangulation.h"

namespace cartocrow::mosaic_cartogram {

void MosaicCartogram::compute() {
	validate();
	computeLandRegions();
	computeArrangement();
	computeDual();
	computeTileMap();
}

void MosaicCartogram::computeArrangement() {
	RegionMap map;
	map.reserve(m_landRegions.size());
	for (const auto &r : m_landRegions) map.insert({ r.name, r.basic() });
	m_arrangement = regionMapToArrangement(map);

	// (temp) ensure that all salient points exactly equal one vertex point
	// this is necessary since the input map may contain "rounding errors"
	for (auto pit = m_salientPoints.begin(); pit != m_salientPoints.end(); ++pit) {
		const Point<Exact> *nearest = nullptr;
		Number<Exact> nearestDistance;

		for (const auto vit : m_arrangement.vertex_handles()) {
			const Point<Exact> &q = vit->point();
			const Number<Exact> d = CGAL::squared_distance(*pit, q);
			if (!nearest || d < nearestDistance) nearest = &q, nearestDistance = d;
		}

		*pit = *nearest;
	}

	// add sea regions such that dual is triangular
	m_seaRegionCount = triangulate(m_arrangement, m_salientPoints);

	// (temp) since Moldova was removed during triangulation, we remove the corresponding land
	// region as well (and then reassign indices)
	m_landRegions.erase(m_landRegions.begin() + m_landIndices.at("MDA"));
	m_landIndices.clear();
	int i = 0;
	for (auto &r : m_landRegions) {
		r.id = i++;
		m_landIndices.insert({ r.name, r.id });
	}
}

void MosaicCartogram::computeDual() {
	m_dual = UndirectedGraph(getRegionCount());
	for (const auto fit : m_arrangement.face_handles()) {
		if (fit->is_unbounded()) continue;  // all other faces (should) have a label

		// get region index corresponding to face
		const std::string &vName = fit->data();
		const int v = getRegionIndex(vName);

		auto circ = fit->outer_ccb();

		// for the three outer sea regions, start at the unbounded face
		if (vName.starts_with("_outer"))
			while (!circ->twin()->face()->is_unbounded())
				++circ;

		// walk along boundary to find adjacent regions
		auto curr = circ;
		std::vector<int> adj;
		do {
			const std::string &uName = curr->twin()->face()->data();
			if (uName.empty()) continue;
			const int u = getRegionIndex(uName);
			if (std::find(adj.begin(), adj.end(), u) == adj.end()) adj.push_back(u);
		} while (--curr != circ);  // in reverse order (so clockwise)

		// add adjacencies to dual
		m_dual.setAdjacenciesUnsafe(v, adj);

		// if `v` is a land region, add adjacencies as neighbors
		if (isLandRegion(v)) {
			LandRegion &region = m_landRegions[v];
			for (int u : adj)
				if (isLandRegion(u))
					region.neighbors.push_back(m_landRegions[u]);
		}
	}
}

void MosaicCartogram::computeLandRegions() {
	// POD for internal use
	struct Part {
		const PolygonWithHoles<Exact> *shape;  // pointer to avoid copies (has no ownership!)
		Number<Exact> area;
		Number<Inexact> value;
		int tiles;
	};

	for (const auto &[name, region] : *m_inputMap) {
		const Number<Inexact> value = m_dataValues.at(name);  // already validated
		int tiles = getTileCount(value);
		if (!tiles) {
			// TODO: how to handle?
			std::cerr << "[warning] " << name << " is too small\n";
		}

		std::vector<PolygonWithHoles<Exact>> polygons;
		region.shape.polygons_with_holes(std::back_inserter(polygons));

		// simple case: the region is contiguous
		if (polygons.size() == 1) {
			const auto &p = polygons[0];
			m_landRegions.push_back({
				0, name, std::nullopt, value, tiles, region.color, p, computeGuidingShape(p, tiles)
			});
			continue;
		}

		// compute area of each part
		std::vector<Part> parts;
		Number<Exact> totalArea = 0;
		for (const auto &p : polygons) {
			const auto a = area(p);
			parts.push_back({ &p, a });
			totalArea += a;
		}

		// sort parts from largest to smallest area
		std::sort(parts.begin(), parts.end(), [](const auto &p1, const auto &p2) {
			return p1.area > p2.area;
		});

		// allocate tiles
		// TODO: improve, like e.g. seats are assigned in parliament
		for (auto &p : parts) {
			p.value = approximate(p.area / totalArea * value);
			const int n = std::min(getTileCount(p.value), tiles);
			p.tiles = n;
			tiles -= n;
		}
		parts[0].tiles += tiles;  // assign any remaining tiles to the largest part

		int i = 0;
		for (const auto &p : parts) {
			if (!p.tiles) {
				// TODO: redistribute remaining value?
				// TODO: how to handle holes? (if they're not adjacent to sea)
				std::cerr << "[warning] " << parts.size() - i << " subregion(s) of " << name
				          << " are too small and have been removed\n";
				break;
			}
			m_landRegions.push_back({
				0,
				name + '_' + std::to_string(i++),
				name,
				p.value,
				p.tiles,
				region.color,
				*p.shape,
				computeGuidingShape(*p.shape, p.tiles)
			});
		}
	}
	std::cerr << std::flush;

	// sort land regions by name and assign indices in that order
	std::sort(m_landRegions.begin(), m_landRegions.end(), [](const auto &r1, const auto &r2) {
		return r1.name < r2.name;
	});
	int i = 0;
	for (auto &r : m_landRegions) {
		r.id = i++;
		m_landIndices.insert({ r.name, r.id });
	}
}

void MosaicCartogram::computeTileMap() {
	std::vector<Point<Exact>> centroids(getRegionCount());
	for (const auto f : m_arrangement.face_handles()) {
		const std::string label = f->data();
		if (!label.empty()) centroids[getRegionIndex(label)] = centroid(f);
	}

	VisibilityDrawing vd(
		m_dual,
		getRegionIndex("_outer0"),
		getRegionIndex("_outer1"),
		getRegionIndex("_outer2"),
		centroids
	);

	m_tileMap = HexagonalMap(vd, m_landRegions, m_seaRegionCount);
}

void MosaicCartogram::validate() const {
	// validate region names and data values
	for (const auto &[name, region] : *m_inputMap) {
		if (name.empty()) {
			throw std::logic_error("Region names cannot be empty");
		}
		if (name.find('_') != std::string::npos) {
			throw std::logic_error("The region name '" + name + "' is illegal; it cannot contain underscores");
		}
		if (!m_dataValues.contains(name)) {
			throw std::logic_error("No data value is specified for region '" + name + "'");
		}
		const double v = m_dataValues.at(name);
		if (!std::isfinite(v) || v < 0) {
			throw std::logic_error("Region '" + name + "' has an illegal data value; it must be non-negative");
		}
	}

	// report on any ignored data
	std::vector<std::string> ignored;
	for (const auto &[name, region] : m_dataValues)
		if (!m_inputMap->contains(name))
			ignored.push_back(name);
	if (!ignored.empty()) {
		std::sort(ignored.begin(), ignored.end());
		std::cerr << "[warning] For the following regions, data was provided, but they are not on the map:";
		for (const auto &s : ignored) std::cerr << ' ' << s << ',';
		std::cerr << "\b " << std::endl;
	}
}

} // namespace cartocrow::mosaic_cartogram
