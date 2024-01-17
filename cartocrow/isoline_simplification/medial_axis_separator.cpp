/*
The CartoCrow library implements algorithmic geo-visualization methods,
developed at TU Eindhoven.
Copyright (C) 2021  Netherlands eScience Center and TU Eindhoven

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3f of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "medial_axis_separator.h"
#include <CGAL/Voronoi_diagram_2.h>
#include <CGAL/Segment_Delaunay_graph_adaptation_traits_2.h>
#include <unordered_set>

typedef CGAL::Segment_Delaunay_graph_adaptation_traits_2<SDG2> AT;
typedef CGAL::Segment_Delaunay_graph_degeneracy_removal_policy_2<SDG2> AP;
typedef CGAL::Voronoi_diagram_2<SDG2, AT, AP> VD;

namespace cartocrow::isoline_simplification {
Separator medial_axis_separator(const SDG2& delaunay, std::vector<Isoline<K>>& isolines) {
//	std::unordered_map<SDG2::Point_2, Isoline<K>*> point_to_isoline;
//
//	for (auto& isoline : isolines) {
//		for (auto p : isoline.m_points) {
//			point_to_isoline[p] = &isoline;
//		}
//	}

//	std::unordered_map<Isoline<K>*, std::vector<SDG2::Edge>> edges;

//	auto contained = [&point_to_isoline](SDG2::Edge edge) {
//		SDG2::Site_2 p = edge.first->vertex(SDG2::cw(edge.second))->site();
//		SDG2::Site_2 q = edge.first->vertex(SDG2::ccw(edge.second))->site();
//
//		auto point_of_site = [](SDG2::Site_2 site) {
//			SDG2::Point_2 point;
//			if (site.is_point()) {
//				point = site.point();
//			} else {
//				point = site.source();
//			}
//			return point;
//		};
//		SDG2::Point_2 p_point = point_of_site(p);
//		SDG2::Point_2 q_point = point_of_site(q);
//
//		auto p_iso = point_to_isoline[p_point];
//		auto q_iso = point_to_isoline[q_point];
//
//		return p_iso != q_iso;
//	};
//
//	VD voronoi(delaunay);
//
//
//	std::unordered_set<SDG2::Edge> subset;

//	subset.insert(*delaunay.finite_edges_begin());


//	while (true) {
//		VD::Halfedge current;
//
//		// Find a half-edge contained in the separator
//		bool found = false;
//		for (auto eit = voronoi.bounded_halfedges_begin(); eit != voronoi.bounded_halfedges_end();
//		     ++eit) {
//			if (contained(eit->dual())) {
//				current = *eit;
//				found = true;
//			}
//		}
//		if (!found) break;
//
//		auto v = current.source();
//		auto start = v->incident_halfedges();
//		auto eit = start;
//		do {
//			if (contained(eit->dual())) {
//
//			}
//		}
//		while (++eit != start);
//	}
//	if (contained(first.next()->dual())) {
//
//	}

	std::unordered_map<SDG2::Point_2, std::pair<Isoline<K>*, SDG2::Point_2>> point_to_isoline;

	for (auto& isoline : isolines) {
		for (int i = 0; i < isoline.m_points.size(); i++) {
			Isoline<K>* pointer = &isoline;
			auto p = isoline.m_points[i];
			SDG2::Point_2 next;
			if (i == isoline.m_points.size() - 1) {
				next = isoline.m_points[i+1];
			} else {
				next = p + (p - isoline.m_points[i - 1]);
			}
			point_to_isoline[p] = std::pair(pointer, next);
		}
	}

	std::unordered_map<Isoline<K>*, std::vector<SDG2::Edge>> edges;

	for (auto eit = delaunay.finite_edges_begin(); eit != delaunay.finite_edges_end(); ++eit) {
		SDG2::Edge edge = *eit;
		SDG2::Site_2 p = edge.first->vertex(SDG2::cw(edge.second))->site();
		SDG2::Site_2 q = edge.first->vertex(SDG2::ccw(edge.second))->site();
		auto point_of_site = [](SDG2::Site_2 site) {
			SDG2::Point_2 point;
			if (site.is_point()) {
				point = site.point();
			} else {
				point = site.source();
			}
			return point;
		};
		SDG2::Point_2 p_point = point_of_site(p);
		SDG2::Point_2 q_point = point_of_site(q);

		auto [p_iso, p_next] = point_to_isoline[p_point];
		auto [q_iso, q_next] = point_to_isoline[q_point];

		if (p_iso != q_iso) {
			CGAL::Object o = delaunay.primal(edge);
			typename Gt::Segment_2 s;
			CGAL::Parabola_segment_2<Gt> ps;

			Gt::Point_2 point_on_Voronoi_edge;
			if (CGAL::assign(s, o)) {
				point_on_Voronoi_edge = midpoint(s);
			}
			if (CGAL::assign(ps, o)) {
				// Roundabout way to obtain start and end of parabolic segment because they are protected -_-
				std::vector<typename Gt::Point_2> pts;
				ps.generate_points(pts, (typename Gt::FT)(1000000));
				auto start = pts.front();
				auto end = pts.back();
				point_on_Voronoi_edge = start;
			}

			if (CGAL::right_turn(p_point, p_next, point_on_Voronoi_edge)) {
				edges[p_iso].push_back(edge);
			} else {
				edges[q_iso].push_back(edge);
			}
		}
	}

	return edges;
}

std::variant<Gt::Point_2, Gt::Segment_2> site_projection(const SDG2& delaunay, const SDG2::Edge& edge, const SDG2::Site_2& site) {
	if (site.is_point()) {
		return { site.point() };
	} else {
		typename Gt::Line_2    l;
		typename Gt::Segment_2 s;
		typename Gt::Ray_2     r;
		CGAL::Parabola_segment_2<Gt> ps;

		CGAL::Object o = delaunay.primal(edge);

		// Parabolic segment and line cases cannot occur because they require both sites to be a point (?)
		if (CGAL::assign(s, o)) {
			auto start = site.segment().supporting_line().projection(s.source());
			auto end = site.segment().supporting_line().projection(s.end());
			return { Segment<Inexact>(start, end) };

		}
		if (CGAL::assign(ps, o)) {
			// Roundabout way to obtain start and end of parabolic segment because they are protected -_-
			std::vector<typename Gt::Point_2> pts;
			ps.generate_points(pts, (typename Gt::FT)(1000000));
			auto start = site.segment().supporting_line().projection(pts.front());
			auto end = site.segment().supporting_line().projection(pts.back());
			return { Segment<Inexact>(start, end) };
		}
	}
}

Gt::Segment_2 snap_endpoints(Gt::Segment_2 proj, Gt::Segment_2 original) {
	Gt::Point_2 start;
	if (compare_distance_to_point(proj.source(), original.source(), original.target()) == CGAL::SMALLER) {
		start = original.source();
	} else {
		start = original.target();
	}
	Gt::Point_2 end;
	if (compare_distance_to_point(proj.target(), original.source(), original.target()) == CGAL::SMALLER) {
		end = original.source();
	} else {
		end = original.target();
	}
	return { start, end };
}

std::vector<Gt::Segment_2> matching(const SDG2& delaunay, const Separator& separator) {
	std::vector<Gt::Segment_2> matching;

	for (auto& [_, edges]: separator)
	for (auto edge : edges) {
		SDG2::Site_2 p = edge.first->vertex(SDG2::cw(edge.second))->site();
		SDG2::Site_2 q = edge.first->vertex(SDG2::ccw(edge.second))->site();

		if (p.is_point() && q.is_point()) {
			matching.emplace_back(p.point(), q.point());
		} else if (p.is_point() && q.is_segment()) {
			auto q_seg = snap_endpoints(std::get<Segment<Inexact>>(site_projection(delaunay, edge, q)), q.segment());

			matching.emplace_back(p.point(), q_seg.source());
			matching.emplace_back(p.point(), q_seg.target());
		} else if (p.is_segment() && q.is_segment()){
			auto p_seg = snap_endpoints(std::get<Segment<Inexact>>(site_projection(delaunay, edge, p)), p.segment());
			auto q_seg = snap_endpoints(std::get<Segment<Inexact>>(site_projection(delaunay, edge, q)), q.segment());
			matching.emplace_back(p_seg.source(), q_seg.source());
			matching.emplace_back(p_seg.target(), q_seg.target());
		}
	}

	return matching;
}
}