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

#include <ogrsf_frmts.h>
#include "cartocrow/core/core.h"
#include "cartocrow/core/polyline.h"
#include "cartocrow/core/multi_polyline.h"
#include "cartocrow/core/multipolygon_with_holes.h"
#include "cartocrow/core/multi_point.h"
#include "cartocrow/core/straight_geometry.h"

namespace cartocrow {
StraightGeometry<Inexact> ogrGeometryToStraightGeometry(const OGRGeometry& geometry);
MultipolygonWithHoles<Inexact> ogrMultiPolygonToPolygonSetRaw(const OGRMultiPolygon& multiPolygon);
MultipolygonWithHoles<Inexact> ogrPolygonToPolygonSetRaw(const OGRPolygon& ogrPolygon);
Polygon<Inexact> ogrLinearRingToPolygon(const OGRLinearRing& ogrLinearRing);
MultiPolyline<Inexact> ogrMultiLineStringToPolylineSet(const OGRMultiLineString& ogrMultiLineString);
Polyline<Inexact> ogrLineStringToPolyline(const OGRLineString& ogrLineString);
PolygonWithHoles<Inexact> ogrPolygonToPolygonWithHoles(const OGRPolygon& ogrPolygon);
MultiPoint<Inexact> ogrMultiPointToPointSet(const OGRMultiPoint& ogrMultiPoint);
Point<Inexact> ogrPointToPoint(const OGRPoint& ogrPoint);
OGRLinearRing polygonToOGRLinearRing(const Polygon<Inexact>& polygon);
OGRPolygon polygonWithHolesToOGRPolygon(const PolygonWithHoles<Inexact>& polygon);
OGRMultiPolygon polygonSetToOGRMultiPolygon(const PolygonSet<Inexact>& polygonSet);
OGRLinearRing polygonToOGRLinearRing(const Polygon<Exact>& polygon);
OGRPolygon polygonWithHolesToOGRPolygon(const PolygonWithHoles<Exact>& polygon);
OGRMultiPolygon polygonSetToOGRMultiPolygon(const PolygonSet<Exact>& polygonSet);
}
