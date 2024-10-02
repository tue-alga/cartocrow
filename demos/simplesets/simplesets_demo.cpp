/*
The CartoCrow library implements algorithmic geo-visualization methods,
developed at TU Eindhoven.
Copyright (C) 2024 TU Eindhoven

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

#include "simplesets_demo.h"
#include "cartocrow/core/ipe_reader.h"
#include "cartocrow/renderer/ipe_renderer.h"
#include "cartocrow/simplesets/dilated/dilated_poly.h"
#include "cartocrow/simplesets/drawing_algorithm.h"
#include "cartocrow/simplesets/helpers/approximate_convex_hull.h"
#include "cartocrow/simplesets/helpers/arrangement_helpers.h"
#include "cartocrow/simplesets/helpers/cs_polygon_helpers.h"
#include "cartocrow/simplesets/helpers/cs_polyline_helpers.h"
#include "cartocrow/simplesets/helpers/poly_line_gon_intersection.h"
#include "cartocrow/simplesets/parse_input.h"
#include "cartocrow/simplesets/partition_algorithm.h"
#include "cartocrow/simplesets/partition_painting.h"
#include "cartocrow/simplesets/patterns/bank.h"
#include "cartocrow/simplesets/patterns/island.h"
#include "cartocrow/simplesets/patterns/matching.h"
#include "cartocrow/simplesets/patterns/single_point.h"
#include "colors.h"
#include <CGAL/Bbox_2.h>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDockWidget>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QSlider>
#include <QVBoxLayout>

namespace fs = std::filesystem;
using namespace cartocrow;
using namespace cartocrow::renderer;
using namespace cartocrow::simplesets;

SimpleSetsDemo::SimpleSetsDemo() {
	setWindowTitle("SimpleSets");

	m_gs = GeneralSettings{2.1, 2, M_PI, 70.0 / 180 * M_PI}; //nyc
	//	m_gs = GeneralSettings{5.204, 2, M_PI, 70.0 / 180 * M_PI}; //diseasome
	m_ps = PartitionSettings{true, true, true, true, 0.1};
	//	m_ps = PartitionSettings{true, true, true, false, 0.5};
	m_ds = DrawSettings{{CB::light_blue, CB::light_red, CB::light_green, CB::light_purple, CB::light_orange}, 0.7};
	m_cds = ComputeDrawingSettings{0.675};

	auto* dockWidget = new QDockWidget();
	addDockWidget(Qt::RightDockWidgetArea, dockWidget);
	auto* vWidget = new QWidget();
	auto* vLayout = new QVBoxLayout(vWidget);
	vLayout->setAlignment(Qt::AlignTop);
	dockWidget->setWidget(vWidget);

	auto* basicOptions = new QLabel("<h3>Input</h3>");
	vLayout->addWidget(basicOptions);
	auto* fileSelector = new QPushButton("Select file");
	vLayout->addWidget(fileSelector);

	auto* fitToScreenButton = new QPushButton("Fit to screen");
	vLayout->addWidget(fitToScreenButton);

	auto* settingsLabel = new QLabel("<h3>Settings</h3>");
	vLayout->addWidget(settingsLabel);
	auto* coverSlider = new QSlider(Qt::Orientation::Horizontal);
	vLayout->addWidget(coverSlider);
	coverSlider->setMinimum(0);
	coverSlider->setMaximum(80);
	coverSlider->setValue(47);

	m_renderer = new GeometryWidget();
	m_renderer->setDrawAxes(false);
	setCentralWidget(m_renderer);

	m_renderer->setMinZoom(0.01);
	m_renderer->setMaxZoom(1000.0);

	std::filesystem::path filePath("data/nyc.txt");

	loadFile(filePath);
	computePartitions();
	computeDrawing(coverSlider->value() / 10.0);
	fitToScreen();

	connect(fitToScreenButton, &QPushButton::clicked, [this]() {
		fitToScreen();
	});
	connect(fileSelector, &QPushButton::clicked, [this, fileSelector, coverSlider]() {
		QString startDir = "data/";
		std::filesystem::path filePath = QFileDialog::getOpenFileName(this, tr("Select SimpleSets input"), startDir).toStdString();
		if (filePath == "") return;
		loadFile(filePath);
	    computePartitions();
	    computeDrawing(coverSlider->value() / 10.0);
	    fitToScreen();
		fileSelector->setText(QString::fromStdString(filePath.filename()));
	});
	connect(coverSlider, &QSlider::valueChanged, [this, coverSlider] {
		double value = coverSlider->value() / 10.0;
		computeDrawing(value);
	});

	fitToScreenButton->click();
}

void SimpleSetsDemo::loadFile(const std::filesystem::path& filePath) {
	std::ifstream inputStream(filePath, std::ios_base::in);
	if (!inputStream.good()) {
		throw std::runtime_error("Failed to read input");
	}
	std::stringstream buffer;
	buffer << inputStream.rdbuf();
	m_points = parseCatPoints(buffer.str());
}

void SimpleSetsDemo::fitToScreen() {
	std::vector<Point<Inexact>> pts;
	std::transform(m_points.begin(), m_points.end(), std::back_inserter(pts),
	               [](const CatPoint& pt) { return pt.point; });
	Box box = CGAL::bbox_2(pts.begin(), pts.end());
	auto delta = 2 * m_gs.dilationRadius();
	Box expanded(box.xmin() - delta, box.ymin() - delta, box.xmax() + delta, box.ymax() + delta);
	m_renderer->fitInView(expanded);
}

void SimpleSetsDemo::resizeEvent(QResizeEvent *event) {
	fitToScreen();
}

void SimpleSetsDemo::computePartitions(){
	m_partitions = partition(m_points, m_gs, m_ps, 8 * m_gs.dilationRadius());
}

void SimpleSetsDemo::computeDrawing(double cover) {
	Partition* thePartition;
	bool found = false;
	for (auto& [time, partition] : m_partitions) {
		if (time < cover * m_gs.dilationRadius()) {
			thePartition = &partition;
			found = true;
		}
	}
	if (found) {
		m_partition = *thePartition;
	} else {
		m_partition = m_partitions.front().second;
	}

	m_renderer->clear();

	auto pp = std::make_shared<PartitionPainting>(m_partition, m_gs, m_ds);
	m_renderer->addPainting(pp, "Partition");

	m_dpd = std::make_shared<DilatedPatternDrawing>(m_partition, m_gs, m_cds);
	auto ap = std::make_shared<SimpleSetsPainting>(*m_dpd, m_ds);
	m_renderer->addPainting(ap, "Arrangement");
}

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	SimpleSetsDemo demo;
	demo.show();
	app.exec();
}