#include "circles_convex_hull_demo.h"
#include "cartocrow/circle_segment_helpers/approximate_convex_hull_of_disks.h"
#include "cartocrow/circle_segment_helpers/cs_render_helpers.h"
#include <QApplication>

CircleConvexHullDemo::CircleConvexHullDemo() {
	setWindowTitle("Convex hull of circles");
	auto renderer = new GeometryWidget();
	renderer->setDrawAxes(false);
	setCentralWidget(renderer);

	std::vector<Circle<Inexact>> cs({
	    {{0, 0},   2},
		{{10, 4},  12},
		{{7, -6},  8},
		{{5, -8},  1},
		{{3, 3},   3},
		{{15, -4}, 9},
		{{5, -4},  8},
		{{0, -1},  5},
		{{5, -3},  12},
		{{8, -9},  16},
	});

	renderer->fitInView(Box(-5, -15, 20, 10));

	std::vector<std::shared_ptr<Circle<Inexact>>> circlePtrs;
	for (const auto& c : cs) {
		circlePtrs.push_back(std::make_shared<Circle<Inexact>>(c));
		renderer->registerEditable(circlePtrs.back());
	}

	std::function<void(GeometryRenderer&)> drawFunc = [circlePtrs](GeometryRenderer& renderer) {
		std::vector<RationalRadiusCircle> cs;
		for (const auto& c : circlePtrs) {
			cs.emplace_back(pretendExact(c->center()), sqrt(c->squared_radius()));
		}
		auto hull = approximateConvexHull(cs);

		RenderPath path = renderPath(hull);
	  	renderer.setMode(GeometryRenderer::fill);
	  	renderer.setFill(Color{150, 150, 150});
		for (const auto& c : circlePtrs) {
			renderer.draw(*c);
		}
	  	for (const auto& c : circlePtrs) {
			renderer.draw(c->center());
	  	}
	    renderer.setMode(GeometryRenderer::stroke);
	    renderer.setStroke(Color{0, 0, 0}, 3.0);
	    renderer.draw(path);
	};
	renderer->addPainting(drawFunc, "Disks");
}

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	CircleConvexHullDemo demo;
	demo.show();
	app.exec();
}
