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

Point<Inexact> toInexact(const Point<Exact>& p) {
	return Point<Inexact>(CGAL::to_double(p.x()), CGAL::to_double(p.y()));
}

Vector<Inexact> toInexact(const Vector<Exact>& v) {
	return Vector<Inexact>(CGAL::to_double(v.x()), CGAL::to_double(v.y()));
}

Circle<Inexact> toInexact(const Circle<Exact>& c) {
	return Circle<Inexact>(toInexact(c.center()), CGAL::to_double(c.squared_radius()));
}

Line<Inexact> toInexact(const Line<Exact>& l) {
	return Line<Inexact>(CGAL::to_double(l.a()), CGAL::to_double(l.b()), CGAL::to_double(l.c()));
}

Segment<Inexact> toInexact(const Segment<Exact>& s) {
	return Segment<Inexact>(toInexact(s.start()), toInexact(s.end()));
}

Polygon<Inexact> toInexact(const Polygon<Exact>& p) {
	Polygon<Inexact> result;
	for (auto v = p.vertices_begin(); v < p.vertices_end(); ++v) {
		result.push_back(toInexact(*v));
	}
	return result;
}

PolygonWithHoles<Inexact> toInexact(const PolygonWithHoles<Exact>& p) {
	PolygonWithHoles<Inexact> result(toInexact(p.outer_boundary()));
	for (auto hole = p.holes_begin(); hole < p.holes_end(); ++hole) {
		result.add_hole(toInexact(*hole));
	}
	return result;
}

} // namespace cartocrow
