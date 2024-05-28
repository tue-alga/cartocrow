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

#include "simple_smoothing.h"
#include "ipe_bezier_wrapper.h"
#include "ipeshape.h"

namespace cartocrow::isoline_simplification {
BezierSpline simple_smoothing(const Isoline<K>& iso) {
	if (iso.m_closed) {
		std::vector<ipe::Vector> vs;
		std::vector<Point<K>> pts(iso.m_points.begin(), iso.m_points.end());
		for (int i = 0; i < pts.size(); i++) {
			Point<K> p1;
			Point<K> p2;
			if (i == 0) {
				p1 = pts[i] + (pts.back() - pts[i]) / 4;
				p2 = pts[i] + (pts[i+1] - pts[i]) / 4;
			} else if (i == pts.size() - 1) {
				p1 = pts[i] + (pts[i-1] - pts[i]) / 4;
				p2 = pts[i] + (pts.front() - pts[i]) / 4;
			} else {
				p1 = pts[i] + (pts[i-1] - pts[i]) / 4;
				p2 = pts[i] + (pts[i+1] - pts[i]) / 4;
			}
			vs.push_back(pv(p1));
			vs.push_back(pv(p2));
		}
		ipe::ClosedSpline spline(vs);
		std::vector<ipe::Bezier> bzs;
		spline.beziers(bzs);
		return parse_ipe_beziers(bzs);
	} else {
		std::vector<ipe::Vector> vs;
		std::vector<Point<K>> pts(iso.m_points.begin(), iso.m_points.end());
		for (int i = 0; i < pts.size(); i++) {
			if (i == 0 || i == pts.size() - 1) {
				vs.push_back(pv(pts[i]));
			} else {
				Point<K> p1 = pts[i] + (pts[i-1] - pts[i]) / 4;
				Point<K> p2 = pts[i] + (pts[i+1] - pts[i]) / 4;
				vs.push_back(pv(p1));
				vs.push_back(pv(p2));
			}
		}
		ipe::Curve curve;
		curve.appendSpline(vs);

		std::vector<ipe::Bezier> bzs;
		for (int i = 0; i < curve.countSegments(); i++) {
			curve.segment(i).beziers(bzs);
		}
		return parse_ipe_beziers(bzs);
	}
}
}
