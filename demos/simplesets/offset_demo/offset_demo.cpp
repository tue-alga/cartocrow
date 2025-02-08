#include "offset_demo.h"
#include "cartocrow/circle_segment_helpers/cavc_helpers.h"
#include "cartocrow/circle_segment_helpers/cs_render_helpers.h"
#include "cartocrow/renderer/geometry_painting.h"
#include "cartocrow/renderer/geometry_widget.h"
#include "cartocrow/renderer/render_path.h"
#include "cartocrow/simplesets/dilated/dilated_poly.h"
#include "cartocrow/simplesets/patterns/bank.h"
#include "cartocrow/simplesets/patterns/single_point.h"
#include "demos/simplesets/colors/colors.h"
#include <QApplication>
#include <QDockWidget>
#include <QVBoxLayout>

using namespace cartocrow;
using namespace cartocrow::renderer;
using namespace cartocrow::simplesets;

OffsetDemo::OffsetDemo() {
	setWindowTitle("Offset");
	m_renderer = new GeometryWidget();
	m_renderer->setDrawAxes(false);
	setCentralWidget(m_renderer);

	auto* dockWidget = new QDockWidget();
	addDockWidget(Qt::RightDockWidgetArea, dockWidget);
	auto* vWidget = new QWidget();
	auto* vLayout = new QVBoxLayout(vWidget);
	vLayout->setAlignment(Qt::AlignTop);
	dockWidget->setWidget(vWidget);

	auto* offsetSlider = new QSlider(Qt::Orientation::Horizontal);
	vLayout->addWidget(offsetSlider);
	offsetSlider->setMinimum(0);
	offsetSlider->setMaximum(160);

	CSPolygonSet input;
	Bank bank({{0, {0, 0}}, {0, {2, 0}}, {0, {1, 2}}});
	SinglePoint sp({0, {2, 4}});
	DilatedPoly dilatedB(bank, 0.5);
	DilatedPoly dilatedSP(sp, 0.5);
	input.insert(dilatedB.m_contour);
	input.insert(dilatedSP.m_contour);

	m_renderer->addPainting([input, this](GeometryRenderer& renderer) {
		renderer.setMode(GeometryRenderer::stroke);
	  	renderer.setStroke(Color{0, 0, 0}, 2);
		renderer.draw(renderPath(input));

		renderer.setStroke(CB::blue, 2);
		renderer.draw(renderPath(m_smoothed));

	  	renderer.setStroke(CB::light_blue, 2);
	  	renderer.draw(renderPath(m_smoothed_));

	  	renderer.setStroke(CB::red, 2);
	  	renderer.draw(renderPath(m_eroded));

	  	renderer.setStroke(CB::green, 2);
	  	renderer.draw(renderPath(m_dilated));
	}, "Curve");

	connect(offsetSlider, &QSlider::valueChanged, [this, input, offsetSlider] {
		double r = offsetSlider->value() / 100.0;
		m_smoothed = approximateSmoothCO(input, r);
	  	m_smoothed_ = approximateSmoothOC(input, r);
	    m_eroded = approximateErode(input, r);
	    m_dilated = approximateDilate(input, r);
		m_renderer->repaint();
	});

	offsetSlider->setValue(1);
}

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	OffsetDemo demo;
	demo.show();
	app.exec();
}
