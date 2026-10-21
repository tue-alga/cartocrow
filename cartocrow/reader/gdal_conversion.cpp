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

#include "gdal_conversion.h"

namespace cartocrow {
StraightGeometry<Inexact> ogrGeometryToStraightGeometry(const OGRGeometry& geometry) {
    switch (wkbFlatten(geometry.getGeometryType())) {
        case wkbMultiPolygon: {
            const OGRMultiPolygon *poMultiPolygon = geometry.toMultiPolygon();
            return ogrMultiPolygonToPolygonSetRaw(*poMultiPolygon);
        }
        case wkbPolygon: {
            const OGRPolygon *poly = geometry.toPolygon();
            return ogrPolygonToPolygonWithHoles(*poly);
        }
        case wkbLinearRing: {
            const OGRLinearRing *poly = geometry.toLinearRing();
            return ogrLinearRingToPolygon(*poly);
        }
        case wkbLineString: {
            const OGRLineString *pl = geometry.toLineString();
            return ogrLineStringToPolyline(*pl);
        }
        case wkbMultiLineString: {
            const OGRMultiLineString *pl = geometry.toMultiLineString();
            return ogrMultiLineStringToPolylineSet(*pl);
        }
        case wkbPoint: {
            const OGRPoint* p = geometry.toPoint();
            return ogrPointToPoint(*p);
        }
        case wkbMultiPoint: {
            const OGRMultiPoint* mp = geometry.toMultiPoint();
            return ogrMultiPointToPointSet(*mp);
        }
        default: throw std::runtime_error("Unhandled geometry type: " + std::string(geometry.getGeometryName()));
    }
}

MultipolygonWithHoles<Inexact> ogrMultiPolygonToPolygonSetRaw(const OGRMultiPolygon& multiPolygon) {
	MultipolygonWithHoles<Inexact> polygonSet;
    for (const auto& poly : multiPolygon) {
		auto pgnWH = ogrPolygonToPolygonWithHoles(*poly);
		polygonSet.add_polygon_with_holes(pgnWH);
    }
    return polygonSet;
}

Polygon<Inexact> ogrLinearRingToPolygon(const OGRLinearRing& ogrLinearRing) {
    Polygon<Inexact> polygon;
    for (auto &pt: ogrLinearRing) {
        polygon.push_back({pt.getX(), pt.getY()});
    }
    // if the begin and end vertices are equal, remove one of them
    if (polygon.container().front() == polygon.container().back()) {
        polygon.container().pop_back();
    }
    return polygon;
}

MultipolygonWithHoles<Inexact> ogrPolygonToPolygonSetRaw(const OGRPolygon& ogrPolygon) {
	MultipolygonWithHoles<Inexact> polygonSet;
	auto polygon = ogrPolygonToPolygonWithHoles(ogrPolygon);
	polygonSet.add_polygon_with_holes(std::move(polygon));
    return polygonSet;
}

PolygonWithHoles<Inexact> ogrPolygonToPolygonWithHoles(const OGRPolygon& ogrPolygon) {
	auto outer = ogrLinearRingToPolygon(*ogrPolygon.getExteriorRing());
	if (!outer.is_counterclockwise_oriented()) {
		outer.reverse_orientation();
    }
	std::vector<Polygon<Inexact>> holes;
	for (int i = 0; i < ogrPolygon.getNumInteriorRings(); ++i) {
		holes.push_back(ogrLinearRingToPolygon(*ogrPolygon.getInteriorRing(i)));
		if (!holes.back().is_clockwise_oriented()) {
			holes.back().reverse_orientation();
		}
    }
	return {outer, holes.begin(), holes.end()};
}

MultiPolyline<Inexact> ogrMultiLineStringToPolylineSet(const OGRMultiLineString& ogrMultiLineString) {
	MultiPolyline<Inexact> polylineSet;

	for (const auto& lineString : ogrMultiLineString) {
		polylineSet.polylines.push_back(ogrLineStringToPolyline(*lineString));
	}

	return polylineSet;
}

Polyline<Inexact> ogrLineStringToPolyline(const OGRLineString& ogrLineString) {
	Polyline<Inexact> pl;

	for (const auto& pt : ogrLineString) {
		pl.push_back({pt.getX(), pt.getY()});
	}

	return pl;
}

Point<Inexact> ogrPointToPoint(const OGRPoint& ogrPoint) {
	return {ogrPoint.getX(), ogrPoint.getY()};
}

MultiPoint<Inexact> ogrMultiPointToPointSet(const OGRMultiPoint& ogrMultiPoint) {
	std::vector<Point<Inexact>> pts;
	for (const OGRPoint* p : ogrMultiPoint) {
		pts.emplace_back(p->getX(), p->getY());
	}
	return {pts};
}

OGRLinearRing polygonToOGRLinearRing(const Polygon<Inexact>& polygon) {
    OGRLinearRing ring;
    for (const auto& v : polygon.vertices()) {
        auto v_ = v;
        ring.addPoint(v_.x(), v_.y());
    }
    auto v_ = polygon.vertices().front();
    ring.addPoint(v_.x(), v_.y());

    return ring;
}

OGRPolygon polygonWithHolesToOGRPolygon(const PolygonWithHoles<Inexact>& polygon) {
    assert(!polygon.is_unbounded());
    auto outerRing = polygonToOGRLinearRing(polygon.outer_boundary());
    std::vector<OGRLinearRing> holeRings;
    for (const auto& h : polygon.holes()) {
        holeRings.push_back(polygonToOGRLinearRing(h));
    }
    OGRPolygon ogrPolygon;
    ogrPolygon.addRing(&outerRing);
    for (auto& hr : holeRings) {
        ogrPolygon.addRing(&hr);
    }
    return ogrPolygon;
}

OGRMultiPolygon polygonSetToOGRMultiPolygon(const PolygonSet<Inexact>& polygonSet) {
    std::vector<PolygonWithHoles<Inexact>> pgns;
    polygonSet.polygons_with_holes(std::back_inserter(pgns));

    OGRMultiPolygon ogrMultiPolygon;
    for (const auto& pgn : pgns) {
        auto ogrPolygon = polygonWithHolesToOGRPolygon(pgn);
        ogrMultiPolygon.addGeometry(&ogrPolygon);
    }

    return ogrMultiPolygon;
}

OGRLinearRing polygonToOGRLinearRing(const Polygon<Exact>& polygon) {
	return polygonToOGRLinearRing(approximate(polygon));
}

OGRPolygon polygonWithHolesToOGRPolygon(const PolygonWithHoles<Exact>& polygon) {
	return polygonWithHolesToOGRPolygon(approximate(polygon));
}

OGRMultiPolygon polygonSetToOGRMultiPolygon(const PolygonSet<Exact>& polygonSet) {
	return polygonSetToOGRMultiPolygon(approximate(polygonSet));
}
}