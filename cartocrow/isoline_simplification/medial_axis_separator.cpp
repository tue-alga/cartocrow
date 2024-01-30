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
#include "types.h"
#include <CGAL/Segment_Delaunay_graph_adaptation_traits_2.h>
#include <CGAL/Voronoi_diagram_2.h>
#include <unordered_set>

namespace cartocrow::isoline_simplification {
typedef CGAL::Segment_Delaunay_graph_adaptation_traits_2<SDG2> AT;
typedef CGAL::Segment_Delaunay_graph_degeneracy_removal_policy_2<SDG2> AP;
typedef CGAL::Voronoi_diagram_2<SDG2, AT, AP> VD;

std::pair<SDG2::Site_2, SDG2::Site_2> defining_sites(const SDG2::Edge& edge) {
	return { edge.first->vertex(SDG2::cw(edge.second))->site(), edge.first->vertex(SDG2::ccw(edge.second))->site() };
}

SDG2::Point_2 point_of_site(const SDG2::Site_2& site) {
	SDG2::Point_2 point;
	if (site.is_point()) {
		point = site.point();
	} else {
		point = site.source();
	}
	return point;
}

Gt::Point_2 point_of_Voronoi_edge(const SDG2::Edge& edge, const SDG2& delaunay) {
	CGAL::Object o = delaunay.primal(edge);
	typename Gt::Segment_2 s;
	typename Gt::Line_2 l;
	typename Gt::Ray_2 r;
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
		point_on_Voronoi_edge = start;
	}
	if (CGAL::assign(l, o)) {
		point_on_Voronoi_edge = l.point();
	}
	if (CGAL::assign(r, o)) {
		point_on_Voronoi_edge = r.source();
	}
	return point_on_Voronoi_edge;
}

std::string type_of_Voronoi_edge(const SDG2::Edge& edge, const SDG2& delaunay) {
	CGAL::Object o = delaunay.primal(edge);
	typename Gt::Segment_2 s;
	typename Gt::Line_2 l;
	typename Gt::Ray_2 r;
	CGAL::Parabola_segment_2<Gt> ps;

	if (CGAL::assign(s, o)) {
		return "Linear";
	}
	if (CGAL::assign(ps, o)) {
		return "Parabolic";
	}
	if (CGAL::assign(l, o)) {
		return "Line";
	}
	if (CGAL::assign(r, o)) {
		return "Ray";
	}
}

std::string type_of_site(const SDG2::Site_2& site) {
	if (site.is_point()) {
		return "Point";
	} else {
		return "Segment";
	}
}

