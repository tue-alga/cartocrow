#include "circle_tangents_demo.h"

#include <QApplication>
#include <QDockWidget>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QLabel>

#include "cartocrow/circle_segment_helpers/circle_tangents.h"
#include "cartocrow/circle_segment_helpers/poly_line_gon_intersection.h"
#include "cartocrow/renderer/geometry_widget.h"

using namespace cartocrow;
using namespace cartocrow::renderer;

std::pair<bool, bool> properTangent(const Segment<Exact>& seg, const RationalRadiusCircle& c, bool end) {
	auto p = end ? seg.target() : seg.source();
	bool endpointsOnCircle = c.circle().has_on_boundary(p);
	Line<Exact> sl = seg.supporting_line();
	Line<Exact> perp = Line<Exact>(c.center, p).perpendicular(p);
	bool tangent = sl == perp || sl == perp.opposite();
	return {endpointsOnCircle, tangent};
}

std::tuple<bool, bool, bool> properTangent(const RationalTangent& seg, const RationalRadiusCircle& c1, const RationalRadiusCircle& c2) {
	auto pl = seg.polyline();
	auto firstEdge = pl.edge(0);
	auto lastEdge = pl.num_edges() == 1 ? firstEdge : pl.edge(1);
	auto [p1, q1] = properTangent(firstEdge, c1, false);
	auto [p2, q2] = properTangent(lastEdge, c2, true);
	auto csPl = polylineToCSPolyline(pl);
	auto r1 = intersection(csPl, circleToCSPolygon(c1.circle()), true).empty();
	auto r2 = intersection(csPl, circleToCSPolygon(c2.circle()), true).empty();

	return {p1 && p2, q1 && q2, r1 && r2};
}

std::tuple<bool, bool, bool> properTangent(const RationalTangent& seg, const Point<Exact>& p, const RationalRadiusCircle& c, bool pointToCircle) {
	auto pl = seg.polyline();
	auto firstEdge = pl.edge(0);
	auto lastEdge = pl.num_edges() == 1 ? firstEdge : pl.edge(1);
	auto pointEdge = pointToCircle ? firstEdge : lastEdge;
	auto circleEdge = pointToCircle ? lastEdge : firstEdge;
	auto p1 = pointToCircle ? pointEdge.source() == p : pointEdge.target() == p;
	auto [p2, q2] = properTangent(circleEdge, c, pointToCircle);

	auto inters = intersection(polylineToCSPolyline(pl), circleToCSPolygon(c.circle()), true);
	auto r = inters.empty();

	return {p1 && p2, q2, r};
}

Color whiten(const Color& color, double a) {
	return {static_cast<int>(255 * a + color.r * (1-a)), static_cast<int>(255 * a + color.g * (1-a)), static_cast<int>(255 * a + color.b * (1-a))};
}

