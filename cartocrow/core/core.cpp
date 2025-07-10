/*
The CartoCrow library implements algorithmic geo-visualization methods,
developed at TU Eindhoven.
Copyright (C) 2021  Netherlands eScience Center and TU Eindhoven

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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 05-12-2019
*/

#include "core.h"
#include <CGAL/number_utils_classes.h>

namespace cartocrow {

Color::Color() : r(0), g(0), b(0) {}
Color::Color(int r, int g, int b) : r(r), g(g), b(b) {}
Color::Color(int rgb) : r((rgb & 0xff0000) >> 16), g((rgb & 0x00ff00) >> 8), b(rgb & 0x0000ff) {}

Number<Inexact> wrapAngle(Number<Inexact> alpha, Number<Inexact> beta) {
	return wrap<Inexact>(alpha, beta, beta + M_2xPI);
}

Number<Inexact> wrapAngleUpper(Number<Inexact> alpha, Number<Inexact> beta) {
	return wrapUpper<Inexact>(alpha, beta, beta + M_2xPI);
}

Point<Exact> pretendExact(const Point<Inexact>& p) {
	return {p.x(), p.y()};
}

Vector<Exact> pretendExact(const Vector<Inexact>& v) {
	return {v.x(), v.y()};
}

Circle<Exact> pretendExact(const Circle<Inexact>& c) {
	return {pretendExact(c.center()), c.squared_radius()};
}

Line<Exact> pretendExact(const Line<Inexact>& l) {
	return {l.a(), l.b(), l.c()};
}

Ray<Exact> pretendExact(const Ray<Inexact>& r) {
	return {pretendExact(r.source()), pretendExact(r.to_vector())};
}

Segment<Exact> pretendExact(const Segment<Inexact>& s) {
	return {pretendExact(s.source()), pretendExact(s.target())};
}

Rectangle<Exact> pretendExact(const Rectangle<Inexact>& r) {
	return {r.xmin(), r.ymin(), r.xmax(), r.ymax()};
}

Triangle<Exact> pretendExact(const Triangle<Inexact>& t) {
	return {pretendExact(t.vertex(0)), pretendExact(t.vertex(1)), pretendExact(t.vertex(2))};
}

Polygon<Exact> pretendExact(const Polygon<Inexact>& p) {
	std::vector<Point<Exact>> exact_points;
	pretendExact(p.vertices_begin(), p.vertices_end(), std::back_inserter(exact_points));
	return {exact_points.begin(), exact_points.end()};
}

PolygonWithHoles<Exact> pretendExact(const PolygonWithHoles<Inexact>& p) {
	auto outer = pretendExact(p.outer_boundary());
	std::vector<Polygon<Exact>> holes;
	for (const auto& h : p.holes()) {
		holes.push_back(pretendExact(h));
	}
	return {outer, holes.begin(), holes.end()};
}

PolygonSet<Exact> pretendExact(const PolygonSet<Inexact>& p) {
	std::vector<PolygonWithHoles<Inexact>> polygons;
	p.polygons_with_holes(std::back_inserter(polygons));

	PolygonSet<Exact> result;
	for (const auto& polygon : polygons) {
		result.insert(pretendExact(polygon));
	}
	return result;
}

} // namespace cartocrow
