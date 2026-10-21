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

#include "../catch.hpp"

#include "cartocrow/reader/ipe_reader.h"
#include "cartocrow/renderer/ipe_renderer.h"

using namespace cartocrow;
using namespace renderer;

// Test case names need to be unique, so we prefix with IpeReader.
#define TEST_CASE_(name) TEST_CASE("[IpeReader] " name, "[IpeReader]")

TEST_CASE_("Reading points") {
	IpeReader ipeReader("data/test_ipe_reader.ipe");

	auto points = ipeReader.read<Multiple, Point<Inexact>, WithoutAttributes>();
	CHECK(points.size() == 4);

	auto exists = [&](Point<Inexact> point) {
		return std::find(points.begin(), points.end(), point) != points.end();
	};

	CHECK(exists({0, 0}));
	CHECK(exists({64, 64}));
	CHECK(exists({64, 0}));
	CHECK(exists({0, 64}));
}

TEST_CASE_("Reading a polygon") {
	std::vector<Point<Inexact>> points({{144.544, 155.39}, {123.907, 113.135}, {178.446, 67.44}, {249.199, 124.927}});
	Polygon<Inexact> expectedPolygon(points.begin(), points.end());

	IpeReader ipeReader("data/test_ipe_reader.ipe");
	auto parsedPolygon = ipeReader.read<Single, Polygon<Inexact>, WithoutAttributes>();
	CHECK(parsedPolygon == expectedPolygon);
}

TEST_CASE_("Reading points and a polygon") {
	std::vector<Point<Inexact>> points(
	    {{144.544, 155.39}, {123.907, 113.135}, {178.446, 67.44}, {249.199, 124.927}});
	Polygon<Inexact> expectedPolygon(points.begin(), points.end());

	using PointOrPoly = std::variant<Point<Inexact>, Polygon<Inexact>>;

	IpeReader ipeReader("data/test_ipe_reader.ipe");
	auto pointOrPolys = ipeReader.read<Multiple, PointOrPoly, WithoutAttributes>();

	CHECK(pointOrPolys.size() == 5);

	auto exists = [&](PointOrPoly&& p) {
		return std::find(pointOrPolys.begin(), pointOrPolys.end(), p) != pointOrPolys.end();
	};
	
	CHECK(exists(expectedPolygon));
	CHECK(exists(Point<Inexact>{0, 0}));
	CHECK(exists(Point<Inexact>{64, 64}));
	CHECK(exists(Point<Inexact>{64, 0}));
	CHECK(exists(Point<Inexact>{0, 64}));
}

TEST_CASE_("Reading polygon sets") {
	// Test whether polygon is automatically converted to MultipolygonWithHoles.
	// The file contains 2 polygon sets and 1 polygon.
	IpeReader ipeReader("data/test_ipe_reader.ipe");
	ipeReader.setPage(1);

	auto psrs = ipeReader.read<Multiple, MultipolygonWithHoles<Inexact>, WithoutAttributes>();
	auto pgns = ipeReader.read<Multiple, Polygon<Inexact>, WithoutAttributes>();

	CHECK(psrs.size() == 3);
	CHECK(pgns.size() == 1);
}

TEST_CASE_("Reading polylines, segments, Bézier curves and splines") {
	// Test open geometries.
	// The file contains a line segment, a polyline, a cubic Bézier curve, a cubic Bézier spline.
	IpeReader ipeReader("data/test_ipe_reader.ipe");
	ipeReader.setPage(2);

	auto ls = ipeReader.read<Multiple, Segment<Inexact>, WithoutAttributes>();
	CHECK(ls.size() == 1); // should only return the line segment

	auto pls = ipeReader.read<Multiple, Polyline<Inexact>, WithoutAttributes>();
	CHECK(pls.size() == 2); // should return the polyline and the line segment

	auto cbcs = ipeReader.read<Multiple, CubicBezierCurve, WithoutAttributes>();
	CHECK(cbcs.size() == 2); // should return the cubic Bézier curve and the line segment

	auto cbss = ipeReader.read<Multiple, CubicBezierSpline, WithoutAttributes>();
	CHECK(cbss.size() == 4); // should return all
}

TEST_CASE_("Read points from specific layer") {
	IpeReader ipeReader("data/test_ipe_reader.ipe");
	ipeReader.setPage(3);

	ipeReader.setLayerFilter("red");
	auto redPoints = ipeReader.read<Multiple, Point<Inexact>, WithoutAttributes>();
	ipeReader.setLayerFilter(0);
	auto bluePoints = ipeReader.read<Multiple, Point<Inexact>, WithoutAttributes>();
	ipeReader.setLayerFilter("green");
	auto greenPoints = ipeReader.read<Multiple, Point<Inexact>, WithoutAttributes>();

	CHECK(bluePoints.size() == 7);
	CHECK(redPoints.size() == 11);
	CHECK(greenPoints.size() == 8);
}

