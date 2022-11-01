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

#include "vw_demo.h"

#include <QApplication>

#include "cartocrow/core/core.h"
#include "cartocrow/core/region_arrangement.h"
#include "cartocrow/core/region_map.h"
#include "cartocrow/core/timer.h"
#include "cartocrow/renderer/geometry_painting.h"
#include "cartocrow/renderer/geometry_widget.h"
#include "cartocrow/renderer/ipe_renderer.h"
#include "cartocrow/simplification/painting.h"
#include "cartocrow/simplification/vertex_removal/visvalingam_whyatt.h"

using namespace cartocrow;
using namespace cartocrow::simplification;
using namespace cartocrow::renderer;

VWDemo::VWDemo() {
	setWindowTitle("CartoCrow : Visvalingam-Whyatt demo");

	m_renderer = new GeometryWidget();
	setCentralWidget(m_renderer);

	std::filesystem::path file =
	    std::filesystem::absolute(std::filesystem::path("data/benelux.ipe"));
	std::cout << "file " << file << "\n";

	this->regions = ipeToRegionMap(file);
	this->inmap = std::make_shared<VWTraits::Map>(regionMapToArrangement<VWVertex>(this->regions));

	std::cout << "in count " << this->inmap->number_of_edges() << "\n";

	this->c = 50;
	QToolBar* toolBar = new QToolBar();
	toolBar->addWidget(new QLabel("c = "));
	m_cSlider = new QSlider(Qt::Horizontal);
	m_cSlider->setMinimum(0);
	m_cSlider->setMaximum(this->inmap->number_of_edges());
	m_cSlider->setValue(this->c);
	toolBar->addWidget(this->m_cSlider);
	addToolBar(toolBar);
	m_cLabel = new QLabel("50");
	toolBar->addWidget(this->m_cLabel);
	connect(m_cSlider, &QSlider::valueChanged, [&](int value) {
		this->c = value;
		m_cLabel->setText(QString::number(this->c));
		recalculate();
	});

	recalculate();
}

void VWDemo::recalculate() {
	Timer t;

	auto outmap = std::make_shared<VWTraits::Map>(regionMapToArrangement<VWVertex>(regions));

	VWSimplification vw(*outmap);
	vw.initialize();
	t.stamp("Initialization");
	vw.simplify(c, 10000000);
	t.stamp("Simplification done");
	t.output();

	Painting<VWTraits::Map>::Options in_options;
	in_options.line_width = 2;
	auto in_painting = std::make_shared<Painting<VWTraits::Map>>(inmap, in_options);

	Painting<VWTraits::Map>::Options out_options;
	out_options.color = Color{200, 10, 50};
	out_options.line_width = 2;
	auto out_painting = std::make_shared<Painting<VWTraits::Map>>(outmap, out_options);

	m_renderer->clear();
	m_renderer->addPainting(in_painting, "Input map");
	m_renderer->addPainting(out_painting, "Output map");

	m_renderer->update();
}

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	VWDemo demo;
	demo.show();
	app.exec();
}
