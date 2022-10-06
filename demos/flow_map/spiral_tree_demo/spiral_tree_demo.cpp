/*
The Necklace Map console application implements the algorithmic
geo-visualization method by the same name, developed by
Bettina Speckmann and Kevin Verbeek at TU Eindhoven
(DOI: 10.1109/TVCG.2010.180 & 10.1142/S021819591550003X).
Copyright (C) 2021  Netherlands eScience Center and TU Eindhoven

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

#include "spiral_tree_demo.h"

#include <QApplication>

#include "cartocrow/core/core.h"
#include "cartocrow/core/timer.h"
#include "cartocrow/flow_map/painting.h"
#include "cartocrow/flow_map/parameters.h"
#include "cartocrow/flow_map/place.h"
#include "cartocrow/flow_map/spiral_tree.h"
#include "cartocrow/flow_map/spiral_tree_obstructed_algorithm.h"
#include "cartocrow/renderer/geometry_painting.h"
#include "cartocrow/renderer/geometry_widget.h"
#include "cartocrow/renderer/ipe_renderer.h"

using namespace cartocrow;
using namespace cartocrow::flow_map;
using namespace cartocrow::renderer;

SpiralTreeDemo::SpiralTreeDemo() {
	setWindowTitle("CartoCrow – Spiral tree demo");

	/*m_obstacle.push_back(Point<Inexact>(0, 50));
	m_obstacle.push_back(Point<Inexact>(25, 40));
	m_obstacle.push_back(Point<Inexact>(8, 95));
	m_obstacle.push_back(Point<Inexact>(50, 140));
	m_obstacle.push_back(Point<Inexact>(-41, 134));
	m_obstacle.push_back(Point<Inexact>(-43, 60));
	m_obstacle.push_back(Point<Inexact>(0, 70));
	m_obstacle.push_back(Point<Inexact>(-20, 20));*/

	// join not working
	/*m_obstacle.push_back(Point<Inexact>(0, 50));
	m_obstacle.push_back(Point<Inexact>(20, 80));
	m_obstacle.push_back(Point<Inexact>(-20, 70));*/

	// right vertex event bug
	/*m_obstacle.push_back(Point<Inexact>(-3, 38));
	m_obstacle.push_back(Point<Inexact>(11, 56));
	m_obstacle.push_back(Point<Inexact>(21, 79));
	m_obstacle.push_back(Point<Inexact>(15, 108));
	m_obstacle.push_back(Point<Inexact>(-57, 140));*/

	// far vertex event on near side bug
	/*m_obstacle.push_back(Point<Inexact>(-19, 59));
	m_obstacle.push_back(Point<Inexact>(9, 83));
	m_obstacle.push_back(Point<Inexact>(26, 59));
	m_obstacle.push_back(Point<Inexact>(49, 114));
	m_obstacle.push_back(Point<Inexact>(-33, 117));*/

	// another far vertex event handling bug
	/*m_obstacle.push_back(Point<Inexact>(0, 50));
	m_obstacle.push_back(Point<Inexact>(8, 95));
	m_obstacle.push_back(Point<Inexact>(50, 140));
	m_obstacle.push_back(Point<Inexact>(0, 175));
	m_obstacle.push_back(Point<Inexact>(-50, 100));*/

	// join event with obstacle edge
	/*m_obstacle.push_back(Point<Inexact>(0, 50));
	m_obstacle.push_back(Point<Inexact>(8, 95));
	m_obstacle.push_back(Point<Inexact>(50, 140));
	m_obstacle.push_back(Point<Inexact>(-37, 175));
	m_obstacle.push_back(Point<Inexact>(-50, 100));*/

	// φ = π issues
	/*m_obstacle.push_back(Point<Inexact>(-7, 19));
	m_obstacle.push_back(Point<Inexact>(-76, 30));
	m_obstacle.push_back(Point<Inexact>(-82, -47));*/

	m_obstacle.push_back(Point<Inexact>(0, 7));
	m_obstacle.push_back(Point<Inexact>(4, 11));
	m_obstacle.push_back(Point<Inexact>(-8, 8));

	m_renderer = new GeometryWidget();
	setCentralWidget(m_renderer);

	QToolBar* toolBar = new QToolBar();
	toolBar->addWidget(new QLabel("α = "));
	m_alphaSlider = new QSlider(Qt::Horizontal);
	m_alphaSlider->setMinimum(0);
	m_alphaSlider->setMaximum(499);
	m_alphaSlider->setValue(200);
	toolBar->addWidget(m_alphaSlider);
	addToolBar(toolBar);
	m_alphaLabel = new QLabel("0.200π");
	toolBar->addWidget(m_alphaLabel);
	connect(m_alphaSlider, &QSlider::valueChanged, [&](int value) {
		m_alpha = M_PI * value / 1000.0;
		m_alphaLabel->setText(QString("%1π").arg(value / 1000.0, 0, 'f', 3));
		recalculate();
	});
	connect(m_renderer, &GeometryWidget::dragStarted, [&](Point<Inexact> p) {
		m_draggedPoint = findClosestPoint(p, 10 / m_renderer->zoomFactor());
		recalculate();
	});
	connect(m_renderer, &GeometryWidget::dragMoved, [&](Point<Inexact> p) {
		if (m_draggedPoint != nullptr) {
			Point<Inexact> originalPoint = *m_draggedPoint;
			*m_draggedPoint = p;
			if (!m_obstacle.is_simple()) {
				*m_draggedPoint = originalPoint;
			}
			recalculate();
		}
	});
	connect(m_renderer, &GeometryWidget::dragEnded, [&](Point<Inexact> p) {
		m_draggedPoint = nullptr;
		recalculate();
	});
	recalculate();
}

void SpiralTreeDemo::recalculate() {
	Timer t;
	auto tree = std::make_shared<SpiralTree>(Point<Inexact>(0, 0), m_alpha);
	tree->addPlace("p1", Point<Inexact>(0, 20), 1);
	tree->addPlace("p2", Point<Inexact>(6, 7), 1);
	tree->addPlace("p3", Point<Inexact>(-18, 0), 1);
	tree->addPlace("p4", Point<Inexact>(4, -9), 1);
	tree->addPlace("p5", Point<Inexact>(-4, -11), 1);
	tree->addObstacle(m_obstacle);
	tree->addShields();
	t.stamp("Constructing tree and obstacles");
	SpiralTreeObstructedAlgorithm algorithm(tree);
	algorithm.run();
	t.stamp("Computing reachable region");

	t.output();

	Painting::Options options;
	Painting p(nullptr, tree, options);
	auto painting = std::make_shared<Painting>(nullptr, tree, options);

	m_renderer->clear();
	m_renderer->addPainting(painting);
	m_renderer->addPainting(algorithm.debugPainting());

	m_renderer->update();

	/*IpeRenderer ipeRenderer(painting);
	ipeRenderer.addPainting(algorithm.debugPainting());
	ipeRenderer.save("/tmp/spiral-tree.ipe");*/
}

Point<Inexact>* SpiralTreeDemo::findClosestPoint(Point<Inexact> p, Number<Inexact> radius) {
	Point<Inexact>* closest = nullptr;
	Number<Inexact> minSquaredDistance = radius * radius;
	for (auto& vertex : m_obstacle) {
		Number<Inexact> squaredDistance = (vertex - p).squared_length();
		if (squaredDistance < minSquaredDistance) {
			minSquaredDistance = squaredDistance;
			closest = &vertex;
		}
	}
	return closest;
}

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	SpiralTreeDemo demo;
	demo.show();
	app.exec();
}
