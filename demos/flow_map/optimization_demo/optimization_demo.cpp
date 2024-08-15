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
#include <QDockWidget>
#include <QStatusBar>

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

	for (int i = 0; i < 40; i++) {
		m_places.push_back(std::make_shared<Point<Inexact>>(
		    static_cast<float>(std::rand()) / RAND_MAX * 50 - 25,
		    static_cast<float>(std::rand()) / RAND_MAX * 50 - 25));
	}

	m_renderer = new GeometryWidget();
	m_renderer->zoomIn();
	m_renderer->zoomIn();
	m_renderer->zoomIn();
	m_renderer->setMaxZoom(10000);
	m_renderer->setGridMode(GeometryWidget::GridMode::POLAR);
	setCentralWidget(m_renderer);

	m_costGraph = new CostGraph();
	QDockWidget* dockWidget = new QDockWidget();
	dockWidget->setWindowTitle("Cost history");
	dockWidget->setWidget(m_costGraph);
	addDockWidget(Qt::RightDockWidgetArea, dockWidget);

	QToolBar* toolBar = new QToolBar();
	toolBar->addSeparator();
	m_optimizeButton = new QPushButton("Run optimization");
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
		for (int i = 0; i < 100; i++) {
			m_iterationCount++;
			m_smoothTree->optimize();
			updateCostLabel();
			if (m_stopOnNanCheckbox->isChecked() && std::isnan(m_smoothTree->computeCost())) {
				m_optimizeButton->setChecked(false);
				m_optimizeTimer->stop();
				break;
			}
		}
		m_renderer->update();
	});
	m_stopOnNanCheckbox = new QCheckBox("Stop on nan");
	toolBar->addWidget(m_stopOnNanCheckbox);
	m_optimizeOneStepButton = new QPushButton("Optimize one step");
	toolBar->addWidget(m_optimizeOneStepButton);
	connect(m_optimizeOneStepButton, &QPushButton::clicked, [&]() {
		m_iterationCount++;
		m_smoothTree->optimize();
		m_renderer->update();
		updateCostLabel();
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

	m_costLabel = new QLabel();
	statusBar()->addWidget(m_costLabel);

	for (auto place : m_places) {
		m_renderer->registerEditable(place);
	}
	connect(m_renderer, &GeometryWidget::edited, [&]() {
		recalculate();
	});
	recalculate();
}

void OptimizationDemo::recalculate() {
	m_renderer->clear();
	m_costGraph->clear();
	m_iterationCount = 0;
	auto tree = std::make_shared<SpiralTree>(Point<Inexact>(0, 0), m_alpha);
	for (const auto& place : m_places) {
		tree->addPlace("", *place, 0.1f);
	}
	tree->addShields();

	ReachableRegionAlgorithm::ReachableRegion reachableRegion = ReachableRegionAlgorithm(tree).run();
	SpiralTreeObstructedAlgorithm(tree, reachableRegion).run();
	m_smoothTree = std::make_shared<SmoothTree>(tree);
	/*for (auto& node : m_smoothTree->m_nodes) {
		if (node->getType() == Node::ConnectionType::kSubdivision) {
			node->m_position.setPhi(wrapAngle(
			    node->m_position.phi() + static_cast<float>(std::rand()) / RAND_MAX * 0.2 - 0.1,
			    -M_PI));
		}
	}*/
	SmoothTreePainting::Options options;
	auto painting = std::make_shared<SmoothTreePainting>(m_smoothTree, options);
	m_renderer->addPainting(painting, "Smooth tree");

	m_renderer->update();
	updateCostLabel();
}

void OptimizationDemo::updateCostLabel() {
	m_costGraph->addStep({m_smoothTree->computeObstacleCost(), m_smoothTree->computeSmoothingCost(),
	                      m_smoothTree->computeAngleRestrictionCost(),
	                      m_smoothTree->computeBalancingCost(),
	                      m_smoothTree->computeStraighteningCost()});
	Number<Inexact> cost = m_smoothTree->computeCost();
	m_costLabel->setText(
	    QString("Iteration %1 | Cost function: %2")
	        .arg(m_iterationCount)
	        .arg(std::isnan(cost) ? "<b><font color=\"#d5004a\">nan</font></b>" : QString::number(cost)));
}

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	OptimizationDemo demo;
	demo.show();
	app.exec();
}
