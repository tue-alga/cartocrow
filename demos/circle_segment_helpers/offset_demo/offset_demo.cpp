#include "offset_demo.h"
#include "cartocrow/circle_segment_helpers/cavc_helpers.h"
#include "cartocrow/circle_segment_helpers/cs_render_helpers.h"
#include "cartocrow/core/arrangement_helpers.h"
#include "cartocrow/renderer/geometry_painting.h"
#include "cartocrow/renderer/geometry_widget.h"
#include "cartocrow/renderer/render_path.h"
#include "demos/widgets/double_slider.h"

#include <QApplication>
#include <QDockWidget>
#include <QVBoxLayout>
#include <QLabel>

#include <CGAL/approximated_offset_2.h>

using namespace cartocrow;
using namespace cartocrow::renderer;

CSPolygon dilateSegment(const Segment<Exact>& segment, const Number<Exact>& dilationRadius) {
    std::vector<Point<Exact>> points({segment.source(), segment.target()});
    Polygon<Exact> polygon(points.begin(), points.end());
    auto dilation = CGAL::approximated_offset_2(polygon, dilationRadius, M_EPSILON);
    return dilation.outer_boundary();
}

CSPolygon dilatePolyline(const Polyline<Exact>& polyline, const Number<Exact>& dilationRadius) {
    CSArrangement arr;

    for (auto eit = polyline.edges_begin(); eit != polyline.edges_end(); ++eit) {
        auto seg = *eit;
        auto dilated = dilateSegment(seg, dilationRadius);
        for (auto cit = dilated.curves_begin(); cit != dilated.curves_end(); ++cit) {
            CGAL::insert(arr, *cit);
        }
    }

    auto dilated = ccb_to_general_polygon<ArrCSTraits>(*(arr.unbounded_face()->inner_ccbs_begin()));
    if (dilated.orientation() == CGAL::CLOCKWISE) {
        dilated.reverse_orientation();
    }

    return dilated;
}

OffsetDemo::OffsetDemo() {
	setWindowTitle("Offset");

	m_renderer = new GeometryWidget();
	m_renderer->setDrawAxes(false);
    m_renderer->fitInView(Box(-2, -2, 5, 7));
	setCentralWidget(m_renderer);

	auto* dockWidget = new QDockWidget();
	addDockWidget(Qt::RightDockWidgetArea, dockWidget);
	auto* vWidget = new QWidget();
	auto* vLayout = new QVBoxLayout(vWidget);
	vLayout->setAlignment(Qt::AlignTop);
	dockWidget->setWidget(vWidget);

    auto* offsetLabel = new QLabel("Offset");
    vLayout->addWidget(offsetLabel);
	auto* offsetSlider = new DoubleSlider(Qt::Orientation::Horizontal);
	vLayout->addWidget(offsetSlider);
	offsetSlider->setMinimum(0);
	offsetSlider->setMaximum(1.6);

	CSPolygonSet input;
    Polyline<Exact> polyline1;
    polyline1.push_back({0, 0});
    polyline1.push_back({2, 0});
    polyline1.push_back({1, 2});
    auto dilatedPolyline1 = dilatePolyline(polyline1, 0.5);

    Polyline<Exact> polyline2;
    polyline2.push_back({2, 4});
    polyline2.push_back({3, 5});
    auto dilatedPolyline2 = dilatePolyline(polyline2, 0.5);

	input.insert(dilatedPolyline1);
    input.insert(dilatedPolyline2);

	m_renderer->addPainting([input, this](GeometryRenderer& renderer) {
		renderer.setMode(GeometryRenderer::stroke);
	  	renderer.setStroke(Color{200, 200, 200}, 2);
		renderer.draw(renderPath(input));

		renderer.setStroke(Color(50, 50, 255), 2);
		renderer.draw(renderPath(m_smoothed));

	  	renderer.setStroke(Color(255, 200, 200), 2);
	  	renderer.draw(renderPath(m_smoothed_));

	  	renderer.setStroke(Color(255, 50, 50), 2);
	  	renderer.draw(renderPath(m_eroded));

	  	renderer.setStroke(Color(50, 255, 50), 2);
	  	renderer.draw(renderPath(m_dilated));
	}, "Curve");

	connect(offsetSlider, &DoubleSlider::valueChanged, [this, input, offsetSlider] {
		double r = offsetSlider->value();
		m_smoothed = approximateSmoothCO(input, r);
	  	m_smoothed_ = approximateSmoothOC(input, r);
	    m_eroded = approximateErode(input, r);
	    m_dilated = approximateDilate(input, r);
		m_renderer->repaint();
	});

	offsetSlider->setValue(0.01);
}

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	OffsetDemo demo;
	demo.show();
	app.exec();
}
