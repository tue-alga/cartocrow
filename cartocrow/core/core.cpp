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

Point<Inexact> approximate(const Point<Exact>& p) {
	return Point<Inexact>(CGAL::to_double(p.x()), CGAL::to_double(p.y()));
}

Vector<Inexact> approximate(const Vector<Exact>& v) {
	return Vector<Inexact>(CGAL::to_double(v.x()), CGAL::to_double(v.y()));
}

Circle<Inexact> approximate(const Circle<Exact>& c) {
	return Circle<Inexact>(approximate(c.center()), CGAL::to_double(c.squared_radius()));
}

Line<Inexact> approximate(const Line<Exact>& l) {
	return Line<Inexact>(CGAL::to_double(l.a()), CGAL::to_double(l.b()), CGAL::to_double(l.c()));
}

Segment<Inexact> approximate(const Segment<Exact>& s) {
	return Segment<Inexact>(approximate(s.start()), approximate(s.end()));
}

Polygon<Inexact> approximate(const Polygon<Exact>& p) {
	Polygon<Inexact> result;
	for (auto v = p.vertices_begin(); v < p.vertices_end(); ++v) {
		result.push_back(approximate(*v));
	}
	return result;
}

PolygonWithHoles<Inexact> approximate(const PolygonWithHoles<Exact>& p) {
	PolygonWithHoles<Inexact> result(approximate(p.outer_boundary()));
	for (auto hole = p.holes_begin(); hole < p.holes_end(); ++hole) {
		result.add_hole(approximate(*hole));
	}
	return result;
}

Number<Inexact> wrapAngle(Number<Inexact> alpha, Number<Inexact> beta) {
	return wrap<Inexact>(alpha, beta, beta + M_2xPI);
}

Number<Inexact> wrapAngleUpper(Number<Inexact> alpha, Number<Inexact> beta) {
	return wrapUpper<Inexact>(alpha, beta, beta + M_2xPI);
}

} // namespace cartocrow
