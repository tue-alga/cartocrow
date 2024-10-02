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

Number<Inexact> wrapAngle(Number<Inexact> alpha, Number<Inexact> beta) {
	return wrap<Inexact>(alpha, beta, beta + M_2xPI);
}

Number<Inexact> wrapAngleUpper(Number<Inexact> alpha, Number<Inexact> beta) {
	return wrapUpper<Inexact>(alpha, beta, beta + M_2xPI);
}

} // namespace cartocrow
