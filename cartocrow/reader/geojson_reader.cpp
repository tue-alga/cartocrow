#include "geojson_reader.h"

namespace cartocrow {
Point<Inexact> parsePoint(const JSONObject& o) {
	return {o[0], o[1]};
}

MultiPoint<Inexact> parseMultiPoint(const JSONObject& o) {
	MultiPoint<Inexact> pointSet;
	for (const auto& ptO : o) {
		pointSet.points.push_back(parsePoint(ptO));
	}
	return pointSet;
}

Polyline<Inexact> parseLineString(const JSONObject& o) {
	Polyline<Inexact> polyline;
	for (const auto& ptO : o) {
		polyline.push_back(parsePoint(ptO));
	}
	return polyline;
}

MultiPolyline<Inexact> parseMultiLineString(const JSONObject& o) {
	MultiPolyline<Inexact> ps;
	for (const auto& lsO : o) {
		ps.polylines.push_back(parseLineString(lsO));
	}
	return ps;
}

Polygon<Inexact> parseLinearRing(const JSONObject& o) {
	Polygon<Inexact> polygon;
	for (const auto& ptO : o) {
		polygon.push_back(parsePoint(ptO));
	}
	// if the begin and end vertices are equal, remove one of them
	if (polygon.container().front() == polygon.container().back()) {
		polygon.container().pop_back();
	}
	return polygon;
}

PolygonWithHoles<Inexact> parsePolygon(const JSONObject& o) {
	PolygonWithHoles<Inexact> polygonWH;
	for (const auto& lrO : o) {
		auto polygon = parseLinearRing(lrO);
		if (polygonWH.is_empty()) {
			polygonWH.outer_boundary() = polygon;
		} else {
			polygonWH.add_hole(polygon);
		}
	}
	return polygonWH;
}

MultipolygonWithHoles<Inexact> parseMultiPolygon(const JSONObject& o) {
	MultipolygonWithHoles<Inexact> ps;
	for (const auto& pO : o) {
		ps.add_polygon_with_holes(parsePolygon(pO));
	}
	return ps;
}
}
