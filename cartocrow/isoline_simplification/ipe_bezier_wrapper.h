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

#ifndef CARTOCROW_IPE_BEZIER_WRAPPER_H
#define CARTOCROW_IPE_BEZIER_WRAPPER_H

#include "ipeshape.h"
#include "ipegeo.h"
#include "types.h"
#include "../core/bezier.h"

namespace cartocrow::isoline_simplification {
std::vector<Point<K>> parabola_intersections(Segment<K> seg, Line<K> dir,
                                                Point<K> focus, Point<K> p1, Point<K> p2);
ipe::Vector pv(Point<K> p);
Point<K> vp(ipe::Vector p);

BezierCurve parse_ipe_bezier(const ipe::Bezier& bz);
BezierSpline parse_ipe_beziers(const std::vector<ipe::Bezier>& bzs);
}
#endif //CARTOCROW_IPE_BEZIER_WRAPPER_H