CircleTangentsDemo::CircleTangentsDemo() {
    setWindowTitle("Circle tangents");
    auto renderer = new GeometryWidget();
    renderer->setDrawAxes(false);
    setCentralWidget(renderer);

	auto* dockWidget = new QDockWidget();
	addDockWidget(Qt::RightDockWidgetArea, dockWidget);
	auto* vWidget = new QWidget();
	auto* vLayout = new QVBoxLayout(vWidget);
	vLayout->setAlignment(Qt::AlignTop);
	dockWidget->setWidget(vWidget);
	dockWidget->setMinimumWidth(300);

	QCheckBox* exactCheckBox = new QCheckBox("Exact");
	vLayout->addWidget(exactCheckBox);

	QLabel* tangentInfo = new QLabel();
	vLayout->addWidget(tangentInfo);

	auto p1 = std::make_shared<Point<Inexact>>(-325, 0);
    auto c1 = std::make_shared<Circle<Inexact>>(Point<Inexact>(-125, 0), 100 * 100);
    auto c2 = std::make_shared<Circle<Inexact>>(Point<Inexact>(125, 0), 100 * 100);
	auto p2 = std::make_shared<Point<Inexact>>(325, 0);

    renderer->registerEditable(c1);
    renderer->registerEditable(c2);
	renderer->registerEditable(p1);
	renderer->registerEditable(p2);

	connect(exactCheckBox, &QCheckBox::stateChanged, [renderer] { renderer->repaint(); });

    renderer->addPainting([c1, c2, p1, p2, exactCheckBox, tangentInfo](GeometryRenderer& r) {
		if (!exactCheckBox->isChecked()) {
			r.setMode(GeometryRenderer::stroke | GeometryRenderer::vertices);
			r.setStroke(Color(0, 0, 0), 2.0);

			r.draw(*c1);
			r.draw(c1->center());
			r.draw(*c2);
			r.draw(c2->center());

			auto outer = bitangents(*c1, *c2, false);
			auto inner = bitangents(*c1, *c2, true);
			auto p1Tangents = tangents(*p1, *c1);
			auto p2Tangents = tangents(*c2, *p2);

			std::stringstream ss;
			auto drawTangents = [&ss, &r, &c1, &c2](std::optional<std::pair<Segment<Inexact>, Segment<Inexact>>> ts, std::string name, Color color)  {
				if (ts.has_value()) {
					auto [t1, t2] = *ts;
					r.setStroke(color, 2.0);
					r.draw(t1);
					r.setStroke(whiten(color, 0.7), 2.0);
					r.draw(t2);
				}
			};

			drawTangents(outer, "Outer", Color(0, 0, 255));
			drawTangents(inner, "Inner", Color(0, 255, 0));
			drawTangents(p1Tangents, "Point-circle", Color(0, 200, 200));
			drawTangents(p2Tangents, "Circle-point", Color(200, 0, 200));
			tangentInfo->setText(ss.str().c_str());
			r.setStroke(Color(0, 0, 0), 2.0);
			r.draw(*p1);
			r.draw(*p2);
		} else {
			r.setMode(GeometryRenderer::stroke | GeometryRenderer::vertices);
			r.setStroke(Color(0, 0, 0), 2.0);

			RationalRadiusCircle c1E(pretendExact(c1->center()), sqrt(c1->squared_radius()));
			RationalRadiusCircle c2E(pretendExact(c2->center()), sqrt(c2->squared_radius()));
			auto p1E = pretendExact(*p1);
			auto p2E = pretendExact(*p2);
			r.draw(Circle<Exact>(c1E.center, CGAL::square(c1E.radius)));
			r.draw(Circle<Exact>(c2E.center, CGAL::square(c2E.radius)));
			r.draw(c1E.center);
			r.draw(c2E.center);

			auto outer = rationalBitangents(c1E, c2E, false);
			auto inner = rationalBitangents(c1E, c2E, true);
			auto p1Tangents = rationalTangents(p1E, c1E);
			auto p2Tangents = rationalTangents(c2E, p2E);

			std::stringstream ss;
			auto drawTangents = [&ss, &r, &c1E, &c2E, &p1E, &p2E](std::optional<std::pair<RationalTangent, RationalTangent>> ts, std::string name, Color color, int point = 0)  {
				if (ts.has_value()) {
					auto [t1, t2] = *ts;
					r.setStroke(color, 2.0);
					r.draw(t1.polyline());
					r.setStroke(whiten(color, 0.7), 2.0);
					r.draw(t2.polyline());
					if (point == 0) {
						auto [onCircle1, tangent1, inters1] = properTangent(t1, c1E, c2E);
						auto [onCircle2, tangent2, inters2] = properTangent(t2, c1E, c2E);
						ss << name << "1 endpoints lie on the circles: " << onCircle1 << "\n" <<
						   name << "1 is tangent to both circles: " << tangent1 << "\n" <<
						   name << "1 and circles are interior-disjoint: " << inters1 << "\n" <<
						   name << "2 endpoints lie on the circles: " << onCircle2 << "\n" <<
						   name << "2 is tangent to both circles: " << tangent2 << "\n" <<
						   name << "2 and circles are interior-disjoint: " << inters2 << "\n";
					} else {
						auto cE = point == 1 ? c1E : c2E;
						auto pE = point == 1 ? p1E : p2E;
						auto [endpoints1, tangent1, inters1] = properTangent(t1, pE, cE, point == 1);
						auto [endpoints2, tangent2, inters2] = properTangent(t2, pE, cE, point == 1);
						ss << name << "1 endpoints on circle and point: " << endpoints1 << "\n" <<
						   name << "1 is tangent to circle: " << tangent1 << "\n" <<
						   name << "1 and circles are interior-disjoint: " << inters1 << "\n" <<
						   name << "2 endpoints on circle and point: " << endpoints2 << "\n" <<
						   name << "2 is tangent to circle: " << tangent2 << "\n" <<
						   name << "2 and circles are interior-disjoint: " << inters2 << "\n";
					}
				}
			};

			drawTangents(outer, "Outer", Color(0, 0, 255));
			drawTangents(inner, "Inner", Color(0, 255, 0));
			drawTangents(p1Tangents, "Point-circle", Color(0, 200, 200), 1);
			drawTangents(p2Tangents, "Circle-point", Color(200, 0, 200), 2);
			tangentInfo->setText(ss.str().c_str());
			r.setStroke(Color(0, 0, 0), 2.0);
			r.draw(*p1);
			r.draw(*p2);
		}
    }, "Tangents");
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    CircleTangentsDemo demo;
    demo.show();
    app.exec();
}
