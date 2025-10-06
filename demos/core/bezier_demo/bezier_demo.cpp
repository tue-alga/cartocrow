#include "bezier_demo.h"

#include <QApplication>

#include "cartocrow/core/core.h"
#include "cartocrow/core/cubic_bezier.h"

BezierDemo::BezierDemo() {
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

	// Curve control points
	auto c0 = std::make_shared<Point<Inexact>>(0, 0);
	m_renderer->registerEditable(c0);
	auto c1 = std::make_shared<Point<Inexact>>(1, 2);
	m_renderer->registerEditable(c1);
	auto c2 = std::make_shared<Point<Inexact>>(2, -1);
	m_renderer->registerEditable(c2);
	auto c3 = std::make_shared<Point<Inexact>>(3, 0);
	// Below is the control point for which the CGAL insert in arrangement crashes.
	// (one also needs to uncomment the intersection code below)
//	auto c3 = std::make_shared<Point<Inexact>>(-0.19959947101662578, 0.20031960128096571);
	m_renderer->registerEditable(c3);

	m_renderer->addPainting([p1, p2, c0, c1, c2, c3](GeometryRenderer& renderer) {
		// Define segment, cubic Bézier curve and its extrema, bounding box and inflection points
	  	Segment<Inexact> seg(*p1, *p2);
	  	CubicBezierCurve curve(*c0, *c1, *c2, *c3);
		auto [left, bottom, right, top] = curve.extrema();
		Box box = curve.bbox();
		std::vector<CubicBezierCurve::CurvePoint> inflects;
		curve.inflections(std::back_inserter(inflects));

		// Draw curvature lines of the curve
	    renderer.setStroke(Color(155, 50, 255), 1.0);
	    for (int i = 0; i <= 200; ++i) {
	  	    double t = i / 200.0;
	  	    auto n = curve.normal(t);
	  	    n /= sqrt(n.squared_length());
	  	    auto p = curve.position(t);
	  	    renderer.draw(Segment<Inexact>(p, p + n * curve.curvature(t) / 5));
	    }

		// Draw bounding box of the curve
		renderer.setMode(GeometryRenderer::stroke | GeometryRenderer::fill);
	  	renderer.setStroke(Color(0, 120, 215), 1.0);
	  	renderer.setFill(Color(0, 120, 215));
		renderer.setFillOpacity(5);
		renderer.draw(box);
	  	renderer.setFillOpacity(255);

	  	// Draw the curve itself and the segment
	  	renderer.setMode(GeometryRenderer::stroke);
	    renderer.setStroke(Color(0, 0, 0), 3.0);
		renderer.draw(curve);
		renderer.draw(seg);
		renderer.draw(*p1);
	  	renderer.draw(*p2);
	  	renderer.draw(*c0);
	  	renderer.draw(*c3);

	  	// Draw the two control points in grey
	  	renderer.setStroke(Color(200, 200, 200), 3.0);
	  	renderer.draw(*c1);
	  	renderer.draw(*c2);

	  	// Draw the extrema
	  	renderer.setStroke(Color(0, 120, 215), 1.0);
	  	renderer.draw(left.point);
	  	renderer.draw(bottom.point);
	  	renderer.draw(right.point);
	  	renderer.draw(top.point);

	  	// Draw the inflection points
	  	renderer.setStroke(Color(155, 50, 255), 1.0);
		for (const auto& inflect : inflects) {
			renderer.draw(inflect.point);
		}

		// Draw intersections of curve with line segment
	  	renderer.setStroke(Color(200, 0, 0), 1.0);
	  	std::vector<CubicBezierCurve::CurvePoint> inters;
	  	curve.intersections(seg, std::back_inserter(inters));
	  	for (const auto& inter : inters) {
			  renderer.draw(inter.point);
	  	}

		// Intersections with other curves via CGAL crashes sometimes...
//	  	renderer.setStroke(Color(0, 0, 0), 3.0);
//	  	CubicBezierCurve other({-2, 6}, {4, 2}, {2, -3});
//	  	renderer.draw(other);
//		std::vector<Point<Inexact>> cbInters;
//		curve.intersections(other, std::back_inserter(cbInters));
//	  	for (const auto& inter : cbInters) {
//			std::cout << inter << std::endl;
//			renderer.draw(inter);
//	  	}
	}, "Bézier curve");
}

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	BezierDemo demo;
	demo.show();
	app.exec();
}
