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

Matching matching(const SDG2& delaunay, const Separator& separator, const PointToPoint& p_prev, const PointToPoint& p_next,
                  const PointToIsoline& p_isoline, const PointToIndex& p_index, bool do_snap) {
	std::unordered_map<Gt::Point_2, MatchedTo> matching;

	// Assumes point is in the Voronoi cell of site.
	auto side = [&p_next, &p_prev](const SDG2::Site_2& site, const SDG2::Point_2& point) {
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
	};

	auto project_snap = [&delaunay, &do_snap](const SDG2::Site_2& site, const SDG2::Edge& edge) {
	  	std::vector<Gt::Point_2> pts;
		if (site.is_point()) {
			pts.reserve(1);
			pts.push_back(site.point());
			return pts;
		}

		Gt::Segment_2 seg;
		auto proj_seg = std::get<Segment<K>>(site_projection(delaunay, edge, site));
		if (do_snap) {
			seg = snap_endpoints(proj_seg, site.segment());
		} else {
			seg = proj_seg;
		}

		if (seg.source() == seg.target()) {
			pts.reserve(1);
			pts.push_back(seg.source());
		} else {
			pts.reserve(2);
			pts.push_back(seg.source());
			pts.push_back(seg.target());
		}
		return pts;
	};

	for (auto& [_, edges]: separator)
	for (auto edge : edges) {
		auto [p, q] = defining_sites(edge);
		auto p_pts = project_snap(p, edge);
		auto q_pts = project_snap(q, edge);

		for (int i = 0; i < p_pts.size(); i++) {
			// Below fails on the ends of open isolines.
			auto sign_p = side(p, point_of_Voronoi_edge(edge, delaunay));
			auto sign_q = side(q, point_of_Voronoi_edge(edge, delaunay));

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

	for (auto& [_, ms] : matching)
	for (auto& [_, mi] : ms)
	for (auto& [_, pts] : mi) {
		if (do_snap) {
			std::sort(pts.begin(), pts.end(), [&p_index](Gt::Point_2& p, Gt::Point_2& q) {
				return p_index.at(p) < p_index.at(q);
			});
			pts.erase(std::unique(pts.begin(), pts.end()), pts.end());
		}
	}

	return matching;
}

std::vector<SlopeLadder> slope_ladders(const Matching& matching,
                                       const std::vector<Isoline<K>>& isolines,
                                       const PointToPoint& p_next) {
	std::vector<SlopeLadder> slope_ladders;
	EdgeToSlopeLadder edge_ladder;

	for (const auto& isoline : isolines) {
		auto polyline = isoline.polyline();
		for (auto eit = polyline.edges_begin(); eit != polyline.edges_end(); ++eit) {
			Gt::Segment_2 seg = *eit;
			if (edge_ladder.contains(seg) || edge_ladder.contains(seg.opposite())) continue;

			Gt::Point_2 s = seg.source();
			Gt::Point_2 t = seg.target();

			const auto search = [&](const Gt::Point_2& s, const Gt::Point_2& t, CGAL::Sign initial_dir, CGAL::Sign dir, SlopeLadder& slope_ladder, const auto& search_f) {
				if (!(matching.contains(s) && matching.contains(t))) return;
				auto& s_matching = matching.at(s);
				auto& t_matching = matching.at(t);

				if (!(s_matching.contains(dir) && t_matching.contains(dir))) return;
				auto& s_m = s_matching.at(dir);
			  	auto& t_m = t_matching.at(dir);

				Isoline<K>* shared_isoline;
				for (const auto& [isoline_s_m, _] : s_m) {
					for (const auto& [isoline_t_m, _] : t_m) {
						if (isoline_s_m == isoline_t_m) {
							shared_isoline = isoline_s_m;
						}
					}
				}
				if (shared_isoline == nullptr) return;
			  	auto& sms = s_m.at(shared_isoline);
			  	auto& tms = t_m.at(shared_isoline);

				auto make_rung = [&](const Gt::Point_2& a, const Gt::Point_2& b) {
					edge_ladder[Segment<K>(a, b)] = &slope_ladder;
					if (initial_dir == CGAL::LEFT_TURN) {
						slope_ladder.rungs.emplace_front(a, b);
					} else {
						slope_ladder.rungs.emplace_back(a, b);
					}
					// Continue search in direction opposite of where we came from
					CGAL::Sign new_dir;
					bool found = false;
					for (CGAL::Sign possible_dir : {CGAL::LEFT_TURN, CGAL::RIGHT_TURN}) {
						if (matching.at(a).contains(possible_dir))
						for (const auto& [_, pts] : matching.at(a).at(possible_dir))
						for (const auto& pt : pts)
						if (pt == s) {
							new_dir = -possible_dir;
							found = true;
						}
					}
					assert(found);
					search_f(a, b, initial_dir, new_dir, slope_ladder, search_f);
				};

				bool cap_ = sms.back() == tms.front();
			  	bool cap_r = sms.front() == tms.back();
			  	bool rung_ = p_next.contains(sms.back()) && p_next.at(sms.back()) == tms.front();
				bool rung_r = p_next.contains(tms.back()) && p_next.at(tms.back()) == sms.front();
				if (cap_) {
					slope_ladder.cap[initial_dir] = sms.back();
				} else if (cap_r) {
					slope_ladder.cap[initial_dir] = sms.front();
				} else if (rung_) {
					make_rung(sms.back(), tms.front());
				} else if (rung_r) {
					make_rung(sms.front(), tms.back());
				}
				return;
			};

			SlopeLadder& slope_ladder = slope_ladders.emplace_back();
			slope_ladder.rungs.emplace_back(s, t);
			edge_ladder[seg] = &slope_ladder;

			search(s, t, CGAL::LEFT_TURN, CGAL::LEFT_TURN, slope_ladder, search);
			search(s, t, CGAL::RIGHT_TURN, CGAL::RIGHT_TURN, slope_ladder, search);
		}
	}

	return slope_ladders;
}
}