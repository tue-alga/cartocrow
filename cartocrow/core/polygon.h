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

#ifndef CARTOCROW_CORE_POLYGON_H
#define CARTOCROW_CORE_POLYGON_H

#include "core.h"

namespace cartocrow {

/// Computes the centroid of the given polygon.
///
/// This method throws if the polygon has area 0, in which case the centroid
/// would be ill-defined.
Point<Exact> centroid(const Polygon<Exact>& polygon);

/// Computes the centroid of the given polygon with holes.
Point<Exact> centroid(const PolygonWithHoles<Exact>& polygon);

} // namespace cartocrow

#endif //CARTOCROW_CORE_POLYGON_H
