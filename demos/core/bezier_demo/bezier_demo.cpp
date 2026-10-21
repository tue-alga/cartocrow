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

#include "bezier_demo.h"

#include <QApplication>

#include "cartocrow/core/core.h"
#include "cartocrow/core/cubic_bezier.h"

BezierDemo::BezierDemo() {
	CubicBezierSpline splinee;
    splinee.appendCurve({0, 0}, {1, 0}, {2, 0}, {3, 0});
    splinee.appendCurve({3, 0}, {4, 0}, {5, 0}, {6, 0});
    auto [part1, part2] = splinee.split(0.5);

	setWindowTitle("Bézier demo");

	m_renderer = new GeometryWidget();
	m_renderer->setDrawAxes(false);
	m_renderer->setMinZoom(50.0);
	m_renderer->setMaxZoom(10000.0);
	m_renderer->fitInView(Box(-1, -3, 4, 4));
	setCentralWidget(m_renderer);

	// Segment endpoints
	auto p1 = std::make_shared<Point<Inexact>>(-0.5, 0.3);
	m_renderer->registerEditable(p1);
	auto p2 = std::make_shared<Point<Inexact>>(3.5, 0.3);
	m_renderer->registerEditable(p2);

	// Spline control points
	auto c0 = std::make_shared<Point<Inexact>>(0, 0);
	m_renderer->registerEditable(c0);
	auto c1 = std::make_shared<Point<Inexact>>(1, 0);
	m_renderer->registerEditable(c1);
	auto c2 = std::make_shared<Point<Inexact>>(1, 3);
	m_renderer->registerEditable(c2);
	auto c3 = std::make_shared<Point<Inexact>>(1.5, 1);
	m_renderer->registerEditable(c3);
	auto c4 = std::make_shared<Point<Inexact>>(2, -1);
	m_renderer->registerEditable(c4);
	auto c5 = std::make_shared<Point<Inexact>>(2.5, -0.8);
	m_renderer->registerEditable(c5);
	auto c6 = std::make_shared<Point<Inexact>>(3, 0);
	m_renderer->registerEditable(c6);

	// Curve control points
	auto d0 = std::make_shared<Point<Inexact>>(-1.5, 2);
	m_renderer->registerEditable(d0);
	auto d1 = std::make_shared<Point<Inexact>>(2, 3.0);
	m_renderer->registerEditable(d1);
	auto d2 = std::make_shared<Point<Inexact>>(2, 0.0);
	m_renderer->registerEditable(d2);
	auto d3 = std::make_shared<Point<Inexact>>(1.5, -1);
	m_renderer->registerEditable(d3);

	m_renderer->addPainting([p1, p2, c0, c1, c2, c3, c4, c5, c6, this](GeometryRenderer& renderer) {
		// Define segment, cubic Bézier spline and its extrema, bounding box and inflection points
	  	Segment<Inexact> seg(*p1, *p2);
	  	CubicBezierCurve curve1(*c0, *c1, *c2, *c3);
	  	CubicBezierCurve curve2(*c3, *c4, *c5, *c6);
		CubicBezierSpline spline;
		spline.appendCurve(curve1);
	  	spline.appendCurve(curve2);

		// Draw curvature lines of the curve
	    renderer.setStroke(Color(155, 50, 255), 1.0);
		for (int curveIndex = 0; curveIndex < spline.numCurves(); ++curveIndex) {
			for (int i = 0; i <= 200; ++i) {
				double t = i / 200.0;
				auto n = spline.normal({curveIndex, t});
				n /= sqrt(n.squared_length());
				auto p = spline.position({curveIndex, t});
				renderer.draw(Segment<Inexact>(p, p + n * spline.curvature({curveIndex, t}) / 5));
			}
		}
	}, "Curvature");

	m_renderer->addPainting([c0, c1, c2, c3, c4, c5, c6, this](GeometryRenderer& renderer) {
		CubicBezierCurve curve1(*c0, *c1, *c2, *c3);
		CubicBezierCurve curve2(*c3, *c4, *c5, *c6);
		CubicBezierSpline spline;
		spline.appendCurve(curve1);
		spline.appendCurve(curve2);

		Box box = spline.bbox();
		// Draw bounding box of the spline
		renderer.setMode(GeometryRenderer::stroke | GeometryRenderer::fill);
	  	renderer.setStroke(Color(0, 120, 215), 1.0);
	  	renderer.setFill(Color(0, 120, 215));
		renderer.setFillOpacity(5);
		renderer.draw(box);
	  	renderer.setFillOpacity(255);
	}, "Bounding box");

	m_renderer->addPainting([p1, p2, c0, c1, c2, c3, c4, c5, c6, this](GeometryRenderer& renderer) {
		Segment<Inexact> seg(*p1, *p2);
		CubicBezierCurve curve1(*c0, *c1, *c2, *c3);
		CubicBezierCurve curve2(*c3, *c4, *c5, *c6);
		CubicBezierSpline spline;
		spline.appendCurve(curve1);
		spline.appendCurve(curve2);

		renderer.setStroke(Color(200, 200, 200), 1.0);
		Polyline<Inexact> pl;
		pl.push_back(*c0);
		pl.push_back(*c1);
		pl.push_back(*c2);
		pl.push_back(*c3);
		pl.push_back(*c4);
		pl.push_back(*c5);
		pl.push_back(*c6);
		renderer.draw(pl);

	  	// Draw the spline itself and the segment
	  	renderer.setMode(GeometryRenderer::stroke);
	    renderer.setStroke(Color(0, 0, 0), 3.0);
	  	if (spline.selfIntersects()) {
			  renderer.setStroke(Color(0, 0, 255), 3.0);
	  	}
		renderer.draw(spline);
	    renderer.setStroke(Color(0, 0, 0), 3.0);
	  	renderer.draw(*c0);
	  	renderer.draw(*c3);
	    renderer.draw(*c6);

	  	// Draw the control points in grey
	  	renderer.setStroke(Color(200, 200, 200), 3.0);
	  	renderer.draw(*c1);
	  	renderer.draw(*c2);
	  	renderer.draw(*c4);
	  	renderer.draw(*c5);
	}, "Spline");

	m_renderer->addPainting([c0, c1, c2, c3, c4, c5, c6, this](GeometryRenderer& renderer) {
		CubicBezierCurve curve1(*c0, *c1, *c2, *c3);
		CubicBezierCurve curve2(*c3, *c4, *c5, *c6);
		CubicBezierSpline spline;
		spline.appendCurve(curve1);
		spline.appendCurve(curve2);
		auto [left, bottom, right, top] = spline.extrema();
	  	// Draw the extrema
	  	renderer.setStroke(Color(0, 120, 215), 1.0);
	  	renderer.draw(left.point);
	  	renderer.draw(bottom.point);
	  	renderer.draw(right.point);
	  	renderer.draw(top.point);
	}, "Extrema");

	m_renderer->addPainting([c0, c1, c2, c3, c4, c5, c6, this](GeometryRenderer& renderer) {
		CubicBezierCurve curve1(*c0, *c1, *c2, *c3);
		CubicBezierCurve curve2(*c3, *c4, *c5, *c6);
		CubicBezierSpline spline;
		spline.appendCurve(curve1);
		spline.appendCurve(curve2);
	  	// Draw the inflection points
		std::vector<CubicBezierSpline::SplinePoint> inflects;
		spline.inflections(std::back_inserter(inflects));
	  	renderer.setStroke(Color(155, 50, 255), 1.0);
		for (const auto& inflect : inflects) {
			renderer.draw(inflect.point);
		}
	}, "Inflections");

	m_renderer->addPainting([p1, p2, this, c0, c1, c2, c3, c4, c5, c6](GeometryRenderer& renderer) {
		CubicBezierCurve curve1(*c0, *c1, *c2, *c3);
		CubicBezierCurve curve2(*c3, *c4, *c5, *c6);
		CubicBezierSpline spline;
		spline.appendCurve(curve1);
		spline.appendCurve(curve2);

		Segment<Inexact> seg(*p1, *p2);

		renderer.setStroke(Color(0, 0, 0), 3.0);
		renderer.draw(seg);
		renderer.draw(*p1);
		renderer.draw(*p2);

		// Draw intersections of spline with line segment
	  	renderer.setStroke(Color(200, 0, 0), 1.0);
	  	std::vector<CubicBezierSpline::SplinePoint> inters;
	  	spline.intersections(seg, std::back_inserter(inters));
	  	for (const auto& inter : inters) {
			  renderer.draw(inter.point);
	  	}
	}, "Line segment intersection");

	m_renderer->addPainting([this, c0, c1, c2, c3, c4, c5, c6](GeometryRenderer& renderer) {
		CubicBezierCurve curve1(*c0, *c1, *c2, *c3);
		CubicBezierCurve curve2(*c3, *c4, *c5, *c6);
		CubicBezierSpline spline;
		spline.appendCurve(curve1);
		spline.appendCurve(curve2);

		// Draw nearest to mouse
		renderer.setStroke(Color(0, 200, 0), 1.0);
		auto mp = m_renderer->mousePosition();
		auto closest = spline.nearest(mp);
		renderer.draw(closest.point);
		renderer.draw(Segment<Inexact>(closest.point, mp));
		renderer.draw(spline.position(closest.param));
		renderer.draw(Segment<Inexact>(spline.position(closest.param), mp));
	}, "Nearest");

	m_renderer->addPainting([d0, d1, d2, d3, c0, c1, c2, c3, c4, c5, c6, this](GeometryRenderer& renderer) {
		CubicBezierCurve curve1(*c0, *c1, *c2, *c3);
		CubicBezierCurve curve2(*c3, *c4, *c5, *c6);
		CubicBezierSpline spline;
		spline.appendCurve(curve1);
		spline.appendCurve(curve2);

		renderer.setStroke(Color(200, 200, 200), 1.0);
		Polyline<Inexact> pl;
		pl.push_back(*d0);
		pl.push_back(*d1);
		pl.push_back(*d2);
		pl.push_back(*d3);
		renderer.draw(pl);

		CubicBezierCurve otherCurve(*d0, *d1, *d2, *d3);
		if (spline.intersects(otherCurve)) {
			renderer.setStroke(Color(255, 0, 0), 3.0);
		} else {
			renderer.setStroke(Color(0, 0, 0), 3.0);
		}
		if (otherCurve.selfIntersects()) {
			renderer.setStroke(Color(0, 0, 255), 3.0);
		}
		renderer.draw(otherCurve);

		std::vector<CubicBezierIntersectionResult<CubicBezierSpline, CubicBezierCurve>> intersections;
		spline.intersections(otherCurve, std::back_inserter(intersections));

		for (const auto& result : intersections) {
			renderer.draw(result.point);
		}
		
	  	renderer.setStroke(Color(200, 200, 200), 3.0);
		renderer.draw(*d0);
	    renderer.draw(*d1);
	    renderer.draw(*d2);
	    renderer.draw(*d3);
	}, "Bézier curve intersection");

	m_renderer->addPainting([this](GeometryRenderer& renderer) { 
		CubicBezierSpline s;
		s.appendCurve({4, 2}, {4, 2.5}, {4.5, 3}, {5, 3});
		s.appendCurve({5, 3}, {5.5, 3}, {6, 2.5}, {6, 2});
		s.appendCurve({6, 2}, {6, 1.5}, {5.5, 1}, {5, 1});
		s.appendCurve({5, 1}, {4.5, 1}, {4, 1.5}, {4, 2});

		auto mp = m_renderer->mousePosition();
		auto color = s.boundedSide(mp) == CGAL::ON_BOUNDED_SIDE ? Color(255, 0, 255).shaded(1.5)
		                                                        : Color(200, 200, 200);
		renderer.setMode(GeometryRenderer::fill | GeometryRenderer::stroke);
		renderer.setFill(color);
		renderer.setStroke(Color(0, 0, 0), 3.0);
		renderer.draw(s);
	}, "Bounded side");
}

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	BezierDemo demo;
	demo.show();
	app.exec();
}
