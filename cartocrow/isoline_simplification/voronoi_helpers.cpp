/*
The CartoCrow library implements algorithmic geo-visualization methods,
developed at TU Eindhoven.
Copyright (C) 2024 TU Eindhoven

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "types.h"
#include "voronoi_helpers.h"
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

Point<K> point_of_Voronoi_edge(const SDG2::Edge& edge, const SDG2& delaunay) {
	CGAL::Object o = delaunay.primal(edge);
	Segment<K> s;
	Line<K> l;
	Ray<K> r;
	CGAL::Parabola_segment_2<Gt> ps;

	Point<K> point_on_Voronoi_edge;
	if (CGAL::assign(s, o)) {
		point_on_Voronoi_edge = midpoint(s);
	}
	if (CGAL::assign(ps, o)) {
		// Roundabout way to obtain start and end of parabolic segment because they are protected -_-
		std::vector<Point<K>> pts;
		// small-western-island results in a NaN value somehow...
		Open_Parabola_segment_2 ops{ps};
		if (!std::isnan(ops.get_p1().x())) {
			point_on_Voronoi_edge = ops.get_p1();
		} else if (!std::isnan(ops.get_p2().x())) {
			point_on_Voronoi_edge = ops.get_p2();
		} else {
			throw std::runtime_error("Both endpoints of parabolic segment are NaN!");
		}
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
	Segment<K> s;
	Line<K> l;
	Ray<K> r;
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
	return "Unknown";
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
			Point<K> n;
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

std::variant<Point<K>, Segment<K>> site_projection(const SDG2& delaunay, const SDG2::Edge& edge, const SDG2::Site_2& site) {
	if (site.is_point()) {
		return { site.point() };
	} else {
		Segment<K> s;
		CGAL::Parabola_segment_2<Gt> ps;
		CGAL::Object o = delaunay.primal(edge);

		// Ray and line cases cannot occur because they require both sites to be a point
		if (CGAL::assign(s, o)) {
			auto start = site.segment().supporting_line().projection(s.source());
			auto end = site.segment().supporting_line().projection(s.end());
			return { Segment<Inexact>(start, end) };
		}
		else if (CGAL::assign(ps, o)) {
			// Roundabout way to obtain start and end of parabolic segment because they are protected -_-
			Open_Parabola_segment_2 ops{ps};
			auto p1 = ops.get_p1();
			auto p2 = ops.get_p2();

			if (std::isnan(p1.x()) || std::isnan(p2.x())) {
				return {Segment<K>(Point<K>(0.0, 0.0), Point<K>(1.0, 0.0))};
			}

			auto start = site.segment().supporting_line().projection(p1);
			auto end = site.segment().supporting_line().projection(p2);

			return {Segment<Inexact>(start, end)};
		}
		else {
			throw std::runtime_error("Impossible: a segment Voronoi edge is neither a line segment nor a parabolic "
			                         "segment, but at least one of its sites is a line segment.");
		}
	}
}

Segment<K> snap_endpoints(Segment<K> proj, Segment<K> original) {
	Point<K> start;
	if (compare_distance_to_point(proj.source(), original.source(), original.target()) == CGAL::SMALLER) {
		start = original.source();
	} else {
		start = original.target();
	}
	Point<K> end;
	if (compare_distance_to_point(proj.target(), original.source(), original.target()) == CGAL::SMALLER) {
		end = original.source();
	} else {
		end = original.target();
	}
	return { start, end };
}

Matching matching(const SDG2& delaunay, const Separator& separator, const PointToPoint& p_prev,
                  const PointToPoint& p_next, const PointToIsoline& p_isoline, const PointToVertex& p_vertex,
                  const double angle_filter, const double alignment_filter) {
	std::unordered_map<Point<K>, MatchedTo> matching;

	for (auto& [_, edges]: separator)
	for (auto edge : edges) {
		create_matching(delaunay, edge, matching, p_prev, p_next, p_isoline, p_vertex, angle_filter, alignment_filter);
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

Line<K> supporting_line(const SDG2::Point_2& p, const PointToPoint& p_prev, const PointToPoint& p_next) {
	if (!p_next.contains(p) && !p_prev.contains(p)) {
		throw std::runtime_error("Point should have next or prev.");
	}
	Point<K> prev;
	if (p_prev.contains(p)) {
		prev = p_prev.at(p);
	} else {
		prev = p + (p - p_next.at(p));
	}
	Point<K> next;
	if (p_next.contains(p)) {
		next = p_next.at(p);
	} else {
		next = p + (p - prev);
	}
	auto v1 = prev - p;
	auto v2 = next - p;
	auto l1 = Line<K>(p, v1);
	auto l2 = Line<K>(p, v2);

	Line<K> l3;
	auto orient = CGAL::orientation(prev, p, next);
	if (orient == CGAL::LEFT_TURN) {
		l3 = CGAL::bisector(l1, l2).opposite().perpendicular(p);
	} else if (orient == CGAL::RIGHT_TURN) {
		l3 = CGAL::bisector(l1, l2).perpendicular(p);
	} else {
		l3 = Line<K>(prev, next);
	}
	return l3;
}

Line<K> supporting_line(const SDG2::Site_2& site, const PointToPoint& p_prev, const PointToPoint& p_next) {
	if (site.is_point()) {
		return supporting_line(site.point(), p_prev, p_next);
	} else {
		return site.segment().supporting_line();
	}
}

CGAL::Orientation side(const SDG2::Point_2& p, const SDG2::Point_2& point, const PointToPoint& p_prev, const PointToPoint& p_next) {
	if (!p_next.contains(p) && !p_prev.contains(p)) {
		return CGAL::LEFT_TURN;
	}
	auto l = supporting_line(p, p_prev, p_next);
	return CGAL::enum_cast<CGAL::Orientation>(l.oriented_side(point));
}

/// Assumes point is in the Voronoi cell of site.
CGAL::Orientation side(const SDG2::Site_2& site, const SDG2::Point_2& point, const PointToPoint& p_prev, const PointToPoint& p_next) {
	if (site.is_point()) {
		return side(site.point(), point, p_prev, p_next);
	} else {
		auto s = site.segment();
		return CGAL::orientation(s.source(), s.target(), point);
	}
}

std::vector<Point<K>> project_snap(const SDG2& delaunay, const SDG2::Site_2& site, const SDG2::Edge& edge) {
	std::vector<Point<K>> pts;
	if (site.is_point()) {
		pts.reserve(1);
		pts.push_back(site.point());
		return pts;
	}

	auto proj_seg = std::get<Segment<K>>(site_projection(delaunay, edge, site));
	Segment<K> seg = snap_endpoints(proj_seg, site.segment());

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

std::optional<Segment<K>> check_segment_intersections_Voronoi(const SDG2& delaunay, const Segment<K> seg,
                                                                 const SDG2::Vertex_handle endpoint_handle,
                                                                 const std::unordered_set<SDG2::Vertex_handle>& allowed = std::unordered_set<SDG2::Vertex_handle>(),
                                                                 const std::optional<SDG2::Vertex_handle> collinear_vertex = std::nullopt) {
	auto t = SDG2::Site_2::construct_site_2(seg.source(), seg.target());

	auto check_intersections = [&t, &delaunay](SDG2::Vertex_handle vv) {
		if (!delaunay.is_infinite(vv) && vv->is_segment()) {
			bool intersects = arrangement_type(delaunay, t, vv->site()) == Gt::Arrangement_type_2::result_type::CROSSING;
			if (intersects) {
				return true;
			}
		}
		return false;
	};

	auto c_incircle = [&collinear_vertex](const SDG2& sdg, const SDG2::Face_handle& f, const SDG2::Site_2& q){
		if (collinear_vertex.has_value()) {
			for (int i = 0; i < 3; i++) {
				if (f->vertex(i) == collinear_vertex) {
					return CGAL::NEGATIVE;
				}
			}
		}
		return incircle(sdg, f, q);
	};

	auto vc_start = delaunay.incident_vertices(endpoint_handle);
	auto vc = vc_start;
	do {
		SDG2::Vertex_handle vv(vc);
		if (delaunay.is_infinite(vv)) {
			++vc;
			continue;
		}

		if (check_intersections(vv) && !allowed.contains(vv)) return vv->site().segment();
		++vc;
	} while (vc != vc_start);

	// First, find one face that is in conflict with seg (i.e. seg is close to corresponding vertex of Voronoi diagram)
	SDG2::Face_circulator fc_start = delaunay.incident_faces(endpoint_handle);
	SDG2::Face_circulator fc = fc_start;
	SDG2::Face_handle start_f;
	CGAL::Sign s;

	do {
		SDG2::Face_handle f(fc);
		s = c_incircle(delaunay, f, t);

		if (s != CGAL::POSITIVE) {
			start_f = f;
			break;
		}
		++fc;
	} while (fc != fc_start);

	assert(s != CGAL::POSITIVE);

	std::unordered_set<SDG2::Face_handle> visited;
	std::stack<SDG2::Face_handle> face_stack;
	std::unordered_set<SDG2::Face_handle> positive;
	face_stack.push(start_f);

	while (!face_stack.empty()) {
		const auto curr_f = face_stack.top();
		face_stack.pop();

		// Already visited, so skip
		if (visited.contains(curr_f)) {
			continue;
		}
		visited.insert(curr_f);

		for (int i = 0; i < 3; i++) {
			auto n = curr_f->neighbor(i);
			if (visited.contains(n)) continue;

			for (int j = 0; j < 3; j++) {
				auto vv = n->vertex(j);
				if (check_intersections(vv)) {
					if (!allowed.contains(vv))
						return vv->site().segment();
					else {
						auto vfc_start = delaunay.incident_faces(vv);
						auto vfc = vfc_start;
						do {
							SDG2::Face_handle f(vfc);
							face_stack.push(f);
							++vfc;
						} while (vfc != vfc_start);
					}
				}
			}

			s = c_incircle(delaunay, n, t);

			if (positive.contains(curr_f) && s == CGAL::POSITIVE) continue;

			face_stack.push(n);
			if (s == CGAL::POSITIVE) {
				positive.insert(n);
			}
		}
	}

	// If we are done and haven't found intersections then there are none.
	return std::nullopt;
}

void create_matching(const SDG2& delaunay, const SDG2::Edge& edge, Matching& matching, const PointToPoint& p_prev,
                     const PointToPoint& p_next, const PointToIsoline& p_isoline, const PointToVertex& p_vertex,
                     const double angle_filter, const double alignment_filter) {
	SDG2::Site_2 p, q;
	std::tie(p, q) = defining_sites(edge);

	auto pl = supporting_line(p, p_prev, p_next);
	auto ql = supporting_line(q, p_prev, p_next);

	auto pv = pl.to_vector();
	auto qv = ql.to_vector();
	auto angle = acos((pv * qv) / (sqrt(pv.squared_length()) * sqrt(qv.squared_length())));
	if (angle > M_PI/2) {
		angle = M_PI - angle;
	}
	if (angle > angle_filter) {
		return;
	}

	auto p_pts = project_snap(delaunay, p, edge);
	auto q_pts = project_snap(delaunay, q, edge);

	for (int i = 0; i < p_pts.size(); i++) {
		// Below fails on the ends of open isolines.
		auto sign_p = side(p, point_of_Voronoi_edge(edge, delaunay), p_prev, p_next);
		auto sign_q = side(q, point_of_Voronoi_edge(edge, delaunay), p_prev, p_next);

		auto match = [&](int pi, int qi) {
			auto pp = p_pts[pi];
			auto qp = q_pts[qi];
			bool edge_case = !p_prev.contains(pp) || !p_prev.contains(qp) || !p_next.contains(pp) || !p_next.contains(qp);
//			bool intersects = check_segment_intersections_Voronoi(delaunay, Segment<K>(pp, qp), p_vertex.at(pp)).has_value();
			bool aligned = vertex_alignment(p_prev, p_next, pp, qp, sign_p, sign_q) < alignment_filter;
			if (!edge_case && aligned) {
				matching[pp][sign_p][p_isoline.at(point_of_site(q))].push_back(qp);
				matching[qp][sign_q][p_isoline.at(point_of_site(p))].push_back(pp);
			}
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

std::function<bool(const Point<K>&, const Point<K>&)> compare_along_isoline(const PointToPoint& p_prev, const PointToPoint& p_next) {
	return [&p_prev, &p_next](const Point<K>& p, const Point<K>& q) {
		if (p == q) return false;
		std::optional<Point<K>> earlier;
		if (p_prev.contains(p)) {
			earlier = p_prev.at(p);
		}
		std::optional<Point<K>> later;
		if (p_next.contains(p)) {
			later = p_next.at(p);
		}

		while (true) {
			Point<K> q_pt = q;
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

K::Vector_2 normal(const SDG2::Point_2& p, const PointToPoint& p_prev, const PointToPoint& p_next, CGAL::Sign side) {
	auto l = supporting_line(p, p_prev, p_next);
	auto candidate = l.perpendicular(p);
	if (l.oriented_side(p + candidate.to_vector()) == side) {
		return candidate.to_vector();
	} else {
	    return candidate.opposite().to_vector();
	}
}

double vertex_alignment(const PointToPoint& p_prev, const PointToPoint& p_next, Point<K> u, Point<K> v, CGAL::Sign uv_side, CGAL::Sign vu_side) {
	auto n_u = normal(u, p_prev, p_next, uv_side);
	auto n_v = normal(v, p_prev, p_next, vu_side);
	auto uv = v - u;
	auto vu = u - v;
	auto uv_l = sqrt(uv.squared_length());
	auto angle_u = acos((n_u * uv) / (sqrt(n_u.squared_length()) * uv_l));
	auto angle_v = acos((n_v * vu) / (sqrt(n_v.squared_length()) * uv_l));
	return angle_u + angle_v;
}
}
