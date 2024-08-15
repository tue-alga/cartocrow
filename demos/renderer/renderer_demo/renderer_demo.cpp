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

#include "renderer_demo.h"

#include <QApplication>
#include <QCheckBox>
#include <QDockWidget>
#include <QGridLayout>
#include <QMainWindow>
#include <QPushButton>
#include <QWidget>

using namespace cartocrow;
using namespace cartocrow::renderer;

void DemoPainting::paint(GeometryRenderer& renderer) const {
	// set style
	renderer.setStroke(Color{0, 0, 0}, 2.5);
	renderer.setFill(Color{120, 170, 240});
	renderer.setMode(GeometryRenderer::DrawMode::fill | GeometryRenderer::DrawMode::stroke);

	// draw various shapes
	renderer.draw(Point<Inexact>(-200, 0));
	renderer.drawText(Point<Inexact>(-200, -30), "Point");

	renderer.draw(Line<Inexact>(Point<Inexact>(-120, -20), Point<Inexact>(-80, 20)));
	renderer.drawText(Point<Inexact>(-100, -30), "Line");

	renderer.draw(Segment<Inexact>(Point<Inexact>(-20, -20), Point<Inexact>(20, 20)));
	renderer.drawText(Point<Inexact>(0, -30), "Segment");

	renderer.draw(Circle<Inexact>(Point<Inexact>(100, 0), 20 * 20));
	renderer.drawText(Point<Inexact>(100, -30), "Circle");

	Polygon<Inexact> polygon;
	polygon.push_back(Point<Inexact>(180, -18));
	polygon.push_back(Point<Inexact>(220, -15));
	polygon.push_back(Point<Inexact>(195, 20));
	renderer.draw(polygon);
	renderer.drawText(Point<Inexact>(200, -30), "Polygon");
}

RendererDemo::RendererDemo() {
	setWindowTitle("CartoCrow – Renderer demo");

	m_renderer = new GeometryWidget();
	m_renderer->setDrawAxes(false);
	setCentralWidget(m_renderer);

	QDockWidget* dockWidget = new QDockWidget();
	dockWidget->setWindowTitle("Settings");
	QWidget* settingsWidget = new QWidget(dockWidget);
	QGridLayout* settingsLayout = new QGridLayout(settingsWidget);
	settingsLayout->setAlignment(Qt::AlignTop);
	dockWidget->setWidget(settingsWidget);
	addDockWidget(Qt::RightDockWidgetArea, dockWidget);

	QCheckBox* axesBox = new QCheckBox("Draw axes");
	connect(axesBox, &QCheckBox::stateChanged, [&](int state) {
		m_renderer->setDrawAxes(state == Qt::CheckState::Checked);
	});
	settingsLayout->addWidget(axesBox, 0, 0);

	QCheckBox* polarBox = new QCheckBox("Polar coordinates");
	connect(polarBox, &QCheckBox::stateChanged, [&](int state) {
		if (state == Qt::CheckState::Checked) {
			m_renderer->setGridMode(GeometryWidget::GridMode::POLAR);
		} else {
			m_renderer->setGridMode(GeometryWidget::GridMode::CARTESIAN);
		}
	});
	settingsLayout->addWidget(polarBox, 1, 0);

	std::shared_ptr<DemoPainting> painting = std::make_shared<DemoPainting>();
	m_renderer->addPainting(painting, "Demo painting");

	QPushButton* zoomButton = new QPushButton("Zoom to shapes");
	connect(zoomButton, &QPushButton::clicked, [&]() {
		m_renderer->fitInView(Box(-250, -50, 250, 50));
	});
	settingsLayout->addWidget(zoomButton, 2, 0);
}

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	RendererDemo demo;
	demo.show();
	app.exec();
}
