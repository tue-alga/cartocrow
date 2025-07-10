#include "gdal_conversion.h"

namespace cartocrow {
PolygonSet<Exact> ogrMultiPolygonToPolygonSet(const OGRMultiPolygon& multiPolygon) {
    PolygonSet<Exact> polygonSet;
    for (auto& poly: multiPolygon) {
        for (auto& linearRing: *poly) {
            auto polygon = ogrLinearRingToPolygon(*linearRing);
            if (polygon.is_clockwise_oriented()) {
                polygon.reverse_orientation();
            }
            polygonSet.symmetric_difference(polygon);
        }
    }
    return polygonSet;
}

Polygon<Exact> ogrLinearRingToPolygon(const OGRLinearRing& ogrLinearRing) {
    Polygon<Exact> polygon;
    for (auto &pt: ogrLinearRing) {
        polygon.push_back({pt.getX(), pt.getY()});
    }
    // if the begin and end vertices are equal, remove one of them
    if (polygon.container().front() == polygon.container().back()) {
        polygon.container().pop_back();
    }
    return polygon;
}

PolygonSet<Exact> ogrPolygonToPolygonSet(const OGRPolygon& ogrPolygon) {
    PolygonSet<Exact> polygonSet;
    for (auto& linearRing : ogrPolygon) {
        Polygon<Exact> polygon;
        for (auto& pt : *linearRing) {
            polygon.push_back({pt.getX(), pt.getY()});
        }
        // if the begin and end vertices are equal, remove one of them
        if (polygon.container().front() == polygon.container().back()) {
            polygon.container().pop_back();
        }
        if (polygon.is_clockwise_oriented()) {
            polygon.reverse_orientation();
        }
        polygonSet.symmetric_difference(polygon);
    }
    return polygonSet;
}

PolygonWithHoles<Exact> ogrPolygonToPolygonWithHoles(const OGRPolygon& ogrPolygon) {
    std::vector<PolygonWithHoles<Exact>> pgns;
    ogrPolygonToPolygonSet(ogrPolygon).polygons_with_holes(std::back_inserter(pgns));
    assert(pgns.size() == 1);
    return pgns.front();
}

OGRLinearRing polygonToOGRLinearRing(const Polygon<Exact>& polygon) {
    OGRLinearRing ring;
    for (const auto& v : polygon.vertices()) {
        auto v_ = approximate(v);
        ring.addPoint(v_.x(), v_.y());
    }
    auto v_ = approximate(polygon.vertices().front());
    ring.addPoint(v_.x(), v_.y());

    return ring;
}

OGRPolygon polygonWithHolesToOGRPolygon(const PolygonWithHoles<Exact>& polygon) {
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

OGRMultiPolygon polygonSetToOGRMultiPolygon(const PolygonSet<Exact>& polygonSet) {
    std::vector<PolygonWithHoles<Exact>> pgns;
    polygonSet.polygons_with_holes(std::back_inserter(pgns));

    OGRMultiPolygon ogrMultiPolygon;
    for (const auto& pgn : pgns) {
        auto ogrPolygon = polygonWithHolesToOGRPolygon(pgn);
        ogrMultiPolygon.addGeometry(&ogrPolygon);
    }

    return ogrMultiPolygon;
}
}