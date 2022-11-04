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

	this->map = std::make_shared<VWTraits::Map>(regionMapToArrangement<VWVertex, VWEdge>(*regions));
	this->hist = new HistoricArrangement<VWTraits>(*(this->map));

	int incnt = this->map->number_of_edges(); 
	std::cout << "in count " << incnt  << "\n";

	Timer t;
	VWSimplificationWithHistory simplification(*hist);
	simplification.initialize();
	t.stamp("Initialization");
	simplification.simplify(0);
	t.stamp("Simplification done");
	t.output();

	int outcnt = this->map->number_of_edges();
	
	std::cout << "out count " << outcnt << "\n";

	// compare running time vs no history
	//	std::cout << "creating arrangement\n";
	//	VWTraits::Map map2 =regionMapToArrangement<VWVertex, VWEdge>(*regions);
	//	ObliviousArrangement<VWTraits> mapmod(map2);
	//	int incnt2 = map2.number_of_edges();
	//	std::cout << "in count " << incnt2 << "\n";
	//	Timer t;
	//	VWSimplification simplification2(mapmod);
	//	simplification2.initialize();
	//	t.stamp("Initialization");
	//	simplification2.simplify(0);
	//	t.stamp("Simplification done");
	//	t.output();
	//	int outcnt2 = map2.number_of_edges();
	//	std::cout << "out count " << outcnt2 << "\n";

	this->c = (3 * outcnt + incnt) / 4;
	QToolBar* toolBar = new QToolBar();
	toolBar->addWidget(new QLabel("c = "));
	m_cSlider = new QSlider(Qt::Horizontal);
	m_cSlider->setMinimum(outcnt);
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
	hist->recallComplexity(c);
	m_renderer->update();
}

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	VWDemo demo;
	demo.show();
	app.exec();
}
