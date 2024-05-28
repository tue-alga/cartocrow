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

#ifndef CARTOCROW_COLLAPSE_H
#define CARTOCROW_COLLAPSE_H

#include "types.h"
#include "cartocrow/renderer/geometry_painting.h"
#include "ipeshape.h"
#include "ipegeo.h"

namespace cartocrow::isoline_simplification {
class SlopeLadder {
  public:
	SlopeLadder() = default;
	std::deque<Segment<K>> m_rungs;
	std::unordered_map<CGAL::Sign, Point<K>> m_cap;
	std::vector<Point<K>> m_collapsed;
	double m_cost = 0.0;
	bool m_valid = true;
	bool m_old = false;
	bool m_intersects = false;
	void compute_cost(const PointToPoint& p_prev, const PointToPoint& p_next);
};

class LadderCollapse {
  public:
	virtual void operator()(SlopeLadder& ladder, const PointToPoint& p_prev, const PointToPoint& p_next) = 0;
	virtual std::shared_ptr<renderer::GeometryPainting> painting(const SlopeLadder& ladder, const PointToPoint& p_prev, const PointToPoint& p_next) = 0;
};

//typedef std::function<void(SlopeLadder& ladder, const PointToPoint& p_prev, const PointToPoint& p_next)> LadderCollapse;
typedef std::function<Point<K>(Point<K>& s, Point<K>& t, Point<K>& u, Point<K>& v, Line<K>& l)> RungCollapse;

Point<K> min_sym_diff_point(Point<K> s, Point<K> t, Point<K> u, Point<K> v, Line<K> l);
Point<K> projected_midpoint(Point<K> s, Point<K> t, Point<K> u, Point<K> v, Line<K> l);

//LadderCollapse spline_collapse(const RungCollapse& rung_collapse, int repititions);
class SplineCollapse : public LadderCollapse {
  public:
	SplineCollapse(int repetitions, int samples = 50);
	void operator()(SlopeLadder& ladder, const PointToPoint& p_prev, const PointToPoint& p_next) override;
	std::shared_ptr<renderer::GeometryPainting> painting(const SlopeLadder& ladder, const PointToPoint& p_prev, const PointToPoint& p_next) override;

	int m_repetitions;
	int m_samples;

	std::vector<ipe::Vector> controls_from_intersections(const std::vector<Line<K>>& lines,
																	     const std::optional<ipe::Vector>& start,
																	     const std::vector<ipe::Vector>& control_points,
																	     const std::optional<ipe::Vector>& end) const;
	std::vector<ipe::Bezier> controls_to_beziers(const std::vector<ipe::Vector>& control_points) const;
	std::optional<Point<K>> intersection(const std::vector<ipe::Bezier>& bzs, const Line<K>& l) const;
	double cost(const SlopeLadder& ladder, const PointToPoint& p_prev, const PointToPoint& p_next, const std::vector<ipe::Vector>& new_vertices) const;
};

class SplineCollapsePainting : public renderer::GeometryPainting {
  public:
	SplineCollapsePainting(const SlopeLadder& ladder, const PointToPoint& p_prev, const PointToPoint& p_next, SplineCollapse spline_collapse);
	void paint(renderer::GeometryRenderer &renderer) const override;

  private:
	const SlopeLadder& m_ladder;
	const PointToPoint& m_p_prev;
	const PointToPoint& m_p_next;
	const SplineCollapse m_spline_collapse;
};

//void midpoint_collapse(SlopeLadder& ladder, const PointToPoint& p_prev, const PointToPoint& p_next);

class PointCollapsePainting : public renderer::GeometryPainting {
  public:
	PointCollapsePainting(const SlopeLadder& ladder, const PointToPoint& p_prev, const PointToPoint& p_next);
	void paint(renderer::GeometryRenderer &renderer) const override;

  private:
	const SlopeLadder& m_ladder;
	const PointToPoint& m_p_prev;
	const PointToPoint& m_p_next;
};

class MidpointCollapse : public LadderCollapse {
  public:
	MidpointCollapse() = default;
	void operator()(SlopeLadder& ladder, const PointToPoint& p_prev, const PointToPoint& p_next) override;
	std::shared_ptr<renderer::GeometryPainting> painting(const SlopeLadder& ladder, const PointToPoint& p_prev, const PointToPoint& p_next) override;
};

class MinSymDiffCollapse : public LadderCollapse {
  public:
	MinSymDiffCollapse() = default;
	void operator()(SlopeLadder& ladder, const PointToPoint& p_prev, const PointToPoint& p_next) override;
	std::shared_ptr<renderer::GeometryPainting> painting(const SlopeLadder& ladder, const PointToPoint& p_prev, const PointToPoint& p_next) override;
};

class HarmonyLineCollapse : public LadderCollapse {
  public:
	explicit HarmonyLineCollapse(int samples = 50);
	void operator()(SlopeLadder& ladder, const PointToPoint& p_prev, const PointToPoint& p_next) override;
	std::shared_ptr<renderer::GeometryPainting> painting(const SlopeLadder& ladder, const PointToPoint& p_prev, const PointToPoint& p_next) override;

	static std::pair<Point<K>, bool> new_vertex(const Line<K>& harmony_line, const Point<K>& s, const Point<K>& t, const Point<K>& u, const Point<K>& v);

	int m_samples;
};

class LineSplineHybridCollapse : public LadderCollapse {
  public:
	LineSplineHybridCollapse(SplineCollapse spline_collapse, HarmonyLineCollapse line_collapse);
	void operator()(SlopeLadder& ladder, const PointToPoint& p_prev, const PointToPoint& p_next) override;
	std::shared_ptr<renderer::GeometryPainting> painting(const SlopeLadder& ladder, const PointToPoint& p_prev, const PointToPoint& p_next) override;

	SplineCollapse m_spline_collapse;
	HarmonyLineCollapse m_line_collapse;

  private:
	static bool do_line_collapse(const SlopeLadder& ladder, const PointToPoint& p_prev, const PointToPoint& p_next);
};

class HarmonyLinePainting : public renderer::GeometryPainting {
  public:
	HarmonyLinePainting(const SlopeLadder& ladder, const PointToPoint& p_prev, const PointToPoint& p_next, const HarmonyLineCollapse& line_collapse);
	void paint(renderer::GeometryRenderer &renderer) const override;

	const int m_samples;
	const SlopeLadder& m_ladder;
	const PointToPoint& m_p_prev;
	const PointToPoint& m_p_next;
};

//void min_sym_diff_collapse(SlopeLadder& ladder, const PointToPoint& p_prev, const PointToPoint& p_next);

typedef std::unordered_map<Point<K>, std::vector<std::shared_ptr<SlopeLadder>>> PointToSlopeLadders;
typedef std::unordered_map<Segment<K>, std::vector<std::shared_ptr<SlopeLadder>>> EdgeToSlopeLadders;

Line<K> area_preservation_line(Point<K> s, Point<K> t, Point<K> u, Point<K> v);
double symmetric_difference(const Point<K>& s, const Point<K>& t, const Point<K>& u, const Point<K>& v, const Point<K>& p);
double signed_area(const std::vector<Point<K>>& pts);
double area(const std::vector<Point<K>>& pts);
bool point_order_on_line(Line<K> l, Point<K> a, Point<K> b);
}
#endif //CARTOCROW_COLLAPSE_H
