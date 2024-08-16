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

#include "editables_demo.h"

#include <QApplication>
#include <QMainWindow>
#include <QWidget>

using namespace cartocrow;
using namespace cartocrow::renderer;

DemoPainting::DemoPainting(std::shared_ptr<Point<Inexact>> p1, std::shared_ptr<Point<Inexact>> p2,
                           std::shared_ptr<Point<Inexact>> p3)
    : m_p1(p1), m_p2(p2), m_p3(p3) {}

void DemoPainting::paint(GeometryRenderer& renderer) const {
	// set style
	renderer.setStroke(Color{0, 0, 0}, 2.5);
	renderer.setFill(Color{120, 170, 240});
	renderer.setMode(GeometryRenderer::DrawMode::fill | GeometryRenderer::DrawMode::stroke);

	// draw circle through the three points
	if (!CGAL::collinear(*m_p1, *m_p2, *m_p3)) {
		renderer.draw(Circle<Inexact>(*m_p1, *m_p2, *m_p3));
	}
	renderer.draw(*m_p1);
	renderer.draw(*m_p2);
	renderer.draw(*m_p3);
}

EditablesDemo::EditablesDemo() {
	setWindowTitle("CartoCrow – Editables demo");

	m_renderer = new GeometryWidget();
	setCentralWidget(m_renderer);

	auto p1 = std::make_shared<Point<Inexact>>(-40, 30);
	m_renderer->registerEditable(p1);
	auto p2 = std::make_shared<Point<Inexact>>(30, 40);
	m_renderer->registerEditable(p2);
	auto p3 = std::make_shared<Point<Inexact>>(40, -30);
	m_renderer->registerEditable(p3);

	auto painting = std::make_shared<DemoPainting>(p1, p2, p3);
	m_renderer->addPainting(painting, "Demo painting");
}

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	EditablesDemo demo;
	demo.show();
	app.exec();
}
