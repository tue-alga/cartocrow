//
// Created by steven on 1/23/24.
//

#include "collapse.h"

#include <utility>
#include "ipeshape.h"
#include "ipegeo.h"
#include "ipe_bezier_wrapper.h"

namespace cartocrow::isoline_simplification {
void midpoint_collapse(SlopeLadder& ladder, const PointToPoint& p_prev, const PointToPoint& p_next) {
	ladder.m_collapsed.clear();
	if (!ladder.m_valid) return;
	for (const auto& rung : ladder.m_rungs) {
		auto reversed = p_next.contains(rung.target()) && p_next.at(rung.target()) == rung.source();
		auto t = reversed ? rung.target() : rung.source();
		auto u = reversed ? rung.source() : rung.target();
		Gt::Point_2 s = p_prev.at(t);
		Gt::Point_2 v = p_next.at(u);

		ladder.m_collapsed.push_back(projected_midpoint(s, t, u, v));
	}
}

LadderCollapse spline_collapse(const RungCollapse& rung_collapse) {
	return [rung_collapse](SlopeLadder& ladder, const PointToPoint& p_prev, const PointToPoint& p_next) {
		ladder.m_collapsed.clear();
		if (!ladder.m_valid)
			return;

		std::vector<ipe::Vector> control_points;
		if (ladder.m_cap.contains(CGAL::LEFT_TURN)) {
			control_points.push_back(pv(ladder.m_cap.at(CGAL::LEFT_TURN)));
		}
		for (const auto& rung : ladder.m_rungs) {
			auto reversed = p_next.contains(rung.target()) && p_next.at(rung.target()) == rung.source();
			auto t = reversed ? rung.target() : rung.source();
			auto u = reversed ? rung.source() : rung.target();
			Gt::Point_2 s = p_prev.at(t);
			Gt::Point_2 v = p_next.at(u);
			control_points.push_back(pv(rung_collapse(s, t, u, v)));
		}
		if (ladder.m_cap.contains(CGAL::RIGHT_TURN)) {
			control_points.push_back(pv(ladder.m_cap.at(CGAL::RIGHT_TURN)));
		}

		ipe::Curve curve;
		curve.appendSpline(control_points);
		if (curve.countSegments() > 1) {
			throw std::runtime_error("Expected only one segment in spline.");
		}

		std::vector<ipe::Bezier> bzs;
		auto curved_segment = curve.segment(0);
		curved_segment.beziers(bzs);

		auto intersection = [&bzs](Gt::Line_2 l) {
			ipe::Line line = ipe::Line::through(pv(l.point(0)), pv(l.point(1)));
			std::vector<ipe::Vector> inters;
			for (auto& b : bzs) {
				b.intersect(line, inters);
			}
			if (inters.size() != 1) {
				//			std::cerr << "Expected one spline--line intersection but encountered: " << inters.size() << std::endl;
				return std::optional<Gt::Point_2>();
			}
			return std::optional(vp(inters.front()));
		};

		for (int i = 0; i < ladder.m_rungs.size(); i++) {
			const auto& rung = ladder.m_rungs[i];
			auto reversed = p_next.contains(rung.target()) && p_next.at(rung.target()) == rung.source();
			auto t = reversed ? rung.target() : rung.source();
			auto u = reversed ? rung.source() : rung.target();
			Gt::Point_2 s = p_prev.at(t);
			Gt::Point_2 v = p_next.at(u);
			Gt::Line_2 l = area_preservation_line(s, t, u, v);
			Gt::Point_2 new_vertex;
			if (i == 0 && !ladder.m_cap.contains(CGAL::LEFT_TURN) ||
			    i == ladder.m_rungs.size() - 1 && !ladder.m_cap.contains(CGAL::RIGHT_TURN)) {
				//		if (control_points.size() == 1) {
//				new_vertex = l.projection(midpoint(rung));
				new_vertex = rung_collapse(s, t, u, v);
			} else {
				auto inter = intersection(l);
				new_vertex = inter.has_value() ? *inter : rung_collapse(s, t, u, v);
			}
			ladder.m_collapsed.push_back(new_vertex);
		}
	};
}

Gt::Point_2 min_sym_diff_point(Gt::Point_2 s, Gt::Point_2 t, Gt::Point_2 u, Gt::Point_2 v) {
	Gt::Line_2 l = area_preservation_line(s, t, u, v);
	Gt::Line_2 svl(s, v);
	Gt::Line_2 stl(s, t);
	Gt::Line_2 uvl(u, v);
	Gt::Point_2 new_vertex;
	// todo: handle degenerate cases
	// todo: seems that new_vertex may be equal to s or v, causing issues
	if (svl.oriented_side(t) == svl.oriented_side(u)) {
		if (squared_distance(svl, t) > squared_distance(svl, u)) {
			auto i = *intersection(l, stl);
			new_vertex = *boost::get<Gt::Point_2>(&i);
		} else {
			auto i = *intersection(l, uvl);
			new_vertex = *boost::get<Gt::Point_2>(&i);
		}
	} else {
		if (svl.oriented_side(t) == svl.oriented_side(l.point())) {
			auto i = *intersection(l, stl);
			new_vertex = *boost::get<Gt::Point_2>(&i);
		} else {
			auto i = *intersection(l, uvl);
			new_vertex = *boost::get<Gt::Point_2>(&i);
		}
	}
	// If nearly collinear use midpoint
	auto dist_threshold = 0.0001 * squared_distance(s, v);
	if (squared_distance(new_vertex, s) < dist_threshold || squared_distance(new_vertex, v) < dist_threshold) {
		new_vertex = l.projection(midpoint(s, v));
	}
	return new_vertex;
}

Gt::Point_2 projected_midpoint(Gt::Point_2 s, Gt::Point_2 t, Gt::Point_2 u, Gt::Point_2 v) {
	Gt::Line_2 l = area_preservation_line(s, t, u, v);
	return l.projection(midpoint(t, u));
}

void min_sym_diff_collapse(SlopeLadder& ladder, const PointToPoint& p_prev, const PointToPoint& p_next) {
	ladder.m_collapsed.clear();
	if (!ladder.m_valid) return;
	for (const auto& rung : ladder.m_rungs) {
		auto reversed = p_next.contains(rung.target()) && p_next.at(rung.target()) == rung.source();
		auto t = reversed ? rung.target() : rung.source();
		auto u = reversed ? rung.source() : rung.target();
		Gt::Point_2 s;
		if (p_prev.contains(t)) {
			s = p_prev.at(t);
		} else {
			return;
		}
		Gt::Point_2 v;
		if (p_next.contains(u)) {
			v = p_next.at(u);
		} else {
			return;
		}

		ladder.m_collapsed.push_back(min_sym_diff_point(s, t, u, v));
	}
}

bool point_order_on_line(Gt::Line_2 l, Gt::Point_2 a, Gt::Point_2 b) {
	auto dir_line = l.to_vector();
	auto dir_pts = b - a;
	return dir_line * dir_pts > 0;
}

void harmony_line_collapse(SlopeLadder& ladder, const PointToPoint& p_prev, const PointToPoint& p_next, int n_samples = 500) {
	ladder.m_collapsed.clear();
	if (!ladder.m_valid) return;

	// Compute sample line
	Gt::Line_2 sample_line;
	Gt::Line_2 initial_harmony_line;
	if (ladder.m_rungs.size() == 1) {
		auto& rung = ladder.m_rungs.front();
		auto reversed = p_next.contains(rung.target()) && p_next.at(rung.target()) == rung.source();
		auto t = reversed ? rung.target() : rung.source();
		auto u = reversed ? rung.source() : rung.target();
		Gt::Point_2 s = p_prev.at(t);
		Gt::Point_2 v = p_next.at(u);
		sample_line = area_preservation_line(s, t, u, v);
		initial_harmony_line = sample_line.perpendicular(midpoint(t, u));
	} else {
		auto& first_rung = ladder.m_rungs.front();
		auto& last_rung = ladder.m_rungs.back();
		initial_harmony_line = Gt::Line_2(midpoint(first_rung), midpoint(last_rung));
		sample_line =
		    initial_harmony_line.perpendicular(midpoint(midpoint(first_rung), midpoint(last_rung)));
	}

	std::optional<Gt::Point_2> first;
	std::optional<Gt::Point_2> last;
	for (const auto& rung : ladder.m_rungs) {
		auto reversed = p_next.contains(rung.target()) && p_next.at(rung.target()) == rung.source();
		auto t = reversed ? rung.target() : rung.source();
		auto u = reversed ? rung.source() : rung.target();
		Gt::Point_2 s = p_prev.at(t);
		Gt::Point_2 v = p_next.at(u);
		auto area_l = area_preservation_line(s, t, u, v);
		for (auto& p : {s, t, u, v}) {
			auto pt = sample_line.projection(area_l.projection(p));
			if (!first.has_value() || point_order_on_line(area_l, pt, *first)) {
				first = pt;
			}
			if (!last.has_value() || point_order_on_line(area_l, *last, pt)) {
				last = pt;
			}
		}
	}

	K::Vector_2 step_v = (*last - *first) / (n_samples - 1);

	double best_score = std::numeric_limits<double>::infinity();
	Gt::Line_2 best_line;

	for (int i = 0; i < n_samples; i++) {
		K::Vector_2 offset = i * step_v;
		Gt::Point_2 pt = *first + offset;
		Gt::Line_2 harmony_line(pt, initial_harmony_line.direction());

		for (const auto& rung : ladder.m_rungs) {
			auto reversed = p_next.contains(rung.target()) && p_next.at(rung.target()) == rung.source();
			auto t = reversed ? rung.target() : rung.source();
			auto u = reversed ? rung.source() : rung.target();
			Gt::Point_2 s = p_prev.at(t);
			Gt::Point_2 v = p_next.at(u);

			Gt::Line_2 l = area_preservation_line(s, t, u, v);
			auto inter = *intersection(harmony_line, l);
			Gt::Point_2 new_vertex = *boost::get<Gt::Point_2>(&inter);
		}
	}

	for (const auto& rung : ladder.m_rungs) {
		auto reversed = p_next.contains(rung.target()) && p_next.at(rung.target()) == rung.source();
		auto t = reversed ? rung.target() : rung.source();
		auto u = reversed ? rung.source() : rung.target();
		Gt::Point_2 s = p_prev.at(t);
		Gt::Point_2 v = p_next.at(u);


		Gt::Line_2 l = area_preservation_line(s, t, u, v);
		Gt::Point_2 new_vertex = l.projection(midpoint(rung));
		ladder.m_collapsed.push_back(new_vertex);
	}
}

double symmetric_difference(const Gt::Point_2& s, const Gt::Point_2& t, const Gt::Point_2& u, const Gt::Point_2& v, const Gt::Point_2& p) {
	Gt::Segment_2 st = Gt::Segment_2(s, t);
	Gt::Segment_2 tu = Gt::Segment_2(t, u);
	Gt::Segment_2 uv = Gt::Segment_2(u, v);

	Gt::Segment_2 sp = Gt::Segment_2(s, p);
	Gt::Segment_2 pv = Gt::Segment_2(p, v);

	auto st_pv = intersection(st, pv);
	auto tu_pv = intersection(tu, pv);
	auto tu_sp = intersection(tu, sp);
	auto uv_sp = intersection(uv, sp);

	double cost = 0.0;

	if (st_pv.has_value()) {
		if (auto st_pv_pt = boost::get<Gt::Point_2>(&*st_pv)) {
			cost += area({s, p, *st_pv_pt});
		}
		if (tu_pv.has_value()) {
			if (auto tu_pv_pt = boost::get<Gt::Point_2>(&*tu_pv)) {
				cost += area({*tu_pv_pt, u, v});
				if (auto st_pv_pt = boost::get<Gt::Point_2>(&*st_pv)) {
					cost += area({*st_pv_pt, t, *tu_pv_pt});
				}
			}
		} else {
			if (auto st_pv_pt = boost::get<Gt::Point_2>(&*st_pv)) {
				cost += area({*st_pv_pt, t, u, v});
			}
		}
	} else if (uv_sp.has_value()) {
		if (auto uv_sp_pt = boost::get<Gt::Point_2>(&*uv_sp)) {
			cost += area({*uv_sp_pt, p, v});
		}
		if (tu_sp.has_value()) {
			if (auto tu_sp_pt = boost::get<Gt::Point_2>(&*tu_sp)) {
				cost += area({s, t, *tu_sp_pt});
				if (auto uv_sp_pt = boost::get<Gt::Point_2>(&*uv_sp)) {
					cost += area({*tu_sp_pt, u, *uv_sp_pt});
				}
			}
		} else {
			if (auto uv_sp_pt = boost::get<Gt::Point_2>(&*uv_sp)) {
				cost += area({s, t, u, *uv_sp_pt});
			}
		}
	} else if (!tu_sp.has_value() && !tu_pv.has_value()) {
		cost += area({s, t, u, v, p});
	} else if (!tu_sp.has_value()) {
		if (auto tu_pv_pt = boost::get<Gt::Point_2>(&*tu_pv)) {
			cost += area({p, s, t, *tu_pv_pt});
			cost += area({*tu_pv_pt, u, v});
		}
	} else if (!tu_pv.has_value()) {
		if (auto tu_sp_pt = boost::get<Gt::Point_2>(&*tu_sp)) {
			cost += area({s, t, *tu_sp_pt});
			cost += area({*tu_sp_pt, u, v, p});
		}
	} else {
		if (auto tu_sp_pt = boost::get<Gt::Point_2>(&*tu_sp)) {
			cost += area({s, t, *tu_sp_pt});
		}
		if (auto tu_pv_pt = boost::get<Gt::Point_2>(&*tu_pv)) {
			cost += area({*tu_pv_pt, u, v});
		}
		if (auto tu_sp_pt = boost::get<Gt::Point_2>(&*tu_sp)) {
			if (auto tu_pv_pt = boost::get<Gt::Point_2>(&*tu_pv)) {
				cost += area({*tu_sp_pt, p, *tu_pv_pt});
			}
		}
	}

	return cost;
}

void SlopeLadder::compute_cost(const PointToPoint& p_prev, const PointToPoint& p_next) {
	if (!m_valid) {
		m_cost = std::numeric_limits<double>::infinity();
		return;
	}

	m_cost = 0;

	for (int i = 0; i < m_rungs.size(); ++i) {
		const auto& rung = m_rungs[i];
		auto t = rung.source();
		auto u = rung.target();
		Gt::Point_2 s = p_prev.at(t);
		Gt::Point_2 v = p_next.at(u);
		m_cost += symmetric_difference(s, t, u, v, m_collapsed[i]);
	}

	m_cost /= m_rungs.size();
}

double signed_area(const std::vector<Gt::Point_2>& pts) {
	double total = 0.0;
	auto prev = pts.back();
	for (const Gt::Point_2& curr : pts) {
		total += prev.x() * curr.y() - prev.y() * curr.x();
		prev = curr;
	}
	return total / 2.0;
}

double area(const std::vector<Gt::Point_2>& pts) {
	return abs(signed_area(pts));
}

Gt::Line_2 area_preservation_line(Gt::Point_2 s, Gt::Point_2 t, Gt::Point_2 u, Gt::Point_2 v) {
	double area = signed_area({s, v, u, t});
	if (s == v) {
		// This function should not be called in this case.
		std::cerr << "s: " << s << std::endl;
		std::cerr << "t: " << t << std::endl;
		std::cerr << "u: " << u << std::endl;
		std::cerr << "v: " << v << std::endl;
		throw std::runtime_error("Cannot simplify an isoline of three vertices");
	}
	// From the paper:
	// Simplification of polylines by segment collapse: minimizing areal displacement while preserving area
	// Barry J. Kronenfeld, Lawrence V. Stanislawski, Barbara P. Buttenfield & Tyler Brockmeyer
	auto a = v.y() - s.y();
	auto b = s.x() - v.x();
	auto c = -t.y() * s.x() + (s.y() - u.y()) * t.x() + (t.y() - v.y()) * u.x() + u.y() * v.x();
	return Gt::Line_2(a, b, c);
}
}