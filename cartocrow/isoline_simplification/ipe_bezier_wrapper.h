/*
The CartoCrow library implements algorithmic geo-visualization methods,
developed at TU Eindhoven.
Copyright (C) 2021  Netherlands eScience Center and TU Eindhoven

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

#ifndef CARTOCROW_IPE_BEZIER_WRAPPER_H
#define CARTOCROW_IPE_BEZIER_WRAPPER_H

#include "ipeshape.h"
#include "ipegeo.h"
#include "types.h"

namespace cartocrow::isoline_simplification {
std::vector<Gt::Point_2> parabola_intersections(Gt::Segment_2 seg, Gt::Line_2 dir,
                                                Gt::Point_2 focus, Gt::Point_2 p1, Gt::Point_2 p2);
ipe::Vector pv(Gt::Point_2 p);
Gt::Point_2 vp(ipe::Vector p);
}
#endif //CARTOCROW_IPE_BEZIER_WRAPPER_H
