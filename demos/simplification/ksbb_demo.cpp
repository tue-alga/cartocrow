#include "ksbb_demo.h"

#include <QApplication>

#include "cartocrow/core/core.h"
#include "cartocrow/core/region_arrangement.h"
#include "cartocrow/core/region_map.h"
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
	KSBBDebugPainter(std::shared_ptr<KSBBTraits::Map> arr, Options options = {})
	    : m_arr(arr), m_options(options){};

  protected:
	void paint(renderer::GeometryRenderer& renderer) const override {

		int num = 0;
		for (auto& e : m_arr->edge_handles()) {
			if (e->source()->degree() != 2 || e->target()->degree() != 2) {
				continue;
			}

			if (e->next()->next()->next() == e || e->twin()->next()->next()->next() == e->twin()) {
				continue;
			}
			//if (e->source()->point() != Point<Exact>(208, 224) ||
			//    e->target()->point() != Point<Exact>(128, 192))
			//	continue;


			//std::cout << e->source()->point() << "->" << e->target()->point() << "\n";
			Collapse col = KSBBTraits::ecComputeCollapse(e);

			Color color{ (num * 27) % 256, (50 + num * 13) % 256, (100 + num*73) % 256};
			num++;

			renderer.setStroke(Color(0,0,0), 8);
			renderer.setMode(renderer::GeometryRenderer::stroke);
			Segment<Exact> s = Segment<Exact>(e->source()->point() + (e->target()->point() - e->source()->point()) / 2,
			                   e->target()->point());
			renderer.draw(s);

			renderer.setStroke(color,4);
			renderer.setFill(color);
			renderer.setFillOpacity(75);
			for (Polygon<Exact> p : col.this_face_polygons) {
				Polygon<Inexact> ap = approximate(p);
				renderer.setMode(renderer::GeometryRenderer::fill);
				renderer.draw(ap);
				renderer.setMode(renderer::GeometryRenderer::stroke);
				renderer.draw(ap);
			}
			for (Polygon<Exact> p : col.twin_face_polygons) {
				Polygon<Inexact> ap = approximate(p);
				renderer.setMode(renderer::GeometryRenderer::fill);
				renderer.draw(ap);
			}
		}
	}

  private:
	// The arrangement we are drawing.
	std::shared_ptr<KSBBTraits::Map> m_arr;
	// The drawing options.
	Options m_options;
};

KSBBDemo::KSBBDemo() {
	setWindowTitle("CartoCrow : Kronenfeld-etal demo");

	m_renderer = new GeometryWidget();
	setCentralWidget(m_renderer);

	std::filesystem::path file =
	    std::filesystem::absolute(std::filesystem::path("data/benelux-fix.ipe"));
	std::cout << "reading file " << file << "\n";

	// step 1: create a RegionMap
	this->regions = std::make_shared<RegionMap>(ipeToRegionMap(file));

	std::cout << "creating arrangement\n";

	// step 2: convert this to an arrangement with the KSBBTraits
	// and wrap it in a historic arrangement to allow for quickly recovering all
	// solutions
	this->map =
	    std::make_shared<KSBBTraits::Map>(regionMapToArrangement<KSBBVertex, KSBBEdge>(*regions));
	this->hist = new HistoricArrangement<KSBBTraits>(*(this->map));

	int incnt = this->map->number_of_edges();
	std::cout << "in count " << incnt << "\n";

	Timer t;
	// step 3: initialize the algorithm
	KSBBSimplificationWithHistory simplification(*hist);
	simplification.initialize();
	t.stamp("Initialization");

	// step 4: simplify until no more vertices can be removed
	simplification.simplify(0);
	t.stamp("Simplification done");
	t.output();

	int outcnt = this->map->number_of_edges();

	std::cout << "out count " << outcnt << "\n";

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

	MapPainting::Options in_options;
	in_options.line_width = 1;
	in_options.fill = false;
	auto in_painting = std::make_shared<MapPainting>(this->regions, in_options);

	ArrangementPainting<KSBBTraits::Map>::Options out_options;
	out_options.color = Color{200, 10, 50};
	out_options.line_width = 2;
	auto out_painting =
	    std::make_shared<ArrangementPainting<KSBBTraits::Map>>(this->map, out_options);

	auto debug_paint = std::make_shared<KSBBDebugPainter>(this->map);

	m_renderer->clear();
	m_renderer->addPainting(debug_paint, "Debug");
	m_renderer->addPainting(in_painting, "Input map");
	m_renderer->addPainting(out_painting, "Output map");

	recalculate();
}

void KSBBDemo::recalculate() {
	hist->recallComplexity(c);
	m_renderer->update();
}

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	KSBBDemo demo;
	demo.show();
	app.exec();
}
