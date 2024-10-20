#include "offset_demo.h"
#include "cartocrow/renderer/geometry_painting.h"
#include "cartocrow/renderer/geometry_widget.h"
#include "cartocrow/renderer/render_path.h"
#include "demos/simplesets/colors/colors.h"
#include <QApplication>
#include <QDockWidget>
#include <QVBoxLayout>
#include "cavc/include/cavc/polylineoffset.hpp"

using namespace cartocrow;
using namespace cartocrow::renderer;
using namespace cartocrow::simplesets;

RenderPath renderPath(const cavc::Polyline<double>& polyline) {
	RenderPath rp;
	auto vs = polyline.vertexes();
	if (polyline.isClosed()) {
		vs.push_back(vs.front());
	}

	for (int i = 0; i < vs.size(); ++i) {
		const auto& v = vs[i];
		if (i == 0) {
			rp.moveTo({v.x(), v.y()});
		} else {
			const auto& pv = vs[i-1];
			if (pv.bulgeIsZero()) {
				rp.lineTo({v.x(), v.y()});
			} else {
				bool clockwise = pv.bulgeIsNeg();
				Point<Inexact> source(pv.x(), pv.y());
				Point<Inexact> target(v.x(), v.y());
				Point<Inexact> m = CGAL::midpoint(source, target);
				auto dir = target - source;
				dir /= 2;
				Vector<Inexact> perp = dir.perpendicular(CGAL::CLOCKWISE);
				auto third = m + perp * pv.bulge();
				auto center = CGAL::circumcenter(source, target, third);
				rp.arcTo(center, clockwise, target);
			}
		}
	}

	if (polyline.isClosed()) {
		rp.close();
	}
	return rp;
}

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
	offsetSlider->setMinimum(-80);
	offsetSlider->setMaximum(80);

	// input polyline
	cavc::Polyline<double> input;
	// add vertexes as (x, y, bulge)
	input.addVertex(0, 25, 1);
	input.addVertex(0, 0, 0);
	input.addVertex(2, 0, 1);
	input.addVertex(10, 0, -0.5);
	input.addVertex(8, 9, 0.374794619217547);
	input.addVertex(21, 0, 0);
	input.addVertex(23, 0, 1);
	input.addVertex(32, 0, -0.5);
	input.addVertex(28, 0, 0.5);
	input.addVertex(39, 21, 0);
	input.addVertex(28, 12, 0.5);
	input.isClosed() = true;

	// compute the resulting offset polylines, offset = 3
	m_results = cavc::parallelOffset(input, -3.0);

	m_renderer->addPainting([input, this](GeometryRenderer& renderer) {
		renderer.setMode(GeometryRenderer::stroke);
	  	renderer.setStroke(Color{0, 0, 0}, 1);
		renderer.draw(renderPath(input));

		for (const auto& r : m_results) {
			renderer.setStroke(CB::blue, 1);
			renderer.draw(renderPath(r));
		}
	}, "Curve");

	connect(offsetSlider, &QSlider::valueChanged, [this, input, offsetSlider] {
	  	m_results = cavc::parallelOffset(input, offsetSlider->value() / 10.0);
		m_renderer->repaint();
	});
}

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	OffsetDemo demo;
	demo.show();
	app.exec();
}
