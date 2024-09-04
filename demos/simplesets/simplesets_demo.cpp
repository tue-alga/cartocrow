/*
The CartoCrow library implements algorithmic geo-visualization methods,
developed at TU Eindhoven.
Copyright (C) 2024 TU Eindhoven

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

#include "simplesets_demo.h"
#include "cartocrow/core/ipe_reader.h"
#include "cartocrow/renderer/ipe_renderer.h"
#include "cartocrow/simplesets/dilated/dilated_poly.h"
#include "cartocrow/simplesets/drawing_algorithm.h"
#include "cartocrow/simplesets/helpers/approximate_convex_hull.h"
#include "cartocrow/simplesets/helpers/arrangement_helpers.h"
#include "cartocrow/simplesets/helpers/cs_polygon_helpers.h"
#include "cartocrow/simplesets/helpers/cs_polyline_helpers.h"
#include "cartocrow/simplesets/helpers/poly_line_gon_intersection.h"
#include "cartocrow/simplesets/parse_input.h"
#include "cartocrow/simplesets/partition_algorithm.h"
#include "cartocrow/simplesets/partition_painting.h"
#include "cartocrow/simplesets/patterns/bank.h"
#include "cartocrow/simplesets/patterns/island.h"
#include "cartocrow/simplesets/patterns/matching.h"
#include "cartocrow/simplesets/patterns/single_point.h"
#include "colors.h"
#include <CGAL/Bbox_2.h>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDockWidget>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace fs = std::filesystem;
using namespace cartocrow;
using namespace cartocrow::renderer;
using namespace cartocrow::simplesets;

SimpleSetsDemo::SimpleSetsDemo() {
	setWindowTitle("SimpleSets");

	auto* dockWidget = new QDockWidget();
	addDockWidget(Qt::RightDockWidgetArea, dockWidget);
	auto* vWidget = new QWidget();
	auto* vLayout = new QVBoxLayout(vWidget);
	vLayout->setAlignment(Qt::AlignTop);
	dockWidget->setWidget(vWidget);

	auto* basicOptions = new QLabel("<h3>Input</h3>");
	vLayout->addWidget(basicOptions);
	auto* fileSelector = new QComboBox();
	auto* fileSelectorLabel = new QLabel("Input file");
	fileSelectorLabel->setBuddy(fileSelector);
	vLayout->addWidget(fileSelectorLabel);
	vLayout->addWidget(fileSelector);

	auto* fitToScreenButton = new QPushButton("Fit to screen");
	vLayout->addWidget(fitToScreenButton);

	auto renderer = new GeometryWidget();
	renderer->setDrawAxes(false);
	setCentralWidget(renderer);

	renderer->setMinZoom(0.01);
	renderer->setMaxZoom(1000.0);

	std::filesystem::path filePath("/home/steven/Downloads/test/cartocrow/data/nyc.txt");
	std::ifstream inputStream(filePath, std::ios_base::in);
	if (!inputStream.good()) {
		throw std::runtime_error("Failed to read input");
	}
	std::stringstream buffer;
	buffer << inputStream.rdbuf();
	auto nycPoints = parseCatPoints(buffer.str());

	m_gs = GeneralSettings{2.1, 2, M_PI, 70.0 / 180 * M_PI};
//	m_ps = PartitionSettings{true, true, true, true, 0.5}; todo: fix intersection delay crash
	m_ps = PartitionSettings{true, true, true, false, 0.5};
	m_ds = DrawSettings{{CB::light_blue, CB::light_red, CB::light_green, CB::light_purple, CB::light_orange}};
	m_cds = ComputeDrawingSettings{0.675};
	auto partitionList = partition(nycPoints, m_gs, m_ps, 8 * m_gs.dilationRadius());

	Number<Inexact> cover = 4.7;

	Partition* thePartition;
	for (auto& [time, partition] : partitionList) {
		if (time < cover * m_gs.dilationRadius()) {
			thePartition = &partition;
		}
	}
	m_partition = *thePartition;

//	m_dpd = std::make_shared<DilatedPatternDrawing>(m_partition, m_gs, m_cds);
//	auto ap = std::make_shared<SimpleSetsPainting>(*m_dpd, m_ds);
//	renderer->addPainting(ap, "Arrangement");
//
//	auto pp = std::make_shared<PartitionPainting>(m_partition, m_gs, m_ds);
//	renderer->addPainting(pp, "Partition");

	renderer->addPainting([this](GeometryRenderer& r) {
	  CSPolygon disk = circleToCSPolygon({{0, 0}, 1});
	  std::vector<X_monotone_curve_2> xm_curves_pl({
		  {{0, -2}, {0, 2}}
	  });
	  CSPolyline polyline(xm_curves_pl.begin(), xm_curves_pl.end());

	  r.setMode(GeometryRenderer::stroke);
	  r.setStroke(Color{0, 0, 0}, 3);
	  r.draw(renderPathFromCSPolygon(disk));
//	  r.draw(renderPathFromCSPolyline(polyline));

	  auto inters = poly_line_gon_intersection(disk, polyline);
	  r.setStroke(CB::blue, 3);
	  for (const auto& inter : inters) {
		  r.draw(renderPathFromCSPolyline(inter));
	  }
	}, "Poly-gon-line intersection");

//	auto one = m_dpd->m_dilated[0];
//	auto other = m_dpd->m_dilated[1];
//	auto comp = m_dpd->intersectionComponents(0, 1)[0];
//	auto includeExclude = m_dpd->includeExcludeDisks(0, 1, comp);

	m_cc = std::make_shared<Point<Inexact>>(2, -5);
	renderer->registerEditable(m_cc);

//	renderer->addPainting([this](GeometryRenderer& r) {
//		CSPolygonSet set;
//		CSPolygon disk1 = circleToCSPolygon({{0, 0}, 1});
//		set.join(disk1);
//	    CSPolygon disk2 = circleToCSPolygon({{0, 2.6}, 0.5});
//	    set.difference(disk2);
//	  	std::vector<CatPoint> points({{0, {-2, -2}}, {0, {2, -2}}, {0, {2, 2}}, {0, {-2, 2}}});
//	  	Island island(points);
//	  	Dilated dilated(island, 1.0);
//		set.complement();
//		set.intersection(dilated.m_contour);
//		std::vector<CSPolygonWithHoles> polys;
//		set.polygons_with_holes(std::back_inserter(polys));
//		r.draw(renderPathFromCSPolygon(polys[0].outer_boundary()));
//	    r.draw(renderPathFromCSPolygon(polys[0].holes_begin().operator*()));
//	}, "Polygon set bug?");

//	renderer->addPainting([this, includeExclude, comp](GeometryRenderer& r) {
//	    r.setMode(GeometryRenderer::fill);
////		for (const auto& exclude : includeExclude.exclude) {
////			r.setFill(CB::red);
////			r.draw(exclude);
////		}
////	    for (const auto& include : includeExclude.include) {
////		    r.setFill(CB::green);
////		    r.draw(include);
////	    }
////		auto hull = approximateConvexHull(includeExclude.include);
////	  	auto path = renderPathFromCSPolygon(hull);
////		r.draw(path);
//
//
//
//		auto inclDisks = includeExclude.include;
//		auto exclDisks = includeExclude.exclude;
//		auto componentShape = ccb_to_polygon<CSTraits>(comp.outer_ccb());
//		auto boundaryPart = boundaryParts(comp, 0)[0];
//
//		if (exclDisks.empty()) return boundaryPart;
//
//		std::vector<Circle<Exact>> lineCovering;
//		std::vector<Circle<Exact>> arcCovering;
//
//		for (const auto& d : exclDisks) {
//			auto inter = poly_line_gon_intersection(circleToCSPolygon(d), boundaryPart);
//			bool coversLine = true;
//			for (const auto& p : inter) {
//				if (!isStraight(p)) {
//					coversLine = false;
//					break;
//				}
//			}
//			if (coversLine) {
//				lineCovering.push_back(d);
//			} else {
//				arcCovering.push_back(d);
//			}
//		}
//
//		auto dr = m_gs.dilationRadius();
//		// Smoothing radius
//		auto sr = dr / 5;
//
//		std::vector<Circle<Inexact>> expandedLineCoveringDisks;
//		for (const auto& d : lineCovering) {
//			expandedLineCoveringDisks.emplace_back(approximate(d.center()), squared(sqrt(CGAL::to_double(d.squared_radius())) + sr));
//		}
//		auto diskComponents = connectedDisks(expandedLineCoveringDisks);
//
//		std::vector<CSPolygon> cuts;
//		for (const auto& comp : diskComponents) {
//			std::vector<Circle<Exact>> disks;
//			for (const auto& [i, _] : comp) {
//				disks.push_back(lineCovering[i]);
//			}
//			cuts.push_back(approximateConvexHull(disks));
//		}
//		for (const auto& d : arcCovering) {
//			cuts.push_back(circleToCSPolygon(d));
//		}
//
//		CSPolygonSet polygonSet;
//		for (const auto& cut : cuts) {
//			polygonSet.join(cut);
//		}
//		for (const auto& d: inclDisks) {
//			polygonSet.difference(circleToCSPolygon(d));
//		}
//		for (const auto& d: exclDisks) {
//			if (on_or_inside(componentShape, d.center())) {
//				auto n = nearest(boundaryPart, d.center());
//				auto rect = thinRectangle(d.center(), n, m_gs.pointSize / 5);
//				polygonSet.join(rect);
//			}
//		}
//		polygonSet.complement();
////		polygonSet.intersection(componentShape);
//
//		std::vector<CSPolygonWithHoles> polys;
//		polygonSet.polygons_with_holes(std::back_inserter(polys));
//		//	CGAL::difference(componentShape, )
//		assert(polys.size() == 1);
//		auto poly = polys[0];
//		//	std
////		assert(!poly.has_holes());
//		std::cout << "#holes: " << poly.number_of_holes() << std::endl;
//
//		auto outer = *poly.holes_begin();
//		for (auto cit = outer.curves_begin(); cit != outer.curves_end(); ++cit) {
//			std::cout << cit->source() << " -> " << cit->target() << std::endl;
//			if (cit->is_linear()) {
//				std::cout << cit->supporting_line() << std::endl;
//			} else {
//				std::cout << cit->supporting_circle() << std::endl;
//			}
//		}
//
//
//		r.setFill(CB::light_purple);
//		r.draw(renderPathFromCSPolygon(outer));
//
//	}, "Include exclude");

//	renderer->addPainting([this](GeometryRenderer& r) {
////		X_monotone_curve_2 bot({0, 0}, {5, 0});
////	    X_monotone_curve_2 right({5, 0}, {5, 5});
////	    X_monotone_curve_2 top({5, 5}, {0, 5});
////	  	X_monotone_curve_2 left({0, 5}, {0, 0});
//		std::vector<CatPoint> points({{0, {0, 0}}, {0, {5, -3}}, {0, {5, 5}}, {0, {2, 5}}});
//		Island island(points);
//		Dilated dilated(island, 1.0);
//		auto polygon = dilated.m_contour;
////		Polygon<Exact> polygon(points.begin(), points.end());
////		auto csPolygon = dil
//
////		std::vector<X_monotone_curve_2> xm_curves({bot, right, top, left});
////		CSPolygon polygon(xm_curves.begin(), xm_curves.end());
//		r.setMode(GeometryRenderer::fill);
//		r.setFill(Color{100, 100, 100});
//		r.draw(renderPathFromCSPolygon(polygon));
//
//	    std::vector<X_monotone_curve_2> xm_curves_bp(polygon.curves_begin(), std::next(polygon.curves_begin(), 6));
//
////		auto xm_curves_bp
//		CSPolyline polyline(xm_curves_bp.begin(), xm_curves_bp.end());
//		auto morphed = morph(polyline, polygon, {}, {Circle<Exact>({m_cc->x(), m_cc->y()}, 1)}, m_gs, m_cds);
//	    r.setMode(GeometryRenderer::stroke);
//	    r.setStroke(Color{0, 0, 0}, 3.0);
//		r.draw(renderPathFromCSPolyline(morphed));
//		r.draw(approximateAlgebraic(nearest(polyline, {m_cc->x(), m_cc->y()})));
//		r.draw(*m_cc);
//	}, "Morph test");

	connect(fitToScreenButton, &QPushButton::clicked, [nycPoints, renderer, this]() {
		std::vector<Point<Inexact>> pts;
		std::transform(nycPoints.begin(), nycPoints.end(), std::back_inserter(pts),
		               [](const CatPoint& pt) { return pt.point; });
		Box box = CGAL::bbox_2(pts.begin(), pts.end());
		auto delta = 2 * m_gs.dilationRadius();
		Box expanded(box.xmin() - delta, box.ymin() - delta, box.xmax() + delta, box.ymax() + delta);
		renderer->fitInView(expanded);
	});
}

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	SimpleSetsDemo demo;
	demo.show();
	app.exec();
}
