#include "circle_convex_hull.h"
#include <QApplication>
#include "cartocrow/simplesets/helpers/approximate_convex_hull.h"
#include "cartocrow/simplesets/helpers/cs_polygon_helpers.h"

CircleConvexHullDemo::CircleConvexHullDemo() {
	setWindowTitle("Convex hull of circles");
	auto renderer = new GeometryWidget();
	renderer->setDrawAxes(false);
	setCentralWidget(renderer);

	std::vector<Circle<Exact>> cs({
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

	auto hull = approximateConvexHull(cs);

	std::function<void(GeometryRenderer&)> drawFunc = [cs, hull](GeometryRenderer& renderer) {
		RenderPath path = renderPath(hull);
	  	renderer.setMode(GeometryRenderer::fill);
	  	renderer.setFill(Color{150, 150, 150});
		for (const auto& c : cs) {
			renderer.draw(c);
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