TEST_CASE_("Read with output iterator") {
	IpeReader ipeReader("data/test_ipe_reader.ipe");
	ipeReader.setPage(3);

	std::vector<Point<Inexact>> allPoints;
	ipeReader.setLayerFilter("red");
	ipeReader.read<Multiple, Point<Inexact>, WithoutAttributes>(std::back_inserter(allPoints));
	ipeReader.setLayerFilter(0);
	ipeReader.read<Multiple, Point<Inexact>, WithoutAttributes>(std::back_inserter(allPoints));
	ipeReader.setLayerFilter("green");
	ipeReader.read<Multiple, Point<Inexact>, WithoutAttributes>(std::back_inserter(allPoints));

	ipeReader.read<Single, Point<Inexact>, WithoutAttributes>(std::back_inserter(allPoints));

	CHECK(allPoints.size() == 27);
}

TEST_CASE_("Read layer names") {
	IpeReader ipeReader("data/test_ipe_reader.ipe");
	ipeReader.setPage(3);
	auto names = ipeReader.layerNames();
	CHECK(names == std::vector<std::string>({"blue", "red", "green"}));
}

TEST_CASE_("Read attributes") {
	IpeReader ipeReader("data/test_ipe_reader.ipe");
	ipeReader.setPage(3);

	ipeReader.setLayerFilter("red");
	auto redPoints = ipeReader.read<Multiple, Point<Inexact>, WithAttributes>();
	ipeReader.setLayerFilter(0);
	auto bluePoints = ipeReader.read<Multiple, Point<Inexact>, WithAttributes>();
	ipeReader.setLayerFilter("green");
	auto greenPoints = ipeReader.read<Multiple, Point<Inexact>, WithAttributes>();

	CHECK(bluePoints.size() == 7);
	for (const auto& bp : bluePoints) {
		CHECK(bp.attributes.contains("fill"));
		CHECK(std::holds_alternative<std::string>(bp.attributes.at("fill")));
		CHECK(std::get<std::string>(bp.attributes.at("fill")) == "CB light blue");
	}

	CHECK(redPoints.size() == 11);
	for (const auto& rp : redPoints) {
		CHECK(rp.attributes.contains("fill"));
		CHECK(std::holds_alternative<std::string>(rp.attributes.at("fill")));
		CHECK(std::get<std::string>(rp.attributes.at("fill")) == "CB light red");
	}

	CHECK(greenPoints.size() == 8);
	for (const auto& gp : greenPoints) {
		CHECK(gp.attributes.contains("fill"));
		CHECK(std::holds_alternative<std::string>(gp.attributes.at("fill")));
		CHECK(std::get<std::string>(gp.attributes.at("fill")) == "CB light green");
	}
}

TEST_CASE_("Read ellipses and circles") {
	IpeReader ipeReader("data/test_ipe_reader.ipe");
	ipeReader.setPage(4);

	// The page has 5 ellipses, three of which are circles (two are grouped), one of which is a skewed circle (the object has a transformation matrix), the last is a 'proper' ipe ellipse.
	// The two grouped circles should automatically be ungrouped.

	auto circles = ipeReader.read<Multiple, Circle<Inexact>, WithoutAttributes>();
	CHECK(circles.size() == 3);

	auto ellipses = ipeReader.read<Multiple, Ellipse, WithoutAttributes>();
	CHECK(ellipses.size() == 5);
}

//TEST_CASE_("Manual check: load and save test_ipe_reader.ipe; geometries should be equivalent") {
//	IpeReader ipeReader;
//	IpeRenderer ipeRenderer;
//	
//	auto fn = "data/test_ipe_reader.ipe";
//
//	for (int pageIndex = 0; pageIndex < ipeReader.numberOfPages(fn); ++pageIndex) {
//		ipeReader.setPage(pageIndex);
//		auto geoms = ipeReader.readV<IntermediateIpeGeometry>(fn);
//		ipeRenderer.addPainting([geoms](GeometryRenderer& r) {
//			for (const auto& g : geoms) {
//				std::visit([&](auto& someG) { r.draw(someG); }, g);
//			}
//		});
//		ipeRenderer.nextPage();
//	}
//
//	ipeRenderer.save("test_ipe_reader_saved.ipe");
//}