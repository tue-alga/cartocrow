/*
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

#include "render_path_demo.h"

#include <QApplication>
#include <QCheckBox>
#include <QDockWidget>
#include <QGridLayout>
#include <QMainWindow>
#include <QPushButton>
#include <QWidget>

#include <cartocrow/renderer/render_path.h>

using namespace cartocrow;
using namespace cartocrow::renderer;

void DemoPainting::paint(GeometryRenderer& renderer) const {
	// set style
	renderer.setStroke(Color{0, 0, 0}, 2.5);
	renderer.setFill(Color{120, 170, 240});
	renderer.setMode(GeometryRenderer::DrawMode::fill | GeometryRenderer::DrawMode::stroke);

	// construct a RenderPath with three subpaths
	RenderPath path;

	path.moveTo(Point<Inexact>(-150, -50));
	for (int i = 0; i < 4; i++) {
		double alpha = i * 2 * M_PI / 4;
		path.arcTo(Point<Inexact>(-200 + 100 * cos(alpha), 100 * sin(alpha)), true,
		           Point<Inexact>(-200 + 50 * sqrt(2) * cos(alpha + M_PI / 4),
		                          50 * sqrt(2) * sin(alpha + M_PI / 4)));
	}

	path.moveTo(Point<Inexact>(-30, 0));
	path.lineTo(Point<Inexact>(20, 50));
	path.lineTo(Point<Inexact>(40, -40));
	path.close();
	
	path.moveTo(Point<Inexact>(225, -25));
	for (int i = 0; i < 4; i++) {
		double alpha = i * 2 * M_PI / 4;
		path.arcTo(Point<Inexact>(200 + 25 * cos(alpha), 25 * sin(alpha)), false,
		           Point<Inexact>(200 + 25 * sqrt(2) * cos(alpha + M_PI / 4),
		                          25 * sqrt(2) * sin(alpha + M_PI / 4)));
	}
	path.close();

	// draw various shapes
	renderer.draw(path);
}

RenderPathDemo::RenderPathDemo() {
	setWindowTitle("CartoCrow – Render path demo");

	m_renderer = new GeometryWidget();
	setCentralWidget(m_renderer);

	std::shared_ptr<DemoPainting> painting = std::make_shared<DemoPainting>();
	m_renderer->addPainting(painting, "Demo painting");
}

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	RenderPathDemo demo;
	demo.show();
	app.exec();
}
