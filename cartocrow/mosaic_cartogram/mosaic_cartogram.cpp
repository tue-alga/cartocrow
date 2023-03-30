#include "mosaic_cartogram.h"

namespace cartocrow::mosaic_cartogram {

MosaicCartogram::MosaicCartogram(const std::shared_ptr<RegionMap> map, const std::vector<Point<Exact>> &salient)
    : m_origMap(map), m_salient(salient), m_dual(0) {}

void MosaicCartogram::compute() {
	// create `m_map` from `m_origMap` by only keeping largest polygon for each region
	for (const auto &[id, region] : *m_origMap) {
		auto [largest, area] = getLargest(region.shape);

		// (temp) skip tiny countries; they clutter the visualization
		if (area < 2) {
			std::cout << "remove " << id << '\n';
			continue;
		}

		Region singleton;
		singleton.name = region.name;
		singleton.color = region.color;
		singleton.shape = PolygonSet<Exact>(largest);

		m_map[id] = singleton;
	}

	// create arrangement from `m_map` (TODO)
	m_arr = regionMapToArrangement(*m_origMap);

	// (temp) ensure that all salient points exactly equal one vertex point
	// this is necessary since the input map may contain "rounding errors"
	snapToVertices(m_salient);

	// create bidirectional mapping country <--> index
	std::unordered_set<std::string> countries;
	for (const auto &[id, region] : m_map) countries.insert(id);

	m_indexToCountry = std::vector<std::string>(countries.begin(), countries.end());
	std::sort(m_indexToCountry.begin(), m_indexToCountry.end());

	int i = 0;
	for (std::string &c : m_indexToCountry) m_countryToIndex[c] = i++;

	// compute dual of `m_arr`: create vertex for each face and connect vertices if the corresponding faces are adjacent
	m_dual = UndirectedGraph(countries.size());
	for (auto eit = m_arr.edges_begin(); eit != m_arr.edges_end(); ++eit) {
		std::string &c1 = eit->face()->data();
		std::string &c2 = eit->twin()->face()->data();
		if (c1.empty() || c2.empty()) continue;
		m_dual.addEdge(m_countryToIndex[c1], m_countryToIndex[c2]);
	}
}

void MosaicCartogram::snapToVertices(std::vector<Point<Exact>> &points) {
	for (auto pit = points.begin(); pit != points.end(); ++pit) {
		const Point<Exact> *nearest;
		Number<Exact> nearestDistance = 1000000;

		for (const RegionArrangement::Vertex_iterator vit : m_arr.vertex_handles()) {
			const Point<Exact> &q = vit->point();
			const Number<Exact> d = CGAL::squared_distance(*pit, q);
			if (d < nearestDistance) nearest = &q, nearestDistance = d;
		}

		*pit = *nearest;
	}
}

std::pair<PolygonWithHoles<Exact>, Number<Exact>> MosaicCartogram::getLargest(const PolygonSet<Exact> &set) {
	// convert `PolygonSet` to vector of polygons
	std::vector<PolygonWithHoles<Exact>> polygons;
	set.polygons_with_holes(std::back_inserter(polygons));

	const PolygonWithHoles<Exact> *largest;
	Number<Exact> largestArea = 0;

	for (const auto &p : polygons) {
		// compute area of polygon (= outer minus each hole)
		Number<Exact> a = p.outer_boundary().area();
		for (const auto &h : p.holes()) a -= h.area();

		// update largest-so-far if area is greater
		if (a > largestArea) largest = &p, largestArea = a;
	}

	return std::make_pair(*largest, largestArea);
}

} // namespace cartocrow::mosaic_cartogram
