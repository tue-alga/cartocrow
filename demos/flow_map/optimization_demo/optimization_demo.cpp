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

#include "optimization_demo.h"

#include <QApplication>
#include <QCheckBox>

#include "cartocrow/core/core.h"
#include "cartocrow/flow_map/painting.h"
#include "cartocrow/flow_map/parameters.h"
#include "cartocrow/flow_map/place.h"
#include "cartocrow/flow_map/reachable_region_algorithm.h"
#include "cartocrow/flow_map/smooth_tree_painting.h"
#include "cartocrow/flow_map/spiral_tree.h"
#include "cartocrow/flow_map/spiral_tree_obstructed_algorithm.h"
#include "cartocrow/flow_map/spiral_tree_unobstructed_algorithm.h"
#include "cartocrow/renderer/geometry_painting.h"
#include "cartocrow/renderer/geometry_widget.h"

using namespace cartocrow;
using namespace cartocrow::flow_map;
using namespace cartocrow::renderer;

OptimizationDemo::OptimizationDemo() {
	setWindowTitle("CartoCrow – Optimization demo");

	m_places.push_back(std::make_shared<Point<Inexact>>(11.2121212, 17.0707070));
	m_places.push_back(std::make_shared<Point<Inexact>>(13.9393939, -14.1414141));
	m_places.push_back(std::make_shared<Point<Inexact>>(-4.5454545, -18.9898989));
	m_places.push_back(std::make_shared<Point<Inexact>>(16.6666666, 6.1616161));
	m_places.push_back(std::make_shared<Point<Inexact>>(-9.8989898, 13.9393939));
	m_places.push_back(std::make_shared<Point<Inexact>>(-16.1616161, -2.6262626));

	m_renderer = new GeometryWidget();
	m_renderer->setMaxZoom(10000);
	m_renderer->setGridMode(GeometryWidget::GridMode::POLAR);
	setCentralWidget(m_renderer);

	QToolBar* toolBar = new QToolBar();
	toolBar->addSeparator();
	m_optimizeButton = new QPushButton("Optimize");
	m_optimizeButton->setCheckable(true);
	toolBar->addWidget(m_optimizeButton);
	m_optimizeTimer = new QTimer();
	connect(m_optimizeButton, &QPushButton::clicked, [&]() {
		if (m_optimizeButton->isChecked()) {
			m_optimizeTimer->start(0);
		} else {
			m_optimizeTimer->stop();
		}
	});
	connect(m_optimizeTimer, &QTimer::timeout, [&]() {
		m_smoothTree->optimize();
		m_renderer->update();
	});
	toolBar->addSeparator();
	toolBar->addWidget(new QLabel("α = "));
	m_alphaSlider = new QSlider(Qt::Horizontal);
	m_alphaSlider->setMinimum(0);
	m_alphaSlider->setMaximum(499);
	m_alphaSlider->setValue(139);
	toolBar->addWidget(m_alphaSlider);
	addToolBar(toolBar);
	m_alphaLabel = new QLabel("0.139π");
	toolBar->addWidget(m_alphaLabel);
	connect(m_alphaSlider, &QSlider::valueChanged, [&](int value) {
		m_alpha = M_PI * value / 1000.0;
		m_alphaLabel->setText(QString("%1π").arg(value / 1000.0, 0, 'f', 3));
		recalculate();
	});
	for (auto place : m_places) {
		m_renderer->registerEditable(place);
	}
	connect(m_renderer, &GeometryWidget::edited, [&]() {
		recalculate();
	});
	recalculate();
}

void OptimizationDemo::recalculate() {
	auto tree = std::make_shared<SpiralTree>(Point<Inexact>(0, 0), m_alpha);
	for (const auto& place : m_places) {
		tree->addPlace("", *place, 1);
	}
	tree->addShields();

	m_renderer->clear();
	ReachableRegionAlgorithm::ReachableRegion reachableRegion = ReachableRegionAlgorithm(tree).run();
	SpiralTreeObstructedAlgorithm(tree, reachableRegion).run();
	m_smoothTree = std::make_shared<SmoothTree>(tree);
	SmoothTreePainting::Options options;
	auto painting = std::make_shared<SmoothTreePainting>(m_smoothTree, options);
	m_renderer->addPainting(painting, "Smooth tree");

	m_renderer->update();
}

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	OptimizationDemo demo;
	demo.show();
	app.exec();
}
