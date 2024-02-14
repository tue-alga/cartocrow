//
// Created by steven on 1/23/24.
//

#ifndef CARTOCROW_COLLAPSE_H
#define CARTOCROW_COLLAPSE_H

#include "types.h"

namespace cartocrow::isoline_simplification {
class SlopeLadder {
  public:
	SlopeLadder() = default;
	std::deque<Gt::Segment_2> m_rungs;
	std::unordered_map<CGAL::Sign, Gt::Point_2> m_cap;
	std::vector<Gt::Point_2> m_collapsed;
	double m_cost = 0.0;
	bool m_valid = true;
	bool m_old = false;
	bool m_intersects = false;
	void compute_cost(const PointToPoint& p_prev, const PointToPoint& p_next);
};

typedef std::function<void(SlopeLadder& ladder, const PointToPoint& p_prev, const PointToPoint& p_next)> LadderCollapse;
typedef std::function<Gt::Point_2(Gt::Point_2& s, Gt::Point_2& t, Gt::Point_2& u, Gt::Point_2& v)> RungCollapse;

Gt::Point_2 min_sym_diff_point(Gt::Point_2 s, Gt::Point_2 t, Gt::Point_2 u, Gt::Point_2 v);
Gt::Point_2 projected_midpoint(Gt::Point_2 s, Gt::Point_2 t, Gt::Point_2 u, Gt::Point_2 v);

LadderCollapse spline_collapse(const RungCollapse& rung_collapse);
void midpoint_collapse(SlopeLadder& ladder, const PointToPoint& p_prev, const PointToPoint& p_next);
void min_sym_diff_collapse(SlopeLadder& ladder, const PointToPoint& p_prev, const PointToPoint& p_next);

typedef std::unordered_map<Gt::Point_2, std::vector<std::shared_ptr<SlopeLadder>>> PointToSlopeLadders;
typedef std::unordered_map<Gt::Segment_2, std::vector<std::shared_ptr<SlopeLadder>>> EdgeToSlopeLadders;

Gt::Line_2 area_preservation_line(Gt::Point_2 s, Gt::Point_2 t, Gt::Point_2 u, Gt::Point_2 v);
double symmetric_difference(const Gt::Point_2& s, const Gt::Point_2& t, const Gt::Point_2& u, const Gt::Point_2& v, const Gt::Point_2& p);
double signed_area(const std::vector<Gt::Point_2>& pts);
double area(const std::vector<Gt::Point_2>& pts);
}
#endif //CARTOCROW_COLLAPSE_H