Separator medial_axis_separator(const SDG2& delaunay, const PointToIsoline& isoline, const PointToPoint& prev, const PointToPoint& next) {
	std::unordered_map<Isoline<K>*, std::vector<SDG2::Edge>> edges;

	for (auto eit = delaunay.finite_edges_begin(); eit != delaunay.finite_edges_end(); ++eit) {
		SDG2::Edge edge = *eit;
		auto [p, q] = defining_sites(edge);
		SDG2::Point_2 p_point = point_of_site(p);
		SDG2::Point_2 q_point = point_of_site(q);

		auto p_iso = isoline.at(p_point);
		auto q_iso = isoline.at(q_point);

		if (p_iso != q_iso) {
			Gt::Point_2 n;
			if (next.contains(p_point)) {
				n = next.at(p_point);
			} else {
				n = p_point + (p_point - prev.at(p_point));
			}
			if (CGAL::right_turn(p_point, n, point_of_Voronoi_edge(edge, delaunay))) {
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
		typename Gt::Segment_2 s;
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

Matching matching(const SDG2& delaunay, const Separator& separator, const PointToPoint& p_prev,
                  const PointToPoint& p_next, const PointToIsoline& p_isoline) {
	std::unordered_map<Gt::Point_2, MatchedTo> matching;

	for (auto& [_, edges]: separator)
	for (auto edge : edges) {
		create_matching(delaunay, edge, matching, p_prev, p_next, p_isoline);
	}

	auto comparison_f = compare_along_isoline(p_prev, p_next);

	for (auto& [_, ms] : matching)
	for (auto& [_, mi] : ms)
	for (auto& [_, pts] : mi) {
		std::sort(pts.begin(), pts.end());//, comparison_f);
		pts.erase(std::unique(pts.begin(), pts.end()), pts.end());
	}

	return matching;
}

/// Assumes point is in the Voronoi cell of site.
CGAL::Orientation side(const SDG2::Site_2& site, const SDG2::Point_2& point, const PointToPoint& p_prev, const PointToPoint& p_next) {
	if (site.is_point()) {
		auto p = site.point();
		if (!p_next.contains(p) && !p_prev.contains(p)) {
			return CGAL::LEFT_TURN;
		}
		Gt::Point_2 prev;
		if (p_prev.contains(p)) {
			prev = p_prev.at(p);
		} else {
			prev = p + (p - p_next.at(p));
		}
		Gt::Point_2 next;
		if (p_next.contains(p)) {
			next = p_next.at(p);
		} else {
			next = p + (p - prev);
		}
		auto v1 = prev - p;
		auto v2 = next - p;
		auto l1 = Gt::Line_2(p, v1);
		auto l2 = Gt::Line_2(p, v2);

		Gt::Line_2 l3;
		auto orient = CGAL::orientation(prev, p, next);
		if (orient == CGAL::LEFT_TURN) {
			l3 = CGAL::bisector(l1, l2).opposite().perpendicular(p);
		} else if (orient == CGAL::RIGHT_TURN) {
			l3 = CGAL::bisector(l1, l2).perpendicular(p);
		} else {
			l3 = Gt::Line_2(prev, next);
		}
		return CGAL::enum_cast<CGAL::Orientation>(l3.oriented_side(point));
	} else {
		auto s = site.segment();
		return CGAL::orientation(s.source(), s.target(), point);
	}
}

std::vector<Gt::Point_2> project_snap(const SDG2& delaunay, const SDG2::Site_2& site, const SDG2::Edge& edge) {
	std::vector<Gt::Point_2> pts;
	if (site.is_point()) {
		pts.reserve(1);
		pts.push_back(site.point());
		return pts;
	}

	auto proj_seg = std::get<Segment<K>>(site_projection(delaunay, edge, site));
	Gt::Segment_2 seg = snap_endpoints(proj_seg, site.segment());

	if (seg.source() == seg.target()) {
		pts.reserve(1);
		pts.push_back(seg.source());
	} else {
		pts.reserve(2);
		pts.push_back(seg.source());
		pts.push_back(seg.target());
	}
	return pts;
}

void create_matching(const SDG2& delaunay, const SDG2::Edge& edge, Matching& matching, const PointToPoint& p_prev, const PointToPoint& p_next, const PointToIsoline& p_isoline) {
	auto [p, q] = defining_sites(edge);
	auto p_pts = project_snap(delaunay, p, edge);
	auto q_pts = project_snap(delaunay, q, edge);

	for (int i = 0; i < p_pts.size(); i++) {
		// Below fails on the ends of open isolines.
		auto sign_p = side(p, point_of_Voronoi_edge(edge, delaunay), p_prev, p_next);
		auto sign_q = side(q, point_of_Voronoi_edge(edge, delaunay), p_prev, p_next);

		if (sign_p == sign_q) {
			//				std::cout << "Strange: " << type_of_Voronoi_edge(edge, delaunay) << "  "
			//						  << type_of_site(p) << "  " << type_of_site(q) << std::endl;
		}

		auto match = [&](int pi, int qi) {
			auto pp = p_pts[pi];
			auto qp = q_pts[qi];
			matching[pp][sign_p][p_isoline.at(point_of_site(q))].push_back(qp);
			matching[qp][sign_q][p_isoline.at(point_of_site(p))].push_back(pp);
		};

		if (i < q_pts.size()) {
			match(i, i);
		} else {
			match(i, i - 1);
		}
		if (q_pts.size() > p_pts.size()) {
			match(i, i + 1);
		}
	}
}

std::function<bool(const Gt::Point_2&, const Gt::Point_2&)> compare_along_isoline(const PointToPoint& p_prev, const PointToPoint& p_next) {
	return [&p_prev, &p_next](const Gt::Point_2& p, const Gt::Point_2& q) {
		if (p == q) return false;
		std::optional<Gt::Point_2> earlier;
		if (p_prev.contains(p)) {
			earlier = p_prev.at(p);
		}
		std::optional<Gt::Point_2> later;
		if (p_next.contains(p)) {
			later = p_next.at(p);
		}

		while (true) {
			Gt::Point_2 q_pt = q;
			if (earlier == q_pt)
				return false;
			if (later == q_pt)
				return true;
			if (earlier.has_value() && p_prev.contains(*earlier)) {
				earlier = p_prev.at(*earlier);
			}
			if (later.has_value() && p_next.contains(*later)) {
				later = p_next.at(*later);
			}
		}
	};
}
}