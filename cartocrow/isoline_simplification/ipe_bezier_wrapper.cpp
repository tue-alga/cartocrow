/*
The CartoCrow library implements algorithmic geo-visualization methods,
developed at TU Eindhoven.
Copyright (C) 2024 TU Eindhoven

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

#include "ipe_bezier_wrapper.h"

namespace cartocrow::isoline_simplification {
ipe::Vector pv(Gt::Point_2 p) {
	return ipe::Vector(p.x(), p.y());
}

Gt::Point_2 vp(ipe::Vector v) {
	return Gt::Point_2(v.x, v.y);
}

std::variant<ipe::Bezier, ipe::Segment> parabola_to_curve(Gt::Line_2 dir, Gt::Point_2 focus, Gt::Point_2 p1, Gt::Point_2 p2) {
	auto start_p = dir.projection(p1);
	auto end_p = dir.projection(p1);

	// If the points are collinear CGAL::circumcenter throws an error; draw a segment instead.
	if (CGAL::collinear(focus, start_p, end_p)) {
		return ipe::Segment(pv(p1), pv(p2));
	} else {
		auto control = CGAL::circumcenter(focus, start_p, end_p);
		return ipe::Bezier::quadBezier(pv(p1), pv(control), pv(p2));
	}
}

std::vector<Gt::Point_2> parabola_intersections(Gt::Segment_2 seg, Gt::Line_2 dir, Gt::Point_2 focus, Gt::Point_2 p1, Gt::Point_2 p2) {
	std::vector<Gt::Point_2> pts;
	ipe::Segment ipe_seg(pv(seg.source()), pv(seg.target()));
	auto curve = parabola_to_curve(dir, focus, p1, p2);
	if (std::holds_alternative<ipe::Bezier>(curve)) {
		auto bezier = std::get<ipe::Bezier>(curve);
		std::vector<ipe::Vector> ipe_inters;
		bezier.intersect(ipe_seg, ipe_inters);
		for (const auto& inter : ipe_inters) {
			pts.push_back(vp(inter));
		}
	} else {
		auto other = std::get<ipe::Segment>(curve);
		ipe::Vector inter;

		if (ipe_seg.intersects(other, inter)) {
			pts.push_back(vp(inter));
		}
	}
	return pts;
}

BezierCurve parse_ipe_bezier(const ipe::Bezier& bz) {
	return { vp(bz.iV[0]), vp(bz.iV[1]), vp(bz.iV[2]), vp(bz.iV[3]) };
}

BezierSpline parse_ipe_beziers(const std::vector<ipe::Bezier>& bzs) {
	BezierSpline spline;
	for (const auto& bz : bzs) {
		spline.appendCurve(parse_ipe_bezier(bz));
	}
	return spline;
}
}