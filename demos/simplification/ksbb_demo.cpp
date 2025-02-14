#include "ksbb_demo.h"

#include <QApplication>

#include "cartocrow/core/core.h"
#include "cartocrow/core/arrangement_map.h"
#include "cartocrow/core/boundary_map.h"
#include "cartocrow/core/timer.h"
#include "cartocrow/renderer/geometry_painting.h"
#include "cartocrow/renderer/geometry_widget.h"
#include "cartocrow/renderer/ipe_renderer.h"
#include "cartocrow/simplification/edge_collapse/kronenfeld_etal.h"
#include "cartocrow/simplification/painting.h"

using namespace cartocrow;
using namespace cartocrow::simplification;
using namespace cartocrow::renderer;

class KSBBDebugPainter : public renderer::GeometryPainting {

  public:
	/// Options that determine what to draw in the painting.
	struct Options {
		/// Default constructor.
		Options(){};
	};

	/// Creates a new painting for the given arrangement.
	KSBBDebugPainter(KSBBDemo& demo, Options options = {}) : m_demo(demo), m_options(options){};

  protected:
	void paint(renderer::GeometryRenderer& renderer) const override {

		int num = 0;
		for (auto& e : m_demo.map->edge_handles()) {

			KSBBTraits<std::string>::Map::Halfedge_handle he = &*e;

			if (he->source()->degree() != 2 || he->target()->degree() != 2) {
				continue;
			}

			if (he->next()->next()->next() == he || he->twin()->next()->next()->next() == he->twin()) {
				continue;
			}
			//if (e->source()->point() != Point<Exact>(208, 224) ||
			//    e->target()->point() != Point<Exact>(128, 192))
			//	continue;

			if (m_demo.hist->atPresent() && KSBBTraits<std::string>::ecGetEdgeMark(he) != ECEdgeMark::MAIN) {
				he = he->twin();
			}

			//std::cout << e->source()->point() << "->" << e->target()->point() << "\n";
			Collapse col = m_demo.hist->atPresent() ? KSBBTraits<std::string>::ecGetCollapse(he)
			                                        : KSBBTraits<std::string>::ecComputeCollapse(he);

			Color color{(num * 27) % 256, (50 + num * 13) % 256, (100 + num * 73) % 256};
			num++;

			renderer.setStroke(Color(0, 0, 0), 8);
			renderer.setMode(renderer::GeometryRenderer::stroke);
			Segment<Exact> s = Segment<Exact>(he->source()->point() +
			                                      (he->target()->point() - he->source()->point()) / 2,
			                                  he->target()->point());
			renderer.draw(s);

			renderer.setMode(renderer::GeometryRenderer::fill | renderer::GeometryRenderer::stroke);
			for (Polygon<Exact> p : col.this_face_polygons) {
				Polygon<Inexact> ap = approximate(p);
				renderer.setStroke(color, 4);
				renderer.setFill(color);
				renderer.setFillOpacity(75);
				renderer.draw(ap);
				if (m_demo.hist->atPresent()) {
					renderer.setFillOpacity(255);
					renderer.setStroke(Color(0,0,0), 4);
					renderer.drawText(CGAL::centroid(ap.vertex(0), ap.vertex(1), ap.vertex(2)),
					                  std::to_string(KSBBTraits<std::string>::ecGetBlockingNumber(he)));
				}
			}

			renderer.setFill(color);
			renderer.setFillOpacity(75);
			renderer.setMode(renderer::GeometryRenderer::fill);
			for (Polygon<Exact> p : col.twin_face_polygons) {
				Polygon<Inexact> ap = approximate(p);
				renderer.draw(ap);
			}
		}
	}

  private:
	// The arrangement we are drawing.
	KSBBDemo& m_demo;
	// The drawing options.
	Options m_options;
};

KSBBDemo::KSBBDemo() {
	setWindowTitle("CartoCrow : Kronenfeld-etal demo");

	m_renderer = new GeometryWidget();
	m_renderer->setMaxZoom(3000);
	setCentralWidget(m_renderer);

	std::filesystem::path file =
	    std::filesystem::absolute(std::filesystem::path("data/europe.ipe"));
	std::cout << "reading file " << file << "\n";

	// step 1: create a RegionMap
//	this->inputmap = std::make_shared<BoundaryMap>(ipeToBoundaryMap(file));
    this->inputmap = std::make_shared<RegionArrangement>(regionMapToArrangement(ipeToRegionMap(file)));

	std::cout << "creating arrangement\n";

	// step 2: convert this to an arrangement with the KSBBTraits
	// and wrap it in a historic arrangement to allow for quickly recovering all
	// solutions
	this->map = std::make_shared<KSBBTraits<std::string>::Map>(regionArrangementToArrangementMap<KSBBVertex, KSBBEdge<std::string>>(*(this->inputmap)));
	this->hist = new HistoricArrangement<KSBBTraits<std::string>>(*(this->map));

	int incnt = this->map->number_of_edges();
	std::cout << "in count " << incnt << "\n";

	Timer t;
	// step 3: initialize the algorithm
	this->alg = new KSBBSimplificationWithHistory<std::string>(*hist);
	this->alg->initialize();
	t.stamp("Initialization");

	// step 4: simplify until no more vertices can be removed
	int to = 14000;
	this->alg->simplify(to);
	t.stamp("Simplification done");
	t.output();

	int outcnt = this->map->number_of_edges();

	// initialize a gui with a slider to retrieve all intermediate solutions
	this->c = outcnt;
	QToolBar* toolBar = new QToolBar();
	toolBar->addWidget(new QLabel("c = "));
	m_cSlider = new QSlider(Qt::Horizontal);
	m_cSlider->setMinimum(0);
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
//	auto in_painting = std::make_shared<BoundaryPainting>(this->inputmap, in_options);

	ArrangementPainting<KSBBTraits<std::string>::Map>::Options out_options;
	out_options.color = Color{200, 10, 50};
	out_options.line_width = 2;
	auto out_painting =
	    std::make_shared<ArrangementPainting<KSBBTraits<std::string>::Map>>(this->map, out_options);

	auto debug_paint = std::make_shared<KSBBDebugPainter>(*this);

	m_renderer->clear();
	m_renderer->addPainting(debug_paint, "Debug");
//	m_renderer->addPainting(in_painting, "Input map");
	m_renderer->addPainting(out_painting, "Output map");

	//IpeRenderer ipe(debug_paint);
	//ipe.save("./ipe.ipe");

	recalculate();
}

void KSBBDemo::recalculate() {

	hist->recallComplexity(c);
	if (this->map->number_of_edges() > this->c) {
		std::cout << "simplifying to " << this->c << "\n";
		this->alg->simplify(this->c);
	}
    if (map->is_valid()) {
        std::cout << "Simplification is valid" << std::endl;
    } else {
        std::cout << "Simplification is not valid" << std::endl;
    }
	m_renderer->update();
}

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	KSBBDemo demo;
	demo.show();
	app.exec();
}
