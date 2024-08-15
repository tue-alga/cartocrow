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
#include "cartocrow/simplesets/patterns/bank.h"
#include "cartocrow/simplesets/patterns/island.h"
#include "cartocrow/simplesets/patterns/matching.h"
#include "cartocrow/simplesets/patterns/single_point.h"
#include "cartocrow/simplesets/parse_input.h"
#include "cartocrow/simplesets/partition_algorithm.h"
#include "cartocrow/simplesets/partition_painting.h"
#include "cartocrow/simplesets/drawing_algorithm.h"
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDockWidget>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include "cartocrow/simplesets/dilated/dilated_poly.h"
#include "colors.h"

namespace fs = std::filesystem;
using namespace cartocrow;
using namespace cartocrow::renderer;
using namespace cartocrow::simplesets;

SimpleSetsDemo::SimpleSetsDemo() {
	setWindowTitle("SimpleSets");

	auto* dockWidget = new QDockWidget();
	addDockWidget(Qt::RightDockWidgetArea, dockWidget);
	auto* vWidget = new QWidget();
	auto* vLayout = new QVBoxLayout(vWidget);
	vLayout->setAlignment(Qt::AlignTop);
	dockWidget->setWidget(vWidget);

	auto* basicOptions = new QLabel("<h3>Input</h3>");
	vLayout->addWidget(basicOptions);
	auto* fileSelector = new QComboBox();
	auto* fileSelectorLabel = new QLabel("Input file");
	fileSelectorLabel->setBuddy(fileSelector);
	vLayout->addWidget(fileSelectorLabel);
	vLayout->addWidget(fileSelector);

	auto renderer = new GeometryWidget();
	renderer->setDrawAxes(false);
	setCentralWidget(renderer);

	renderer->setMinZoom(0.01);
	renderer->setMaxZoom(1000.0);

	std::filesystem::path filePath("/home/steven/Documents/cartocrow/data/nyc.txt");
	std::ifstream inputStream(filePath, std::ios_base::in);
	if (!inputStream.good()) {
		throw std::runtime_error("Failed to read input");
	}
	std::stringstream buffer;
	buffer << inputStream.rdbuf();
	auto nycPoints = parseCatPoints(buffer.str());

	m_gs = GeneralSettings{2.1, 2, M_PI, 70.0 / 180 * M_PI};
//	m_ps = PartitionSettings{true, true, true, true, 0.5}; todo: fix intersection delay crash
	m_ps = PartitionSettings{true, true, true, false, 0.5};
	m_ds = DrawSettings{{CB::light_blue, CB::light_red, CB::light_green, CB::light_purple, CB::light_orange}};
	m_cds = ComputeDrawingSettings{0.675};
	auto partitionList = partition(nycPoints, m_gs, m_ps, 8 * m_gs.dilationRadius());
	std::cout << partitionList.size() << std::endl;

	Number<Inexact> cover = 4.7;

	Partition* thePartition;
	for (auto& [time, partition] : partitionList) {
		if (time < cover * m_gs.dilationRadius()) {
			thePartition = &partition;
		}
	}
	m_partition = *thePartition;

	auto pp = std::make_shared<PartitionPainting>(m_partition, m_gs, m_ds);
	renderer->addPainting(pp, "Partition");

	m_arr = dilateAndArrange(m_partition, m_gs, m_cds);
	auto ap = std::make_shared<ArrangementPainting>(m_arr, m_ds, m_partition);
	renderer->addPainting(ap, "Arrangement");
}

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	SimpleSetsDemo demo;
	demo.show();
	app.exec();
}
