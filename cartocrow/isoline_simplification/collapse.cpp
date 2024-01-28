//
// Created by steven on 1/23/24.
//

#include "collapse.h"

namespace cartocrow::isoline_simplification {
void SlopeLadder::compute_collapsed(const PointToPoint& p_prev, const PointToPoint& p_next) {
	for (const auto& rung : m_rungs) {
		auto t = rung.source();
		auto u = rung.target();
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

		Gt::Line_2 l = area_preservation_line(s, t, u, v);
		Gt::Point_2 new_vertex = l.projection(midpoint(rung));
		m_collapsed.push_back(new_vertex);
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
		throw std::runtime_error("Cannot simplify an isoline of three vertices");
	}
	double base_length = sqrt(squared_distance(s, v));
	double height = 2 * area / base_length;
	Gt::Line_2 base(s, v);
	// TODO: This assumes that the orientation of closed isolines is clockwise
	auto vec = base.direction().perpendicular(CGAL::LEFT_TURN).vector();
	vec /= sqrt(vec.squared_length());
	vec *= height;
	CGAL::Aff_transformation_2<K> translate(CGAL::TRANSLATION, vec);
	return base.transform(translate);
}
}