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

#include "collapse.h"

#include <utility>
#include "ipeshape.h"
#include "ipegeo.h"
#include "ipe_bezier_wrapper.h"

namespace cartocrow::isoline_simplification {
SplineCollapse::SplineCollapse(int repetitions, int samples) : m_repetitions(repetitions), m_samples(samples) {};

std::vector<ipe::Bezier> SplineCollapse::controls_to_beziers(const std::vector<ipe::Vector>& control_points) const {
	ipe::Curve curve;
	curve.appendSpline(control_points);
	if (curve.countSegments() > 1) {
		throw std::runtime_error("Expected only one segment in spline.");
	}

	std::vector<ipe::Bezier> bzs;
	auto curved_segment = curve.segment(0);
	curved_segment.beziers(bzs);
	return bzs;
}

std::optional<Point<K>> SplineCollapse::intersection(const std::vector<ipe::Bezier>& bzs, const Line<K>& l) const {
	ipe::Line line = ipe::Line::through(pv(l.point(0)), pv(l.point(1)));
	std::vector<ipe::Vector> inters;
	for (auto& b : bzs) {
		b.intersect(line, inters);
	}
	if (inters.size() != 1) {
		return std::optional<Point<K>>();
	}
	return std::optional(vp(inters.front()));
}

std::vector<ipe::Vector> SplineCollapse::controls_from_intersections(const std::vector<Line<K>>& lines,
                                            const std::optional<ipe::Vector>& start,
                                            const std::vector<ipe::Vector>& control_points,
                                            const std::optional<ipe::Vector>& end) const {
	std::vector<ipe::Vector> new_controls;
	std::vector<ipe::Vector> all_controls;
	if (start.has_value()) {
		all_controls.push_back(*start);
	}
	for (const auto& cp : control_points) {
		all_controls.push_back(cp);
	}
	if (end.has_value()) {
		all_controls.push_back(*end);
	}
	auto bzs = controls_to_beziers(all_controls);
	for (int i = 0; i < lines.size(); i++) {
		auto& l = lines.at(i);
		Point<K> new_vertex;

		auto inter = intersection(bzs, l);
		new_vertex = inter.has_value() ? *inter : vp(control_points.at(i));
		new_controls.push_back(pv(new_vertex));
	}
	return new_controls;
}

double SplineCollapse::cost(const SlopeLadder& ladder, const PointToPoint& p_prev, const PointToPoint& p_next, const std::vector<ipe::Vector>& new_vertices) const {
	double cost = 0.0;

	for (int i = 0; i < ladder.m_rungs.size(); i++) {
		const auto& rung = ladder.m_rungs[i];
		auto reversed = p_next.contains(rung.target()) && p_next.at(rung.target()) == rung.source();
		auto t = reversed ? rung.target() : rung.source();
		auto u = reversed ? rung.source() : rung.target();
		Point<K> s = p_prev.at(t);
		Point<K> v = p_next.at(u);
		cost += symmetric_difference(s, t, u, v, vp(new_vertices[i]));
	}

	return cost;
}

void SplineCollapse::operator()(SlopeLadder& ladder, const PointToPoint& p_prev, const PointToPoint& p_next) {
	if (!ladder.m_valid)
		return;

	if (ladder.m_rungs.size() == 1) {
		MinSymDiffCollapse msdf;
		msdf(ladder, p_prev, p_next);
		return;
	}

	std::optional<ipe::Vector> start;
//	if (ladder.m_cap.contains(CGAL::LEFT_TURN)) {
//		start = pv(ladder.m_cap.at(CGAL::LEFT_TURN));
//	}

	std::optional<ipe::Vector> end;
//	if (ladder.m_cap.contains(CGAL::RIGHT_TURN)) {
//		end = pv(ladder.m_cap.at(CGAL::RIGHT_TURN));
//	}

	std::vector<ipe::Vector> best_controls;
	double best_cost = std::numeric_limits<double>::infinity();

	std::vector<std::pair<Point<K>, Point<K>>> intervals;
	double left_dist = std::numeric_limits<double>::infinity();
	double right_dist = std::numeric_limits<double>::infinity();
	std::vector<Line<K>> lines;
	for (const auto& rung : ladder.m_rungs) {
		auto reversed =
		    p_next.contains(rung.target()) && p_next.at(rung.target()) == rung.source();
		auto t = rung.source();
		auto u = rung.target();
		Point<K> s = reversed ? p_next.at(u) : p_prev.at(t);
		Point<K> v = reversed ? p_prev.at(t) : p_next.at(u);
		auto area_l = area_preservation_line(s, t, u, v);
		lines.push_back(area_l);

		std::optional<Point<K>> first;
		std::optional<Point<K>> last;

		for (auto p : {s, t, u, v}) {
			auto proj = area_l.projection(p);
			if (!first.has_value() || point_order_on_line(area_l, proj, *first)) {
				first = proj;
			}
			if (!last.has_value() || point_order_on_line(area_l, *last, proj)) {
				last = proj;
			}
		}

		auto mid = area_l.projection(midpoint(rung));
		double this_left_dist = sqrt((mid - *first).squared_length());
		double this_right_dist = sqrt((*last - mid).squared_length());
		if (this_left_dist < left_dist) {
			left_dist = this_left_dist;
		}
		if (this_right_dist < right_dist) {
			right_dist = this_right_dist;
		}
		intervals.emplace_back(*first, *last);
	}

	for (int i = 0; i < m_samples; i++) {
		std::vector<ipe::Vector> initial_controls;
		for (int j = 0; j < ladder.m_rungs.size(); j++) {
			const auto& rung = ladder.m_rungs[j];
			const double step = (left_dist + right_dist) / (m_samples - 1);
			const auto& [a, b] = intervals[j];
			const auto diff = (b - a);
			int cutoff = left_dist / (left_dist + right_dist) * m_samples;
			auto mid = lines[j].projection(midpoint(rung));
			const auto step_v = diff / sqrt(diff.squared_length()) * step;
			if (i <= cutoff) {
				initial_controls.push_back(pv(mid - step_v * i));
			} else {
				initial_controls.push_back(pv(mid + step_v * (i - cutoff)));
			}
		}

		if (initial_controls.size() > 1 || start.has_value() || end.has_value()) {
			for (int j = 0; j < m_repetitions; j++) {
				initial_controls = controls_from_intersections(lines, start, initial_controls, end);
			}
		}

		bool too_close = false;
		for (int j = 0; j < ladder.m_rungs.size(); j++) {
			const auto& rung = ladder.m_rungs[j];
			auto reversed =
			    p_next.contains(rung.target()) && p_next.at(rung.target()) == rung.source();
			auto t = reversed ? rung.target() : rung.source();
			auto u = reversed ? rung.source() : rung.target();
			Point<K> s = p_prev.at(t);
			Point<K> v = p_next.at(u);
			auto p = vp(initial_controls.at(j));
			if (CGAL::squared_distance(s, p) < 1E-6 || CGAL::squared_distance(p, v) < 1E-6) {
				too_close = true;
			}
		}

		if (too_close) {
			continue;
		}

		double the_cost = cost(ladder, p_prev, p_next, initial_controls);
		if (the_cost < best_cost) {
			best_controls = initial_controls;
			best_cost = the_cost;
		}
	}

	if (isinf(best_cost)) {
		std::cerr << "All samples of spline are too close to s and v. Falling back to midpoint." << std::endl;
		MidpointCollapse()(ladder, p_prev, p_next);
	} else {
		ladder.m_collapsed.clear();
		std::transform(best_controls.begin(), best_controls.end(),
		               std::back_inserter(ladder.m_collapsed),
		               [](const ipe::Vector& v) { return vp(v); });
	}
}

SplineCollapsePainting::SplineCollapsePainting(const SlopeLadder& ladder, const PointToPoint& p_prev, const PointToPoint& p_next, SplineCollapse spline_collapse) :
 	m_ladder(ladder), m_p_prev(p_prev), m_p_next(p_next), m_spline_collapse(std::move(spline_collapse)) {  }

std::shared_ptr<renderer::GeometryPainting> SplineCollapse::painting(const SlopeLadder& ladder, const PointToPoint& p_prev, const PointToPoint& p_next) {
	return std::make_shared<SplineCollapsePainting>(ladder, p_prev, p_next, *this);
}

void SplineCollapsePainting::paint(cartocrow::renderer::GeometryRenderer& renderer) const {
	if (!m_ladder.m_valid)
		return;

	if (m_ladder.m_rungs.size() == 1) {
		MinSymDiffCollapse msdf;
		msdf.painting(m_ladder, m_p_prev, m_p_next)->paint(renderer);
		return;
	}

	std::optional<ipe::Vector> start;
//	if (m_ladder.m_cap.contains(CGAL::LEFT_TURN)) {
//		start = pv(m_ladder.m_cap.at(CGAL::LEFT_TURN));
//	}

	std::optional<ipe::Vector> end;
//	if (m_ladder.m_cap.contains(CGAL::RIGHT_TURN)) {
//		end = pv(m_ladder.m_cap.at(CGAL::RIGHT_TURN));
//	}

	auto draw_controls = [&](std::vector<ipe::Vector>& controls, bool best) {
		renderer.setMode(renderer::GeometryRenderer::stroke);
		if (!best) {
			renderer.setStroke(Color{20, 20, 255}, 1.0);
			renderer.setStrokeOpacity(100);
		} else {
			renderer.setStroke(Color{20, 20, 255}, 3.0);
		}

	    for (const auto& v : controls) {
	  	  renderer.draw(vp(v));
	    }

		if (controls.size() > 1 || start.has_value() || end.has_value()) {
			std::vector<ipe::Vector> all_controls;
			if (start.has_value()) {
				all_controls.push_back(*start);
			}
			for (const auto& cp : controls) {
				all_controls.push_back(cp);
			}
			if (end.has_value()) {
				all_controls.push_back(*end);
			}
			auto bzs = m_spline_collapse.controls_to_beziers(all_controls);
			BezierSpline spline;
			for (const auto& bz : bzs) {
				spline.appendCurve(vp(bz.iV[0]), vp(bz.iV[1]), vp(bz.iV[2]), vp(bz.iV[3]));
			}
			renderer.draw(spline);
		}
	};

	std::vector<ipe::Vector> best_controls;
	double best_cost = std::numeric_limits<double>::infinity();

	std::vector<std::pair<Point<K>, Point<K>>> intervals;
	double left_dist = std::numeric_limits<double>::infinity();
	double right_dist = std::numeric_limits<double>::infinity();
	std::vector<Line<K>> lines;
	for (const auto& rung : m_ladder.m_rungs) {
		auto reversed = m_p_next.contains(rung.target()) && m_p_next.at(rung.target()) == rung.source();
		auto t = rung.source();
		auto u = rung.target();
		Point<K> s = reversed ? m_p_next.at(u) : m_p_prev.at(t);
		Point<K> v = reversed ? m_p_prev.at(t) : m_p_next.at(u);
		lines.emplace_back(area_preservation_line(s, t, u, v));
		auto& area_l = lines.back();

		std::optional<Point<K>> first;
		std::optional<Point<K>> last;

		for (auto p : {s, t, u, v}) {
			auto proj = area_l.projection(p);
			if (!first.has_value() || point_order_on_line(area_l, proj, *first)) {
				first = proj;
			}
			if (!last.has_value() || point_order_on_line(area_l, *last, proj)) {
				last = proj;
			}
		}

		auto mid = area_l.projection(midpoint(rung));
		double this_left_dist = sqrt((mid - *first).squared_length());
		double this_right_dist = sqrt((*last - mid).squared_length());
		if (this_left_dist < left_dist) {
			left_dist = this_left_dist;
		}
		if (this_right_dist < right_dist) {
			right_dist = this_right_dist;
		}
		intervals.emplace_back(*first, *last);
	}

	for (int i = 0; i < m_spline_collapse.m_samples; i++) {
		std::vector<ipe::Vector> initial_controls;
		for (int j = 0; j < m_ladder.m_rungs.size(); j++) {
			const auto& rung = m_ladder.m_rungs[j];
			const double step = (left_dist + right_dist) / (m_spline_collapse.m_samples - 1);
			const auto& [a, b] = intervals[j];
			const auto diff = (b - a);
			int cutoff = left_dist / (left_dist + right_dist) * m_spline_collapse.m_samples;
			auto mid = lines[j].projection(midpoint(rung));
			const auto step_v = diff / sqrt(diff.squared_length()) * step;
			if (i <= cutoff) {
				initial_controls.push_back(pv(mid - step_v * i));
			} else {
				initial_controls.push_back(pv(mid + step_v * (i - cutoff)));
			}
		}

		if (initial_controls.size() > 1 || start.has_value() || end.has_value()) {
			for (int j = 0; j < m_spline_collapse.m_repetitions - 1; j++) { // note the -1 for drawing purposes
				initial_controls = m_spline_collapse.controls_from_intersections(lines, start, initial_controls, end);
			}
		}

		double the_cost = m_spline_collapse.cost(m_ladder, m_p_prev, m_p_next, initial_controls);
		if (the_cost < best_cost) {
			best_controls = initial_controls;
			best_cost = the_cost;
		}

		draw_controls(initial_controls, false);
	}

	draw_controls(best_controls, true);
}

Point<K> min_sym_diff_point(Point<K> s, Point<K> t, Point<K> u, Point<K> v, Line<K> l) {
	Line<K> svl(s, v);
	Line<K> stl(s, t);
	Line<K> uvl(u, v);
	Point<K> new_vertex;
	// degenerate cases seem to never occur
	if (svl.oriented_side(t) == svl.oriented_side(u)) {
		if (squared_distance(svl, t) > squared_distance(svl, u)) {
			auto i = *intersection(l, stl);
			if (i.type() == typeid(Point<K>)) {
				new_vertex = *boost::get<Point<K>>(&i);
			} else {
				new_vertex = midpoint(s, v);
			}
		} else {
			auto i = *intersection(l, uvl);
			if (i.type() == typeid(Point<K>)) {
				new_vertex = *boost::get<Point<K>>(&i);
			} else {
				new_vertex = midpoint(s, v);
			}
		}
	} else {
		if (svl.oriented_side(t) == svl.oriented_side(l.point())) {
			auto i = *intersection(l, stl);
			if (i.type() == typeid(Point<K>)) {
				new_vertex = *boost::get<Point<K>>(&i);
			} else {
				new_vertex = midpoint(s, v);
			}
		} else {
			auto i = *intersection(l, uvl);
			if (i.type() == typeid(Point<K>)) {
				new_vertex = *boost::get<Point<K>>(&i);
			} else {
				new_vertex = midpoint(s, v);
			}
		}
	}
	// If nearly collinear use midpoint
	auto dist_threshold = 0.0001 * squared_distance(s, v);
	if (squared_distance(new_vertex, s) < dist_threshold || squared_distance(new_vertex, v) < dist_threshold) {
		new_vertex = l.projection(midpoint(s, v));
	}
	return new_vertex;
}

Point<K> projected_midpoint(Point<K> s, Point<K> t, Point<K> u, Point<K> v, Line<K> l) {
	return l.projection(midpoint(t, u));
}

void MinSymDiffCollapse::operator()(SlopeLadder& ladder, const PointToPoint& p_prev, const PointToPoint& p_next) {
	ladder.m_collapsed.clear();
	if (!ladder.m_valid) return;
	for (const auto& rung : ladder.m_rungs) {
		auto reversed = p_next.contains(rung.target()) && p_next.at(rung.target()) == rung.source();
		auto t = reversed ? rung.target() : rung.source();
		auto u = reversed ? rung.source() : rung.target();
		Point<K> s;
		if (p_prev.contains(t)) {
			s = p_prev.at(t);
		} else {
			return;
		}
		Point<K> v;
		if (p_next.contains(u)) {
			v = p_next.at(u);
		} else {
			return;
		}

		ladder.m_collapsed.push_back(min_sym_diff_point(s, t, u, v, area_preservation_line(s, t, u, v)));
	}
}

std::shared_ptr<renderer::GeometryPainting> MinSymDiffCollapse::painting(const SlopeLadder& ladder, const PointToPoint& p_prev, const PointToPoint& p_next) {
	return std::make_shared<PointCollapsePainting>(ladder, p_prev, p_next);
}

void MidpointCollapse::operator()(SlopeLadder& ladder, const PointToPoint& p_prev, const PointToPoint& p_next) {
	ladder.m_collapsed.clear();
	if (!ladder.m_valid) return;
	for (const auto& rung : ladder.m_rungs) {
		auto reversed = p_next.contains(rung.target()) && p_next.at(rung.target()) == rung.source();
		auto t = reversed ? rung.target() : rung.source();
		auto u = reversed ? rung.source() : rung.target();
		Point<K> s = p_prev.at(t);
		Point<K> v = p_next.at(u);

		ladder.m_collapsed.push_back(projected_midpoint(s, t, u, v, area_preservation_line(s, t, u, v)));
	}
}

std::shared_ptr<renderer::GeometryPainting> MidpointCollapse::painting(const SlopeLadder& ladder, const PointToPoint& p_prev, const PointToPoint& p_next) {
	return std::make_shared<PointCollapsePainting>(ladder, p_prev, p_next);
}

PointCollapsePainting::PointCollapsePainting(const SlopeLadder& ladder, const PointToPoint& p_prev, const PointToPoint& p_next) :
      m_ladder(ladder), m_p_prev(p_prev), m_p_next(p_next) {}

void PointCollapsePainting::paint(renderer::GeometryRenderer& renderer) const {

}

bool point_order_on_line(Line<K> l, Point<K> a, Point<K> b) {
	auto dir_line = l.to_vector();
	auto dir_pts = b - a;
	return dir_line * dir_pts > 0;
}

HarmonyLinePainting::HarmonyLinePainting(const SlopeLadder& ladder, const PointToPoint& p_prev, const PointToPoint& p_next, const HarmonyLineCollapse& line_collapse) :
	m_ladder(ladder), m_p_prev(p_prev), m_p_next(p_next), m_samples(line_collapse.m_samples) {}

void HarmonyLinePainting::paint(renderer::GeometryRenderer& renderer) const {
	if (!m_ladder.m_valid) return;
	if (m_ladder.m_rungs.size() == 1) {
		MinSymDiffCollapse msdf;
		msdf.painting(m_ladder, m_p_prev, m_p_next)->paint(renderer);
		return;
	}

	// Compute sample line
	Line<K> sample_line;
	Line<K> initial_harmony_line;
	if (m_ladder.m_rungs.size() == 1) {
		auto& rung = m_ladder.m_rungs.front();
		auto reversed = m_p_next.contains(rung.target()) && m_p_next.at(rung.target()) == rung.source();
		auto t = reversed ? rung.target() : rung.source();
		auto u = reversed ? rung.source() : rung.target();
		Point<K> s = m_p_prev.at(t);
		Point<K> v = m_p_next.at(u);
		sample_line = area_preservation_line(s, t, u, v);
		initial_harmony_line = sample_line.perpendicular(midpoint(t, u));
	} else {
		auto& first_rung = m_ladder.m_rungs.front();
		auto& last_rung = m_ladder.m_rungs.back();
		initial_harmony_line = Line<K>(midpoint(first_rung), midpoint(last_rung));
		sample_line = initial_harmony_line.perpendicular(midpoint(midpoint(first_rung), midpoint(last_rung)));
	}

	std::optional<Point<K>> first;
	std::optional<Point<K>> last;
	for (const auto& rung : m_ladder.m_rungs) {
		auto reversed = m_p_next.contains(rung.target()) && m_p_next.at(rung.target()) == rung.source();
		auto t = reversed ? rung.target() : rung.source();
		auto u = reversed ? rung.source() : rung.target();
		Point<K> s = m_p_prev.at(t);
		Point<K> v = m_p_next.at(u);
		auto area_l = area_preservation_line(s, t, u, v);
		for (auto& p : {s, t, u, v}) {
			auto pt = sample_line.projection(area_l.projection(p));
			if (!first.has_value() || point_order_on_line(sample_line, pt, *first)) {
				first = pt;
			}
			if (!last.has_value() || point_order_on_line(sample_line, *last, pt)) {
				last = pt;
			}
		}
	}

	K::Vector_2 step_v = (*last - *first) / (m_samples - 1);

	double best_cost = std::numeric_limits<double>::infinity();
	Line<K> best_line;

	for (int i = 0; i < m_samples; i++) {
		K::Vector_2 offset = i * step_v;
		Point<K> pt = *first + offset;
		Line<K> harmony_line(pt, initial_harmony_line.direction());

		renderer.setMode(renderer::GeometryRenderer::stroke);
		renderer.setStroke(Color{20, 20, 255}, 1.0);
		renderer.setStrokeOpacity(100);
		renderer.draw(harmony_line);

		double cost = 0.0;

		for (const auto& rung : m_ladder.m_rungs) {
			auto reversed = m_p_next.contains(rung.target()) && m_p_next.at(rung.target()) == rung.source();
			auto t = reversed ? rung.target() : rung.source();
			auto u = reversed ? rung.source() : rung.target();
			Point<K> s = m_p_prev.at(t);
			Point<K> v = m_p_next.at(u);

			auto [p, snapped] = HarmonyLineCollapse::new_vertex(harmony_line, s, t, u, v);

			cost += symmetric_difference(s, t, u, v, p);
		}

		if (cost < best_cost) {
			best_line = harmony_line;
			best_cost = cost;
		}
	}

	renderer.setMode(renderer::GeometryRenderer::stroke);
	renderer.setStroke(Color{20, 20, 255}, 3.0);
	renderer.draw(best_line);
}

HarmonyLineCollapse::HarmonyLineCollapse(int samples): m_samples(samples) {}

std::pair<Point<K>, bool> HarmonyLineCollapse::new_vertex(const Line<K>& harmony_line, const Point<K>& s, const Point<K>& t, const Point<K>& u, const Point<K>& v) {
	auto area_line = area_preservation_line(s, t, u, v);
	auto inter = *intersection(harmony_line, area_line);
	Point<K> new_vertex = *boost::get<Point<K>>(&inter);

	std::optional<Point<K>> first;
	std::optional<Point<K>> last;

	for (const auto& p : {s, t, u, v}) {
		auto pt = area_line.projection(p);
		if (!first.has_value() || point_order_on_line(area_line, pt, *first)) {
			first = pt;
		}
		if (!last.has_value() || point_order_on_line(area_line, *last, pt)) {
			last = pt;
		}
	}

	if (point_order_on_line(area_line, new_vertex, *first)) {
		return { *first, true };
	} else if (point_order_on_line(area_line, *last, new_vertex)) {
		return { *last, true };
	} else {
		return { new_vertex, false };
	}
}

void HarmonyLineCollapse::operator()(SlopeLadder& ladder, const PointToPoint& p_prev, const PointToPoint& p_next) {
	ladder.m_collapsed.clear();
	if (!ladder.m_valid) return;
	if (ladder.m_rungs.size() == 1) {
		MinSymDiffCollapse msdf;
		msdf(ladder, p_prev, p_next);
		return;
	}

	// Compute sample line
	Line<K> sample_line;
	Line<K> initial_harmony_line;
	if (ladder.m_rungs.size() == 1) {
		auto& rung = ladder.m_rungs.front();
		auto reversed = p_next.contains(rung.target()) && p_next.at(rung.target()) == rung.source();
		auto t = reversed ? rung.target() : rung.source();
		auto u = reversed ? rung.source() : rung.target();
		Point<K> s = p_prev.at(t);
		Point<K> v = p_next.at(u);
		sample_line = area_preservation_line(s, t, u, v);
		initial_harmony_line = sample_line.perpendicular(midpoint(t, u));
	} else {
		auto& first_rung = ladder.m_rungs.front();
		auto& last_rung = ladder.m_rungs.back();
		initial_harmony_line = Line<K>(midpoint(first_rung), midpoint(last_rung));
		sample_line =
		    initial_harmony_line.perpendicular(midpoint(midpoint(first_rung), midpoint(last_rung)));
	}

	std::optional<Point<K>> first;
	std::optional<Point<K>> last;
	for (const auto& rung : ladder.m_rungs) {
		auto reversed = p_next.contains(rung.target()) && p_next.at(rung.target()) == rung.source();
		auto t = reversed ? rung.target() : rung.source();
		auto u = reversed ? rung.source() : rung.target();
		Point<K> s = p_prev.at(t);
		Point<K> v = p_next.at(u);
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

	K::Vector_2 step_v = (*last - *first) / (m_samples - 1);

	double best_cost = std::numeric_limits<double>::infinity();
	Line<K> best_line;

	for (int i = 0; i < m_samples; i++) {
		K::Vector_2 offset = i * step_v;
		Point<K> pt = *first + offset;
		Line<K> harmony_line(pt, initial_harmony_line.direction());

		double cost = 0.0;

		for (const auto& rung : ladder.m_rungs) {
			auto reversed = p_next.contains(rung.target()) && p_next.at(rung.target()) == rung.source();
			auto t = reversed ? rung.target() : rung.source();
			auto u = reversed ? rung.source() : rung.target();
			Point<K> s = p_prev.at(t);
			Point<K> v = p_next.at(u);

			auto [p, snapped] = HarmonyLineCollapse::new_vertex(harmony_line, s, t, u, v);
			if (CGAL::squared_distance(s, p) < 1E-6 || CGAL::squared_distance(p, v) < 1E-6) {
				p = projected_midpoint(s, t, u, v, area_preservation_line(s, t, u, v));
			}

			cost += symmetric_difference(s, t, u, v, p);
		}

		if (cost < best_cost) {
			best_line = harmony_line;
			best_cost = cost;
		}
	}

	for (const auto& rung : ladder.m_rungs) {
		auto reversed = p_next.contains(rung.target()) && p_next.at(rung.target()) == rung.source();
		auto t = reversed ? rung.target() : rung.source();
		auto u = reversed ? rung.source() : rung.target();
		Point<K> s = p_prev.at(t);
		Point<K> v = p_next.at(u);


		Point<K> p = new_vertex(best_line, s, t, u, v).first;
		ladder.m_collapsed.push_back(p);
	}
}

std::shared_ptr<renderer::GeometryPainting> HarmonyLineCollapse::painting(const SlopeLadder& ladder, const PointToPoint& p_prev, const PointToPoint& p_next) {
	return std::make_shared<HarmonyLinePainting>(ladder, p_prev, p_next, *this);
}

double symmetric_difference(const Point<K>& s, const Point<K>& t, const Point<K>& u, const Point<K>& v, const Point<K>& p) {
	Segment<K> st = Segment<K>(s, t);
	Segment<K> tu = Segment<K>(t, u);
	Segment<K> uv = Segment<K>(u, v);

	Segment<K> sp = Segment<K>(s, p);
	Segment<K> pv = Segment<K>(p, v);

	auto st_pv = intersection(st, pv);
	auto tu_pv = intersection(tu, pv);
	auto tu_sp = intersection(tu, sp);
	auto uv_sp = intersection(uv, sp);

	double cost = 0.0;

	if (st_pv.has_value()) {
		if (auto st_pv_pt = boost::get<Point<K>>(&*st_pv)) {
			cost += area({s, p, *st_pv_pt});
		}
		if (tu_pv.has_value()) {
			if (auto tu_pv_pt = boost::get<Point<K>>(&*tu_pv)) {
				cost += area({*tu_pv_pt, u, v});
				if (auto st_pv_pt = boost::get<Point<K>>(&*st_pv)) {
					cost += area({*st_pv_pt, t, *tu_pv_pt});
				}
			}
		} else {
			if (auto st_pv_pt = boost::get<Point<K>>(&*st_pv)) {
				cost += area({*st_pv_pt, t, u, v});
			}
		}
	} else if (uv_sp.has_value()) {
		if (auto uv_sp_pt = boost::get<Point<K>>(&*uv_sp)) {
			cost += area({*uv_sp_pt, p, v});
		}
		if (tu_sp.has_value()) {
			if (auto tu_sp_pt = boost::get<Point<K>>(&*tu_sp)) {
				cost += area({s, t, *tu_sp_pt});
				if (auto uv_sp_pt = boost::get<Point<K>>(&*uv_sp)) {
					cost += area({*tu_sp_pt, u, *uv_sp_pt});
				}
			}
		} else {
			if (auto uv_sp_pt = boost::get<Point<K>>(&*uv_sp)) {
				cost += area({s, t, u, *uv_sp_pt});
			}
		}
	} else if (!tu_sp.has_value() && !tu_pv.has_value()) {
		cost += area({s, t, u, v, p});
	} else if (!tu_sp.has_value()) {
		if (auto tu_pv_pt = boost::get<Point<K>>(&*tu_pv)) {
			cost += area({p, s, t, *tu_pv_pt});
			cost += area({*tu_pv_pt, u, v});
		}
	} else if (!tu_pv.has_value()) {
		if (auto tu_sp_pt = boost::get<Point<K>>(&*tu_sp)) {
			cost += area({s, t, *tu_sp_pt});
			cost += area({*tu_sp_pt, u, v, p});
		}
	} else {
		if (auto tu_sp_pt = boost::get<Point<K>>(&*tu_sp)) {
			cost += area({s, t, *tu_sp_pt});
		}
		if (auto tu_pv_pt = boost::get<Point<K>>(&*tu_pv)) {
			cost += area({*tu_pv_pt, u, v});
		}
		if (auto tu_sp_pt = boost::get<Point<K>>(&*tu_sp)) {
			if (auto tu_pv_pt = boost::get<Point<K>>(&*tu_pv)) {
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
		Point<K> s = p_prev.at(t);
		Point<K> v = p_next.at(u);
		m_cost += symmetric_difference(s, t, u, v, m_collapsed[i]);
	}

	m_cost /= m_rungs.size();
}

double signed_area(const std::vector<Point<K>>& pts) {
	double total = 0.0;
	auto prev = pts.back();
	for (const Point<K>& curr : pts) {
		total += prev.x() * curr.y() - prev.y() * curr.x();
		prev = curr;
	}
	return total / 2.0;
}

double area(const std::vector<Point<K>>& pts) {
	return abs(signed_area(pts));
}

Line<K> area_preservation_line(Point<K> s, Point<K> t, Point<K> u, Point<K> v) {
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
	return Line<K>(a, b, c);
}

LineSplineHybridCollapse::LineSplineHybridCollapse(SplineCollapse spline_collapse, HarmonyLineCollapse line_collapse) :
      m_spline_collapse(std::move(spline_collapse)), m_line_collapse(std::move(line_collapse)) {};

void LineSplineHybridCollapse::operator()(SlopeLadder& ladder, const PointToPoint& p_prev, const PointToPoint& p_next) {
	if (!ladder.m_valid) return;
	if (do_line_collapse(ladder, p_prev, p_next)) {
		m_line_collapse(ladder, p_prev, p_next);
	} else {
		m_spline_collapse(ladder, p_prev, p_next);
	}
}

std::shared_ptr<renderer::GeometryPainting> LineSplineHybridCollapse::painting(const SlopeLadder& ladder, const PointToPoint& p_prev, const PointToPoint& p_next) {
	if (do_line_collapse(ladder, p_prev, p_next)) {
		return std::make_shared<HarmonyLinePainting>(ladder, p_prev, p_next, m_line_collapse);
	} else {
		return std::make_shared<SplineCollapsePainting>(ladder, p_prev, p_next, m_spline_collapse);
	}
}

bool LineSplineHybridCollapse::do_line_collapse(const SlopeLadder& ladder, const PointToPoint& p_prev, const PointToPoint& p_next) {
	if (!ladder.m_valid) return false;
	Line<K> sample_line;
	Line<K> initial_harmony_line;
	if (ladder.m_rungs.size() == 1) {
		auto& rung = ladder.m_rungs.front();
		auto reversed = p_next.contains(rung.target()) && p_next.at(rung.target()) == rung.source();
		auto t = reversed ? rung.target() : rung.source();
		auto u = reversed ? rung.source() : rung.target();
		Point<K> s = p_prev.at(t);
		Point<K> v = p_next.at(u);
		sample_line = area_preservation_line(s, t, u, v);
		initial_harmony_line = sample_line.perpendicular(midpoint(t, u));
	} else {
		auto& first_rung = ladder.m_rungs.front();
		auto& last_rung = ladder.m_rungs.back();
		initial_harmony_line = Line<K>(midpoint(first_rung), midpoint(last_rung));
		sample_line = initial_harmony_line.perpendicular(midpoint(midpoint(first_rung), midpoint(last_rung)));
	}

	bool intersects_all_rungs = true;
	for (const auto& rung : ladder.m_rungs) {
		if (!intersection(rung, initial_harmony_line).has_value()) {
			intersects_all_rungs = false;
			break;
		}
	}

	return intersects_all_rungs;
}
}
