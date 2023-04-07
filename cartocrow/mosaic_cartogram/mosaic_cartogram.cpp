#include "mosaic_cartogram.h"

#include <algorithm>

#include "triangulation.h"

namespace cartocrow::mosaic_cartogram {

MosaicCartogram::MosaicCartogram(const std::shared_ptr<RegionMap> map, const std::vector<Point<Exact>> &salient)
    : m_origMap(map), m_salient(salient), m_dual(0) {}

void MosaicCartogram::compute() {
	// transform `m_origMap` such that each region is a singleton, i.e., consists of one polygon
	// each new region gets a unique id: old id + index, where a higher index indicates a smaller region
	for (const auto &[id, region] : *m_origMap) {
		std::vector<PolygonWithHoles<Exact>> polygons;
		region.shape.polygons_with_holes(std::back_inserter(polygons));

		// sort from largest to smallest area
		std::sort(polygons.begin(), polygons.end(), [](const auto &p1, const auto &p2) {
			return area(p1) > area(p2);
		});

		Region main;
		main.name = region.name;
		main.color = region.color;
		main.shape = PolygonSet<Exact>(polygons.front());
		m_map[main.name] = main;

		int i = 2;
		for (auto pit = polygons.begin() + 1; pit != polygons.end(); ++pit) {
			Region r;
			r.name = region.name + std::to_string(i++);
			r.color = region.color;
			r.shape = PolygonSet<Exact>(*pit);
			m_map[r.name] = r;
		}
	}

	// create arrangement from input map
	m_arr = regionMapToArrangement(m_map);

	// (temp) ensure that all salient points exactly equal one vertex point
	// this is necessary since the input map may contain "rounding errors"
	snapToVertices(m_salient);

	// add sea regions such that dual is triangular
	m_arr = triangulate(m_arr, m_salient);
	{
		int i = 0;
		for (const auto f : m_arr.face_handles())
			if (!f->is_unbounded() && f->data().empty())
				f->set_data("_sea" + std::to_string(i++));
	}

	// create bidirectional mapping country <--> index
	std::unordered_set<std::string> countries;
	for (const auto f : m_arr.face_handles())
		if (!f->is_unbounded())
			countries.insert(f->data());

	m_indexToCountry = std::vector<std::string>(countries.begin(), countries.end());
	std::sort(m_indexToCountry.begin(), m_indexToCountry.end());

	{
		int i = 0;
		for (const std::string &c : m_indexToCountry) m_countryToIndex[c] = i++;
	}

	// compute dual of `m_arr`: create vertex for each face and connect vertices if the corresponding faces are adjacent
	m_dual = UndirectedGraph(countries.size());
	for (const auto fit : m_arr.face_handles()) {
		if (fit->data().empty()) continue;

		const int v = m_countryToIndex[fit->data()];
		std::vector<int> adj;

		auto circ = fit->outer_ccb();

		// for the three outer faces, start at the unbounded face
		if (fit->data()[1] == 'o')  // TODO: more robust check
			while (!circ->twin()->face()->is_unbounded())
				++circ;

		auto curr = circ;
		do {
			const auto &s = curr->twin()->face()->data();
			if (s.empty()) continue;
			const int u = m_countryToIndex[s];
			if (std::find(adj.begin(), adj.end(), u) == adj.end()) adj.push_back(u);
		} while (++curr != circ);

		std::reverse(adj.begin(), adj.end());  // reverse `adj` from counterclockwise to clockwise order
		m_dual.setAdjacenciesUnsafe(v, adj);
	}

	// print country name for each index
	// for (int i = 0; i < countries.size(); i++) std::cout << i << " : " << m_indexToCountry[i] << '\n';
	// std::cout << std::endl;

	// print adjacencies for each country
	// for (int i = 0; i < countries.size(); i++) {
	// 	std::cout << "adj[" << i << "] : ";
	// 	for (int j : m_dual.getNeighbors(i)) std::cout << j << ", ";
	// 	std::cout << std::endl;
	// }
}

void MosaicCartogram::snapToVertices(std::vector<Point<Exact>> &points) {
	for (auto pit = points.begin(); pit != points.end(); ++pit) {
		const Point<Exact> *nearest = nullptr;
		Number<Exact> nearestDistance;

		for (const RegionArrangement::Vertex_iterator vit : m_arr.vertex_handles()) {
			const Point<Exact> &q = vit->point();
			const Number<Exact> d = CGAL::squared_distance(*pit, q);
			if (!nearest || d < nearestDistance) nearest = &q, nearestDistance = d;
		}

		*pit = *nearest;
	}
}

} // namespace cartocrow::mosaic_cartogram
