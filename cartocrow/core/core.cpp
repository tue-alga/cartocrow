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
#include <algorithm>
#include <cmath>
#include <CGAL/number_utils_classes.h>

namespace cartocrow {

Number<Inexact> approximate(const Number<Exact>& p) {
	return CGAL::to_double(p);
}

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

PolygonSet<Inexact> approximate(const PolygonSet<Exact>& p) {
	PolygonSet<Inexact> result;
	std::vector<PolygonWithHoles<Exact>> polygons;
	p.polygons_with_holes(std::back_inserter(polygons));
	for (auto polygon : polygons) {
		result.insert(approximate(polygon));
	}
	return result;
}

Color Color::shaded(double f) const {
	f = std::clamp(f, 0.0, 2.0);
	if (f < 1) {
		// darken (shade)
		return {
			(int) std::round(r * f),
			(int) std::round(g * f),
			(int) std::round(b * f)
		};
	} else {
		// lighten (tint)
		f -= 1;
		return {
			(int) std::round(r + (255 - r) * f),
			(int) std::round(g + (255 - g) * f),
			(int) std::round(b + (255 - b) * f),
		};
	}
}

Number<Inexact> wrapAngle(Number<Inexact> alpha, Number<Inexact> beta) {
	return wrap<Inexact>(alpha, beta, beta + M_2xPI);
}

Number<Inexact> wrapAngleUpper(Number<Inexact> alpha, Number<Inexact> beta) {
	return wrapUpper<Inexact>(alpha, beta, beta + M_2xPI);
}

} // namespace cartocrow
