#include "island.h"
#include "bank.h"
#include "cartocrow/simplesets/helpers/cropped_voronoi.h"
#include "cartocrow/simplesets/helpers/point_voronoi_helpers.h"
#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/bounding_box.h>
#include <CGAL/convex_hull_2.h>

namespace cartocrow::simplesets {
Polygon<Inexact> convexHull(const std::vector<Point<Inexact>>& points) {
    std::vector<Point<Inexact>> chPoints;
	CGAL::convex_hull_2(points.begin(), points.end(), std::back_inserter(chPoints));
	return {chPoints.begin(), chPoints.end()};
}

std::optional<Polygon<Inexact>> convexIntersection(const Polygon<Inexact>& p, const Polygon<Inexact>& q) {
	std::vector<PolygonWithHoles<Inexact>> intersection_result;
	CGAL::intersection(p, q, std::back_inserter(intersection_result));
	if (intersection_result.empty()) {
		return std::nullopt;
	} else {
		return intersection_result[0].outer_boundary();
	}
}

Number<Inexact> coverRadiusOfPoints(const std::vector<Point<Inexact>>& points) {
	DT dt;
	auto exact_points = makeExact(points);
	dt.insert(exact_points.begin(), exact_points.end());

	Rectangle<Inexact> bbox = CGAL::bounding_box(points.begin(), points.end());
	std::optional<Number<Inexact>> squaredCoverRadius;

	auto hull = makeExact(convexHull(points));
	Cropped_voronoi_from_delaunay cropped_voronoi(hull);

	for (auto eit = dt.finite_edges_begin(); eit != dt.finite_edges_end(); ++eit) {
		CGAL::Object o = dt.dual(eit);
		Line<Exact> l;
		Ray<Exact> r;
		Segment<Exact> s;
		auto site = eit->first->vertex(dt.cw(eit->second))->point();
		if(CGAL::assign(s,o)) cropped_voronoi << std::pair(site, s);
		if(CGAL::assign(r,o)) cropped_voronoi << std::pair(site, r);
		if(CGAL::assign(l,o)) cropped_voronoi << std::pair(site, l);
	}

	for (const auto& [site, seg] : cropped_voronoi.m_cropped_vd) {
		for (const auto& v : {seg.source(), seg.target()}) {
			auto d = squared_distance(approximate(v), approximate(site));
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

	if (m_catPoints.size() >= 3) {
		bool collinear = true;
		for (int i = 0; i < m_catPoints.size() - 2; ++i) {
			if (!CGAL::collinear(m_catPoints[i].point, m_catPoints[i+1].point, m_catPoints[i+2].point)) {
				collinear = false;
			}
		}
		if (collinear) {
			Bank bank(m_catPoints);
			m_coverRadius = bank.coverRadius();
			m_poly = bank.poly();
			return;
		}
	}

	m_coverRadius = coverRadiusOfPoints(m_points);
	m_poly = convexHull(m_points);
}

std::variant<Polyline<Inexact>, Polygon<Inexact>> Island::poly() const {
	return m_poly;
}

const std::vector<CatPoint>& Island::catPoints() const {
	return m_catPoints;
}

Number<Inexact> Island::coverRadius() const {
	return m_coverRadius;
}
}