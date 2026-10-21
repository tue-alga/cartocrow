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

#include "cs_polyline_helpers_demo.h"
#include "cartocrow/circle_segment_helpers/cs_polyline_helpers.h"
#include "cartocrow/circle_segment_helpers/cs_render_helpers.h"
#include "cartocrow/renderer/geometry_widget.h"
#include <QApplication>

using namespace cartocrow;

CSPolylineHelpersDemo::CSPolylineHelpersDemo() {
	setWindowTitle("Circle-segment polyline helpers demo");
	auto renderer = new GeometryWidget();
	renderer->setDrawAxes(false);
	setCentralWidget(renderer);

	Circle<Exact> circle({0, 0}, 1);
	CSXMCurve arc(circle, {-1, 0}, {1, 0}, CGAL::CLOCKWISE);
	CSXMCurve ls0({-2, 0}, {-1, 0});
	CSXMCurve ls1({1, 0}, {1, -1});
	CSXMCurve ls2({1, -1}, {2, -1});
	std::vector<CSXMCurve> curves({arc, ls1});
	CSPolyline pl(curves.begin(), curves.end());

	renderer->fitInView(Box(-3, -4, 3, 3));

	std::function<void(GeometryRenderer&)> drawFunc = [pl](GeometryRenderer& renderer) {
		renderer.setMode(GeometryRenderer::stroke);
		auto [extended, source, target] = approximateExtend(pl, 1.0, 1);
	  	auto pgn = closeAroundBB(extended, CGAL::COUNTERCLOCKWISE, 1.0, source, target);
	  	renderer.setStroke(Color{255, 0, 0}, 3.0);
		renderer.draw(renderPath(pgn));
	  	renderer.setStroke(Color{0, 0, 255}, 3.0);
	  	renderer.draw(renderPath(extended));
	  	renderer.setStroke(Color{0, 0, 0}, 3.0);
	  	renderer.draw(renderPath(pl));
	};
	renderer->addPainting(drawFunc, "Polyline");
}

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	CSPolylineHelpersDemo demo;
	demo.show();
	app.exec();
}
