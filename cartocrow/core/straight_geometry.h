/*
Copyright (C) 2026  TU Eindhoven

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

#pragma once

#include "polyline.h"
#include "multi_polyline.h"
#include "multi_point.h"
#include "multipolygon_with_holes.h"

namespace cartocrow {
/// Type that serves as an internal representation of the straight (i.e. linear) features of the OGC Simple Feature Access.
/// OGC name:                         MultiPolygon      Polygon              LinearRing
template <class K, class Container = std::vector<Point<K>>>
using StraightGeometry = std::variant<MultipolygonWithHoles<K, Container>, PolygonWithHoles<K, Container>, Polygon<K, Container>,
//                                    MultiLineString LineString   Point     MultiPoint
                                      MultiPolyline<K>, Polyline<K>, Point<K>, MultiPoint<K>>;
}