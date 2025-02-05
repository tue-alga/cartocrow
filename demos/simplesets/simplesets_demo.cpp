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
#include "cartocrow/core/cs_polygon_helpers.h"
#include "cartocrow/renderer/ipe_renderer.h"
#include "cartocrow/renderer/painting_renderer.h"
#include "cartocrow/simplesets/drawing_algorithm.h"
#include "cartocrow/simplesets/parse_input.h"
#include "cartocrow/simplesets/partition_algorithm.h"
#include "cartocrow/simplesets/partition_painting.h"
#include "cartocrow/simplesets/patterns/bank.h"
#include "colors/colors.h"
#include <CGAL/Bbox_2.h>
#include <QApplication>
#include <QCheckBox>
#include <QDockWidget>
#include <QFileDialog>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QVBoxLayout>

namespace fs = std::filesystem;
using namespace cartocrow;
using namespace cartocrow::renderer;
using namespace cartocrow::simplesets;

SimpleSetsDemo::SimpleSetsDemo() {
	setWindowTitle("SimpleSets");

	m_cds = ComputeDrawingSettings{0.675, true, 0.2};

	// Below are two example inputs: nyc and diseasome.
	// Comment/uncomment the following two sets of four lines to switch.

	// nyc
//	m_gs.pointSize = 2.1;
//	m_ds.colors = {CB::light_blue, CB::light_red, CB::light_green, CB::light_purple, CB::light_orange};
//	m_ps.admissibleRadiusFactor = 0.1;
//	std::filesystem::path filePath("data/nyc.txt");

	// diseasome
	m_gs.pointSize = 5.204;
	m_ds.colors = diseasome::colors;
	m_ps.intersectionDelay = false;
	std::filesystem::path filePath("data/diseasome.txt");

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
	auto* coverLabel = new QLabel("Cover");
	vLayout->addWidget(coverLabel);
	auto* coverSlider = new QSlider(Qt::Orientation::Horizontal);
	vLayout->addWidget(coverSlider);
	coverSlider->setMinimum(0);
	coverSlider->setMaximum(80);
	coverSlider->setValue(47);

	auto* ptSizeLabel = new QLabel("Point size");
	vLayout->addWidget(ptSizeLabel);
	auto* ptSizeSlider = new QSlider(Qt::Orientation::Horizontal);
	vLayout->addWidget(ptSizeSlider);
	ptSizeSlider->setMinimum(1);
	ptSizeSlider->setMaximum(80);
	ptSizeSlider->setValue(static_cast<int>(m_gs.pointSize * 10));

	auto* smoothCheckbox = new QCheckBox("Smooth");
	vLayout->addWidget(smoothCheckbox);
	smoothCheckbox->setChecked(true);

	auto* smoothingSliderLabel = new QLabel("Smoothing radius factor");
	vLayout->addWidget(smoothingSliderLabel);
	auto* smoothingSlider = new QSlider(Qt::Orientation::Horizontal);
	vLayout->addWidget(smoothingSlider);
	smoothingSlider->setMinimum(1);
	smoothingSlider->setMaximum(20);
	smoothingSlider->setValue(20);

	m_renderer = new GeometryWidget();
	m_renderer->setDrawAxes(false);
	setCentralWidget(m_renderer);

	m_renderer->setMinZoom(0.01);
	m_renderer->setMaxZoom(1000.0);

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
	connect(ptSizeSlider, &QSlider::valueChanged, [this, ptSizeSlider, coverSlider] {
		double value = ptSizeSlider->value() / 10.0;
		m_gs.pointSize = value;
	  	computePartitions();
	  	computeDrawing(coverSlider->value() / 10.0);
	  	fitToScreen();
	});
	connect(smoothCheckbox, &QCheckBox::stateChanged, [this, smoothCheckbox, coverSlider] {
		m_cds.smooth = smoothCheckbox->isChecked();
		computeDrawing(coverSlider->value() / 10.0);
	});
	connect(smoothingSlider, &QSlider::valueChanged, [this, smoothingSlider, coverSlider] {
		m_cds.smoothingRadiusFactor = smoothingSlider->value() / 100.0;
		if (m_cds.smooth) {
			computeDrawing(coverSlider->value() / 10.0);
		}
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
	auto delta = CGAL::to_double(2 * m_gs.dilationRadius());
	Box expanded(box.xmin() - delta, box.ymin() - delta, box.xmax() + delta, box.ymax() + delta);
	m_renderer->fitInView(expanded);
}

void SimpleSetsDemo::resizeEvent(QResizeEvent *event) {
	fitToScreen();
}

void SimpleSetsDemo::computePartitions(){
	m_partitions = partition(m_points, m_gs, m_ps, 8 * CGAL::to_double(m_gs.dilationRadius()));
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

	bool wellSeparated = true;
	for (const auto& p : m_points) {
		for (const auto& q : m_points) {
			if (p.category == q.category) continue;
			if (CGAL::squared_distance(p.point, q.point) < 4 * m_gs.pointSize * m_gs.pointSize) {
				wellSeparated = false;
			}
		}
	}
	if (wellSeparated) {
		m_dpd = std::make_shared<DilatedPatternDrawing>(m_partition, m_gs, m_cds);
		auto ssp = SimpleSetsPainting(*m_dpd, m_ds);
		auto paintingRenderer = std::make_shared<PaintingRenderer>();
		ssp.paint(*paintingRenderer);
		m_renderer->addPainting(paintingRenderer, "Drawing");
	} else {
		std::cerr << "Points of different category are too close together; not computing a drawing." << std::endl;
	}
}

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	SimpleSetsDemo demo;
	demo.show();
	app.exec();
}
