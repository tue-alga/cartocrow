#ifndef CARTOCROW_CROPPED_VORONOI_H
#define CARTOCROW_CROPPED_VORONOI_H

#include "cartocrow/core/core.h"
#include "cartocrow/simplesets/types.h"
#include <list>

using namespace cartocrow;
using namespace cartocrow::simplesets;

std::optional<Segment<Exact>> intersectionConvex(const Polygon<Exact>& polygon, const Ray<Exact>& ray) {
	auto sourceInside = polygon.has_on_bounded_side(ray.source());

	std::vector<Point<Exact>> inters;
	for (auto eit = polygon.edges_begin(); eit != polygon.edges_end(); eit++) {
		auto edge = *eit;
		auto obj = CGAL::intersection(ray, edge);
		if (obj.has_value()) {
			Segment<Exact> seg;
			Point<Exact> point;
			if (CGAL::assign(seg, obj)) {
				return seg;
			} else if (CGAL::assign(point, obj)) {
				inters.push_back(point);
			}
		}
	}

	assert(!(!sourceInside && inters.size() == 1));
	if (inters.empty()) {
		return std::nullopt;
	} else if (inters.size() == 2) {
		return Segment<Exact>(inters[0], inters[1]);
	} else {
		return Segment<Exact>(ray.source(), inters[0]);
	}
}

std::optional<Segment<Exact>> intersectionConvex(const Polygon<Exact>& polygon, const Line<Exact>& line) {
	std::vector<Point<Exact>> inters;
	for (auto eit = polygon.edges_begin(); eit != polygon.edges_end(); eit++) {
		auto edge = *eit;
		auto obj = CGAL::intersection(line, edge);
		if (obj.has_value()) {
			Segment<Exact> seg;
			Point<Exact> point;
			if (CGAL::assign(seg, obj)) {
				return seg;
			} else if (CGAL::assign(point, obj)) {
				inters.push_back(point);
			}
		}
	}

	assert(inters.size() != 1);
	if (inters.size() == 2) {
		return Segment<Exact>(inters[0], inters[1]);
	} else {
		return std::nullopt;
	}
}

std::optional<Segment<Exact>> intersectionConvex(const Polygon<Exact>& polygon, const Segment<Exact>& segment) {
	auto sourceInside = polygon.has_on_bounded_side(segment.source());
	auto targetInside = polygon.has_on_bounded_side(segment.target());
	if (sourceInside && targetInside) {
		return segment;
	}

	std::vector<Point<Exact>> inters;
	for (auto eit = polygon.edges_begin(); eit != polygon.edges_end(); eit++) {
		auto edge = *eit;
		auto obj = CGAL::intersection(segment, edge);
		if (obj.has_value()) {
			Segment<Exact> seg;
			Point<Exact> point;
			if (CGAL::assign(seg, obj)) {
				return seg;
			} else if (CGAL::assign(point, obj)) {
				inters.push_back(point);
			} else {
				throw std::runtime_error("Intersection between two line segments is not a point nor a line segment");
			}
		}
	}

	if (!sourceInside && !targetInside) {
		if (inters.empty()) return std::nullopt;
		assert(inters.size() == 2);
		// note that here, the orientation of the result segment may not be the same as the original segment.
		return Segment<Exact>(inters[0], inters[1]);
	}

	if (sourceInside) {
		assert(!targetInside);
		assert(inters.size() == 1);
		return Segment<Exact>(segment.source(), inters[0]);
	}

	assert(targetInside);
	assert(inters.size() == 1);
	return Segment<Exact>(inters[0], segment.target());
}

// This part is adapted from:
// https://github.com/CGAL/cgal/blob/master/Triangulation_2/examples/Triangulation_2/print_cropped_voronoi.cpp
// which falls under the CC0 license.

//A class to recover Voronoi diagram from stream.
//Rays, lines and segments are cropped to a polygon
struct Cropped_voronoi_from_delaunay{
	std::list<std::pair<Point<Exact>, Segment<Exact>>> m_cropped_vd;
	Polygon<Exact> m_clipper;
	Cropped_voronoi_from_delaunay(const Polygon<Exact>& clipper):m_clipper(clipper){}
	template <class RSL>
	void crop_and_extract_segment(const Point<Exact>& site, const RSL& rsl){
		auto s = intersectionConvex(m_clipper, rsl);
		if (s.has_value()) {
			m_cropped_vd.push_back({site, *s});
		}
	}
	void operator<<(std::pair<const Point<Exact>&, const Ray<Exact>&> site_ray)    { crop_and_extract_segment(site_ray.first, site_ray.second); }
	void operator<<(std::pair<const Point<Exact>&, const Line<Exact>&> site_line)  { crop_and_extract_segment(site_line.first, site_line.second); }
	void operator<<(std::pair<const Point<Exact>&, const Segment<Exact>&> site_seg){ crop_and_extract_segment(site_seg.first, site_seg.second); }
};

#endif //CARTOCROW_CROPPED_VORONOI_H
