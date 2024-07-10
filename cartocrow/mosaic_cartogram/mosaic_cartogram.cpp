#include "mosaic_cartogram.h"

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cmath>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <utility>
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

void MosaicCartogram::absorbMoldova() {
	std::vector<RegionArrangement::Edge_iterator> remove;
	for (auto e : m_arrangement.edge_handles())
		if (e->face()->data() == "MDA" && e->twin()->face()->data() == "UKR" || e->face()->data() == "UKR" && e->twin()->face()->data() == "MDA")
			remove.push_back(e);
	for (auto e : remove) {
		auto f = m_arrangement.remove_edge(e);
		f->set_data("UKR");  // this is actually only necessary once
	}
	std::cerr << "[info] removed " << remove.size() << " edges to absorb MDA into UKR" << std::endl;

	// remove the corresponding land region as well (and then reassign indices)
	m_landRegions.erase(m_landRegions.begin() + m_regionIndices.at("MDA"));
	m_regionIndices.erase("MDA");
	int i = 0;
	for (auto &r : m_landRegions) {
		r.id = i++;
		m_regionIndices[r._name] = r.id;
	}
	for (const auto &r : m_seaRegions) m_regionIndices[r.name()]--;
}

PolygonWithHoles<Exact> getShape(const RegionArrangement::Face_const_handle face) {
	Polygon<Exact> polygon;
	const auto circ = face->outer_ccb();
	auto curr = circ;
	int n = 0;
	do {
		const auto p = curr->source()->point();
		n++;
		polygon.push_back(p);
	} while (++curr != circ);
	std::cout << n << std::endl;
	return PolygonWithHoles<Exact>(polygon);
}

void MosaicCartogram::computeArrangement() {
	// if sea regions were manually specified, read them now
	if (m_parameters.manualSeas) {
		for (const auto &[name, region] : *m_inputMap) {
			if (name.starts_with("_sea")) {
				std::vector<PolygonWithHoles<Exact>> ps;
				region.shape.polygons_with_holes(std::back_inserter(ps));

				SeaRegion r;
				r.id = parseIntAtEnd(name);
				r.shape = ps[0];
				r.guidingShape = computeGuidingShape(r.shape);
				m_seaRegions.push_back(r);

				assert(r.name() == name);
				m_regionIndices.insert({ name, r.id + m_landRegions.size() });
			}
		}

	}

	// construct arrangement from processed regions
	RegionMap map;
	for (const auto &r : m_landRegions) map.insert({ r._name, r.basic() });
	for (const auto &p : *m_inputMap)
		if (p.first.starts_with('_'))
			map.insert(p);
	m_arrangement = regionMapToArrangement(map);

	// if sea regions were not manually specified, generate them now
	if (!m_parameters.manualSeas) {
		// ensure that all salient points exactly equal one vertex point
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
		triangulate(m_arrangement, m_salientPoints);

		// extract sea regions from new faces
		for (const RegionArrangement::Face_const_handle fit : m_arrangement.face_handles()) {
			const std::string &name = fit->data();
			if (name.starts_with("_sea")) {
				SeaRegion r;
				r.id = parseIntAtEnd(name);
				r.shape = getShape(fit);
				r.guidingShape = computeGuidingShape(r.shape);
				m_seaRegions.push_back(r);
				m_regionIndices.insert({ name, r.id + m_landRegions.size() });
			}
		}
	}

	// (temp) solve Europe-specific problem
	absorbMoldova();

	// add indices for three outer sea regions
	const int i = m_landRegions.size() + m_seaRegions.size();
	m_regionIndices.insert({ "_outer0", i   });
	m_regionIndices.insert({ "_outer1", i+1 });
	m_regionIndices.insert({ "_outer2", i+2 });
}

