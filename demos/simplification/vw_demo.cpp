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
	std::cout << "reading file " << file << "\n";

	this->regions = std::make_shared<RegionMap>(ipeToRegionMap(file));

	std::cout << "creating arrangement\n";

	this->map = std::make_shared<VWTraits::Map>(regionMapToArrangement<VWVertex,VWEdge>(*regions));

	int incnt = this->map->number_of_edges(); 
	std::cout << "in count " << incnt  << "\n";

	Timer t;
	this->simplification = new VWSimplification(*map);
	this->simplification->initialize();
	t.stamp("Initialization");
	simplification->simplify(0);
	t.stamp("Simplification done");
	t.output();

	this->c = incnt / 4;
	QToolBar* toolBar = new QToolBar();
	toolBar->addWidget(new QLabel("c = "));
	m_cSlider = new QSlider(Qt::Horizontal);
	m_cSlider->setMinimum(map->number_of_edges());
	m_cSlider->setMaximum(incnt);
	m_cSlider->setValue(this->c);
	toolBar->addWidget(this->m_cSlider);
	addToolBar(toolBar);
	m_cLabel = new QLabel(QString::number(this->c));
	toolBar->addWidget(this->m_cLabel);
	connect(m_cSlider, &QSlider::valueChanged, [&](int value) {
		this->c = value;
		m_cLabel->setText(QString::number(this->c));
		recalculate();
	});

	
	MapPainting::Options in_options;
	in_options.line_width = 1;
	in_options.fill = false;
	auto in_painting = std::make_shared<MapPainting>(this->regions, in_options);

	ArrangementPainting<VWTraits::Map>::Options out_options;
	out_options.color = Color{200, 10, 50};
	out_options.line_width = 2;
	auto out_painting = std::make_shared<ArrangementPainting<VWTraits::Map>>(this->map, out_options);
		
	m_renderer->clear();
	m_renderer->addPainting(in_painting, "Input map");
	m_renderer->addPainting(out_painting, "Output map");

	recalculate();
}

void VWDemo::recalculate() {
	std::cout << "#edges to " << c << "\n";
	simplification->simplify(c);

	m_renderer->update();
}

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	VWDemo demo;
	demo.show();
	app.exec();
}
