#include "vw_demo.h"

#include <QApplication>

#include "cartocrow/core/core.h"
#include "cartocrow/core/centroid.h"
#include "cartocrow/core/boundary_map.h"
#include "cartocrow/core/arrangement_map.h"
#include "cartocrow/core/arrangement_helpers.h"
#include "cartocrow/core/timer.h"
#include "cartocrow/renderer/geometry_painting.h"
#include "cartocrow/renderer/geometry_widget.h"
#include "cartocrow/renderer/ipe_renderer.h"
#include "cartocrow/simplification/painting.h"
#include "cartocrow/simplification/vertex_removal/visvalingam_whyatt.h"

using namespace cartocrow;
using namespace cartocrow::simplification;
using namespace cartocrow::renderer;

class RegionArrangementPainting : public GeometryPainting {
private:
    std::shared_ptr<RegionArrangement> m_arr;

public:
    RegionArrangementPainting(std::shared_ptr<RegionArrangement> arr) : m_arr(std::move(arr)) {};

    void paint(GeometryRenderer &renderer) const override {
        for (auto fit = m_arr->faces_begin(); fit != m_arr->faces_end(); ++fit) {
            if (!fit->has_outer_ccb()) continue;
            auto region = fit->data();
            if (region.empty() || region == "#") {
                continue;
            }
            auto poly = approximate(face_to_polygon_with_holes<Exact>(fit));
            renderer.setMode(GeometryRenderer::fill | GeometryRenderer::stroke);
            renderer.setFill(Color{200, 200, 200});
            renderer.setStroke(Color{0, 0, 0}, 1.0);
            renderer.draw(poly);
            auto c = centroid(poly);
            renderer.setMode(GeometryRenderer::stroke);
            renderer.drawText(c, region);
        }
//        renderer.setMode(GeometryRenderer::stroke);
//        renderer.setStroke(Color{0, 0, 0}, 1.0);
//        for (auto eit = m_arr->edges_begin(); eit != m_arr->edges_end(); ++eit) {
//            Segment<Exact> seg = eit->curve();
//            renderer.draw(seg);
//        }
    }
};

VWDemo::VWDemo() {
	setWindowTitle("CartoCrow : Visvalingam-Whyatt demo");

	m_renderer = new GeometryWidget();
	setCentralWidget(m_renderer);

	std::filesystem::path file =
	    std::filesystem::absolute(std::filesystem::path("data/chorematic_map/gemeenten-2022_19282vtcs.ipe"));
	std::cout << "reading file " << file << "\n";

	// step 1: create a RegionMap
//	this->inputmap = std::make_shared<BoundaryMap>(ipeToBoundaryMap(file));
    this->inputmap = std::make_shared<RegionArrangement>(regionMapToArrangement(ipeToRegionMap(file, true)));

	std::cout << "creating arrangement\n";

	// step 2: convert this to an arrangement with the VWTraits
	// and wrap it in a historic arrangement to allow for quickly recovering all
	// solution
//	this->map =
//	    std::make_shared<VWTraits<>::Map>(boundaryMapToArrangementMap<VWVertex<>, VWEdge<>, std::monostate>(*(this->inputmap)));
    this->map = std::make_shared<VWTraits<std::string>::Map>(regionArrangementToArrangementMap<VWVertex<std::string>, VWEdge<std::string>>(*(this->inputmap)));
	this->hist = new HistoricArrangement<VWTraits<std::string>>(*(this->map));

	int incnt = this->map->number_of_edges();
	std::cout << "in count " << incnt << "\n";

	Timer t;
	// step 3: initialize the algorithm
	VWSimplificationWithHistory<std::string> simplification(*hist);
	simplification.initialize();
	t.stamp("Initialization");

	// step 4: simplify until no more vertices can be removed
	simplification.simplify(5000);
	t.stamp("Simplification done");
	t.output();

	int outcnt = this->map->number_of_edges();

	std::cout << "out count " << outcnt << "\n";

	// compare running time vs no history
	//std::cout << "creating arrangement\n";
	//VWTraits::Map map2 =regionMapToArrangement<VWVertex, VWEdge>(*regions);
	//ObliviousArrangement<VWTraits> mapmod(map2);
	//int incnt2 = map2.number_of_edges();
	//std::cout << "in count " << incnt2 << "\n";
	//Timer t2;
	//VWSimplification simplification2(mapmod);
	//simplification2.initialize();
	//t2.stamp("Initialization");
	//simplification2.simplify(0);
	//t2.stamp("Simplification done");
	//t2.output();
	//int outcnt2 = map2.number_of_edges();
	//std::cout << "out count " << outcnt2 << "\n";

	// initialize a gui with a slider to retrieve all intermediate solutions
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

	BoundaryPainting::Options in_options;
	in_options.line_width = 1;
//	auto in_painting = std::make_shared<ArrangementPainting>(this->inputmap, in_options);
//    auto out_painting = std::make_shared<ArrangementPainting<VWTraits<std::string>::Map>>(this->map, out_options);

	ArrangementPainting<VWTraits<std::string>::Map>::Options out_options;
	out_options.color = Color{200, 10, 50};
	out_options.line_width = 2;
	auto out_painting = std::make_shared<ArrangementPainting<VWTraits<std::string>::Map>>(this->map, out_options);
    auto regionArrangement = std::make_shared<RegionArrangement>(arrangementMapToRegionArrangement(*map));
    auto ra_painting = std::make_shared<RegionArrangementPainting>(regionArrangement);

	m_renderer->clear();
//	m_renderer->addPainting(in_painting, "Input map");
	m_renderer->addPainting(out_painting, "Output map");
    m_renderer->addPainting(ra_painting, "Region arrangement");

	recalculate();
}

void VWDemo::recalculate() {
	hist->recallComplexity(c);
    if (map->is_valid()) {
        std::cout << "Simplification is valid" << std::endl;
    } else {
        std::cout << "Simplification is not valid" << std::endl;
    }
	m_renderer->update();
}

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	VWDemo demo;
	demo.show();
	app.exec();
}
