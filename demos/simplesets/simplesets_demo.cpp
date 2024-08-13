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

#include "cartocrow/core/ipe_reader.h"
#include "cartocrow/renderer/ipe_renderer.h"
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDockWidget>
#include <QFileDialog>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QMessageBox>
#include "simplesets_demo.h"
#include "cartocrow/simplesets/island.h"
#include "cartocrow/simplesets/bank.h"

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

	Island island({{{1, 1}, 0}, {{2, 1}, 0}, {{3, 1.1}, 0}});
	Bank bank({{{1, 1}, 0}, {{2, 1}, 0}, {{3, 1.1}, 0}});
	std::cout << island.coverRadius() << std::endl;
	std::cout << bank.coverRadius() << std::endl;
}

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	SimpleSetsDemo demo;
	demo.show();
	app.exec();
}