void MosaicCartogram::computeDual() {
	m_dual = UndirectedGraph(getNumberOfRegions());
	for (const auto fit : m_arrangement.face_handles()) {
		if (fit->is_unbounded()) continue;  // all other faces (should) have a label

		// get region index corresponding to face
		const std::string &vName = fit->data();
		const int v = m_regionIndices[vName];

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
			const int u = m_regionIndices[uName];
			if (std::find(adj.begin(), adj.end(), u) == adj.end()) adj.push_back(u);
		} while (--curr != circ);  // in reverse order (so clockwise)

		// add adjacencies to dual
		m_dual.setAdjacenciesUnsafe(v, adj);
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
		if (name[0] == '_') continue;  // skip sea regions (if they were defined manually)

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
			LandRegion r;
			r._name = name;
			r._color = region.color;
			r.dataValue = value;
			r.targetTileCount = tiles;
			r.shape = p;
			r.guidingShape = computeGuidingShape(p, tiles);
			m_landRegions.push_back(std::move(r));
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
			p.value = CGAL::to_double(p.area / totalArea * value);
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
			LandRegion r;
			r._name = name + '_' + std::to_string(i++);
			r.superName = name;
			r._color = region.color;
			r.dataValue = p.value;
			r.targetTileCount = p.tiles;
			r.shape = *p.shape;
			r.guidingShape = computeGuidingShape(*p.shape, p.tiles);
			m_landRegions.push_back(std::move(r));
		}
	}
	std::cerr << std::flush;

	// sort land regions by name and assign indices in that order
	std::sort(m_landRegions.begin(), m_landRegions.end(), [](const auto &r1, const auto &r2) {
		return r1._name < r2._name;
	});
	int i = 0;
	for (auto &r : m_landRegions) {
		r.id = i++;
		m_regionIndices.insert({ r._name, r.id });
	}
}

void MosaicCartogram::computeTileMap() {
	std::vector<Point<Exact>> centroids(getNumberOfRegions());
	for (const auto f : m_arrangement.face_handles()) {
		const std::string label = f->data();
		if (!label.empty()) centroids[m_regionIndices[label]] = centroid(f);
	}

	VisibilityDrawing vd(
		m_dual,
		m_regionIndices["_outer0"],
		m_regionIndices["_outer1"],
		m_regionIndices["_outer2"],
		centroids
	);

	m_tileMap = HexagonalMap(vd, m_landRegions, m_seaRegions);
}

void MosaicCartogram::validate() const {
	if (m_parameters.manualSeas != m_salientPoints.empty()) {
		throw std::logic_error("There must be no salient points if and only if manual seas are specified");
	}

	// validate region names and data values
	for (const auto &[name, region] : *m_inputMap) {
		if (name.empty()) {
			throw std::logic_error("Region names cannot be empty");
		}
		if (name.find('_', m_parameters.manualSeas ? 1 : 0) != std::string::npos) {
			throw std::logic_error("The region name '" + name + "' contains illegal underscores");
		}
		if (name[0] == '_') {
			if (!name.starts_with("_sea") && !name.starts_with("_outer")) {  // TODO: also validate numbering (format and missing values)
				throw std::logic_error("The region name '" + name + "' is illegal; the only legal special names are '_sea' and '_outer'");
			}
			if (m_dataValues.contains(name)) {
				throw std::logic_error("A data value is specified for region '" + name + "', but sea regions may not have a value");
			}
			std::vector<PolygonWithHoles<Exact>> ps;
			region.shape.polygons_with_holes(std::back_inserter(ps));
			if (ps.size() != 1 || ps[0].has_holes()) {
				throw std::logic_error("Region '" + name + "' must consist of one polygon without holes");
			}
		} else {
			if (!m_dataValues.contains(name)) {
				throw std::logic_error("No data value is specified for region '" + name + "'");
			}
			const double v = m_dataValues.at(name);
			if (!std::isfinite(v) || v < 0) {
				throw std::logic_error("Region '" + name + "' has an illegal data value; it must be non-negative");
			}
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

int MosaicCartogram::parseIntAtEnd(const std::string &s) {
	int n = 0, i = 1;
	for (auto c = s.rbegin(); std::isdigit(*c); ++c) n += i * (*c - '0'), i *= 10;
	return n;
}

} // namespace cartocrow::mosaic_cartogram
