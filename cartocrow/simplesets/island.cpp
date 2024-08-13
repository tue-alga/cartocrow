#include "island.h"
#include "point_voronoi_helpers.h"
#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/convex_hull_2.h>
#include <CGAL/bounding_box.h>
#include "cropped_voronoi.h"

namespace cartocrow::simplesets {
Polygon<K> convexHull(const std::vector<Point<K>>& points) {
    std::vector<Point<K>> chPoints;
	CGAL::convex_hull_2(points.begin(), points.end(), std::back_inserter(chPoints));
	return {chPoints.begin(), chPoints.end()};
}

std::optional<Polygon<K>> convexIntersection(const Polygon<K> p, const Polygon<K> q) {
	std::vector<PolygonWithHoles<K>> intersection_result;
	CGAL::intersection(p, q, std::back_inserter(intersection_result));
	if (intersection_result.empty()) {
		return std::nullopt;
	} else {
		return intersection_result[0].outer_boundary();
	}
}

Number<K> coverRadiusOfPoints(const std::vector<Point<K>>& points) {
	DT dt;
	dt.insert(points.begin(), points.end());

	Rectangle<K> bbox = CGAL::bounding_box(points.begin(), points.end());
	std::optional<Number<K>> squaredCoverRadius;

	auto hull = convexHull(points);
	Cropped_voronoi_from_delaunay cropped_voronoi(hull);

	for (auto eit = dt.finite_edges_begin(); eit != dt.finite_edges_end(); ++eit) {
		CGAL::Object o = dt.dual(eit);
		Line<K> l;
		Ray<K> r;
		Segment<K> s;
		auto site = eit->first->vertex(dt.cw(eit->second))->point();
		if(CGAL::assign(s,o)) cropped_voronoi << std::pair(site, s);
		if(CGAL::assign(r,o)) cropped_voronoi << std::pair(site, r);
		if(CGAL::assign(l,o)) cropped_voronoi << std::pair(site, l);
	}

	for (const auto& [site, seg] : cropped_voronoi.m_cropped_vd) {
		for (const auto& v : {seg.source(), seg.target()}) {
			auto d = squared_distance(v, site);
			if (!squaredCoverRadius.has_value() || d > *squaredCoverRadius) {
				squaredCoverRadius = d;
			}
		}
	}

	return sqrt(*squaredCoverRadius);
}

Island::Island(std::vector<CatPoint> catPoints): m_catPoints(std::move(catPoints)) {
	// Store the point positions separately, sometimes only the positions are needed.
	std::transform(m_catPoints.begin(), m_catPoints.end(), std::back_inserter(m_points), [](const CatPoint& cp) {
		return cp.point;
	});

	m_coverRadius = coverRadiusOfPoints(m_points);
	m_polygon = convexHull(m_points);
}

std::variant<Polyline<K>, Polygon<K>> Island::contour() {
	return m_polygon;
}

std::vector<CatPoint> Island::catPoints() {
	return m_catPoints;
}

bool Island::isValid(cartocrow::simplesets::GeneralSettings gs) {
	return true;
}

Number<K> Island::coverRadius() {
	return m_coverRadius;
}
}