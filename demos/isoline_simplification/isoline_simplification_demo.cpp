/*
The CartoCrow library implements algorithmic geo-visualization methods,
developed at TU Eindhoven.
Copyright (C) 2021  Netherlands eScience Center and TU Eindhoven

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3f of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include "isoline_simplification_demo.h"
#include "cartocrow/core/ipe_reader.h"
#include "cartocrow/isoline_simplification/isoline.h"
#include "cartocrow/isoline_simplification/medial_axis_separator.h"
#include "cartocrow/renderer/ipe_renderer.h"
#include "medial_axis_helpers.h"
#include "voronoi_drawer.h"
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Constrained_triangulation_plus_2.h>
#include <CGAL/Polyline_simplification_2/simplify.h>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDockWidget>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>
#include <filesystem>
#include <ipepath.h>
#include <utility>

namespace PS = CGAL::Polyline_simplification_2;
typedef PS::Stop_below_count_threshold Stop;
typedef PS::Squared_distance_cost      Cost;

typedef PS::Vertex_base_2<K> Vb;
typedef CGAL::Constrained_triangulation_face_base_2<K> Fb;
typedef CGAL::Triangulation_data_structure_2<Vb, Fb> TDS;
typedef CGAL::Constrained_Delaunay_triangulation_2<K, TDS, CGAL::Exact_predicates_tag> CDT;
typedef CGAL::Constrained_triangulation_plus_2<CDT>     CT;

namespace fs = std::filesystem;
using namespace cartocrow;
using namespace cartocrow::renderer;
using namespace cartocrow::isoline_simplification;

IsolineSimplificationDemo::IsolineSimplificationDemo() {
	auto dir = std::string("/home/steven/Documents/cartocrow/inputs/");
	setWindowTitle("Medial Axis");

	auto* dockWidget = new QDockWidget();
	addDockWidget(Qt::RightDockWidgetArea, dockWidget);
	auto* vWidget = new QWidget();
	auto* vLayout = new QVBoxLayout(vWidget);
	vLayout->setAlignment(Qt::AlignTop);
	dockWidget->setWidget(vWidget);

	auto* fileSelector = new QComboBox();
	vLayout->addWidget(fileSelector);
	std::string ext(".ipe");
	for (auto &p : fs::recursive_directory_iterator(dir))
	{
		if (p.path().extension() == ext)
			fileSelector->addItem(QString::fromStdString(p.path().stem().string()));
	}

	auto* doMedial = new QCheckBox("Compute medial axis");
	//	doMedial->setCheckState(Qt::Checked);
	vLayout->addWidget(doMedial);

	auto* doCGALSimplify = new QCheckBox("CGAL simplify");
	vLayout->addWidget(doCGALSimplify);

	auto* simplificationTarget = new QSpinBox();
	simplificationTarget->setValue(50);
	simplificationTarget->setMaximum(10000);
	auto* simplificationTargetLabel = new QLabel("#Target vertices");
	simplificationTargetLabel->setBuddy(simplificationTarget);
	vLayout->addWidget(simplificationTargetLabel);
	vLayout->addWidget(simplificationTarget);

	auto* regionIndex = new QSpinBox();
	regionIndex->setValue(0);
	regionIndex->setMaximum(20);
	auto* regionIndexLabel = new QLabel("Region index");
	regionIndexLabel->setBuddy(regionIndex);
	vLayout->addWidget(regionIndexLabel);
	vLayout->addWidget(regionIndex);

	auto* showGrid = new QCheckBox("Show grid");
	vLayout->addWidget(showGrid);

	auto* showVertices = new QCheckBox("Show isoline vertices");
	vLayout->addWidget(showVertices);

	m_recalculate = [this, doMedial, doCGALSimplify, simplificationTarget, regionIndex, showVertices]() {
		recalculate(doMedial->checkState(), simplificationTarget->value(),
					doCGALSimplify->checkState(), regionIndex->value(), showVertices->checkState());
	};

	connect(fileSelector, &QComboBox::currentTextChanged, [this, dir](const QString& name) {
	    std::shared_ptr<ipe::Document> document = IpeReader::loadIpeFile(dir + name.toStdString() + ".ipe");
	    ipe::Page* page = document->page(0);
	    m_isolines = isolinesInPage(page);
		m_recalculate();
	});

	connect(doMedial, &QCheckBox::stateChanged, m_recalculate);
	connect(simplificationTarget, QOverload<int>::of(&QSpinBox::valueChanged), m_recalculate);
	connect(doCGALSimplify, &QCheckBox::stateChanged, m_recalculate);
	connect(regionIndex, QOverload<int>::of(&QSpinBox::valueChanged), m_recalculate);
	connect(showGrid, &QCheckBox::stateChanged, [this](bool v) { m_renderer->setDrawAxes(v); });
	connect(showVertices, &QCheckBox::stateChanged, m_recalculate);

	std::shared_ptr<ipe::Document> document = IpeReader::loadIpeFile(dir + fileSelector->currentText().toStdString() + ".ipe");
	ipe::Page* page = document->page(0);
	m_isolines = isolinesInPage(page);

	m_renderer = new GeometryWidget();
	m_renderer->setDrawAxes(showGrid->checkState());
	setCentralWidget(m_renderer);
	m_recalculate();
}

void IsolineSimplificationDemo::recalculate(bool voronoi, int target, bool cgal_simplify, int region_index, bool show_vertices) {
	// todo: split into repaint and separate recalculations like simplification, voronoi
	m_renderer->clear();
	m_delaunay.clear();
	m_cgal_simplified.clear();

	if (voronoi) {
		for (const auto& isoline : m_isolines) {
			addIsolineToVoronoi(isoline);
		}
	}
	auto medial_axis_p = std::make_shared<MedialAxisPainting>(m_delaunay);
	if (voronoi) {
		m_renderer->addPainting(medial_axis_p, "Medial axis");
	}
	CT ct;

	std::vector<CT::Constraint_id> ids;

	for (const auto& isoline : m_isolines) {
		if (isoline.m_closed) {
			CT::Constraint_id id = ct.insert_constraint(isoline.polygon());
			ids.push_back(id);
		} else {
			CT::Constraint_id id = ct.insert_constraint(isoline.m_points);
			ids.push_back(id);
		}
	}

	m_separator = medial_axis_separator(m_delaunay, m_isolines);
	auto separator_p = std::make_shared<MedialAxisSeparatorPainting>(m_separator, m_delaunay);

	m_matching = matching(m_delaunay, m_separator);
	auto matching_p = std::make_shared<MatchingPainting>(m_matching);

	if (voronoi) {
		m_renderer->addPainting(matching_p, "Matching");
		m_renderer->addPainting(separator_p, "Separator");
	}

	auto isolines_p = std::make_shared<IsolinePainting>(m_isolines, show_vertices);
	m_renderer->addPainting(isolines_p, "Isolines");

	if (cgal_simplify) {
		PS::simplify(ct, Cost(), Stop(target));
		for(auto cit = ct.constraints_begin(); cit != ct.constraints_end(); ++cit) {
			CT::Constraint_id id = *cit;
			auto i = std::distance(ids.begin(), std::find(ids.begin(), ids.end(), id));

			std::vector<Point<K>> simplified_points;
			for (auto vit = ct.points_in_constraint_begin(*cit); vit != ct.points_in_constraint_end(*cit); ++vit) {
				simplified_points.push_back(*vit);
			}
			m_cgal_simplified.emplace_back(std::move(simplified_points), m_isolines[i].m_closed);
		}

		auto cgal_simplified_p = std::make_shared<IsolinePainting>(m_cgal_simplified, show_vertices);
		m_renderer->addPainting(cgal_simplified_p, "CGAL simplified isolines");
	}

	if (voronoi && region_index < m_isolines.size() && m_separator.contains(&m_isolines[region_index])) {
		auto touched_p = std::make_shared<TouchedPainting>(m_separator[&m_isolines[region_index]], m_delaunay);
		m_renderer->addPainting(touched_p, "Touched");
	}

	m_renderer->update();

//	IpeRenderer ipeRenderer;
//	ipeRenderer.addPainting(medial_axis_p);
//	ipeRenderer.addPainting(isolines_p);
//	ipeRenderer.save("/home/steven/Documents/cartocrow/output.ipe");
}

std::vector<Isoline<K>> isolinesInPage(ipe::Page* page) {
	auto isolines = std::vector<Isoline<K>>();

	for (int i = 0; i < page->count(); i++) {
		auto object = page->object(i);
		if (object->type() != ipe::Object::Type::EPath) continue;
		auto path = object->asPath();
		auto matrix = object->matrix();
		auto shape = path->shape();
		for (int j = 0; j < shape.countSubPaths(); j++) {
			auto subpath = shape.subPath(j);
			if (subpath->type() != ipe::SubPath::Type::ECurve) continue;
			auto curve = subpath->asCurve();

			std::vector<Point<K>> points;

			for (int k = 0; k < curve->countSegments(); k++) {
				auto segment = curve->segment(k);
				auto start = matrix * segment.cp(0);
				points.emplace_back(start.x, start.y);
			}

			auto last = matrix * curve->segment(curve->countSegments()-1).last();
			points.emplace_back(last.x, last.y);

			isolines.emplace_back(points, curve->closed());
		}
	}

	return isolines;
}

void IsolineSimplificationDemo::addIsolineToVoronoi(const Isoline<K>& isoline) {
	m_delaunay.insert_segments(isoline.edges_begin(), isoline.edges_end());
}

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	IsolineSimplificationDemo demo;
	demo.show();
	app.exec();
}

MedialAxisPainting::MedialAxisPainting(SDG2& delaunay): m_delaunay(delaunay) {}

void MedialAxisPainting::paint(GeometryRenderer& renderer) const {
	renderer.setStroke(Color(150, 150, 150), 1);
	renderer.setMode(GeometryRenderer::stroke);

	renderer.setStroke(Color(150, 150, 150), 1);
	auto voronoiDrawer = VoronoiDrawer<Gt>(&renderer);
	draw_skeleton<VoronoiDrawer<Gt>, K>(m_delaunay, voronoiDrawer);
}

IsolinePainting::IsolinePainting(std::vector<Isoline<K>>& isolines, bool show_vertices):
      m_isolines(isolines), m_show_vertices(show_vertices) {}

void IsolinePainting::paint(GeometryRenderer& renderer) const {
	// Draw isolines
	if (m_show_vertices) {
		renderer.setMode(GeometryRenderer::stroke | GeometryRenderer::vertices);
	} else {
		renderer.setMode(GeometryRenderer::stroke);
	}
	renderer.setStroke(Color(0, 0, 0), 2);
	for (const auto& isoline : m_isolines) {
		std::visit([&](auto&& v){
			renderer.draw(v);
		}, isoline.m_drawing_representation);
	}
}

MedialAxisSeparatorPainting::MedialAxisSeparatorPainting(Separator& separator, SDG2& delaunay):
      m_separator(separator), m_delaunay(delaunay) {}

void MedialAxisSeparatorPainting::paint(GeometryRenderer& renderer) const {
	for (auto& [_, edges] : m_separator)
	for (auto edge : edges) {
		renderer.setStroke(Color(0, 0, 255), 2.5);
		renderer.setMode(GeometryRenderer::stroke);
		auto voronoiDrawer = VoronoiDrawer<Gt>(&renderer);
		draw_dual_edge<VoronoiDrawer<Gt>, K>(m_delaunay, edge, voronoiDrawer);
	}
}

MatchingPainting::MatchingPainting(std::vector<Segment<K>>& matching):
      m_matching(matching) {}

void MatchingPainting::paint(GeometryRenderer& renderer) const {
	renderer.setMode(GeometryRenderer::stroke);
	renderer.setStroke(Color(50, 200, 200), 1);
	for (const auto& s : m_matching) {
		renderer.draw(s);
	}
}

TouchedPainting::TouchedPainting(std::vector<SDG2::Edge>& edges, SDG2& delaunay):
      m_edges(edges), m_delaunay(delaunay) {}

void TouchedPainting::paint(GeometryRenderer& renderer) const {
	for (auto edge : m_edges) {
		auto draw_site = [&renderer, &edge, this](SDG2::Site_2 site) {
			auto point_or_segment = site_projection(m_delaunay, edge, site);
			std::visit([&renderer](auto v) { renderer.draw(v); }, point_or_segment);
		};

		SDG2::Site_2 p = edge.first->vertex(SDG2::cw(edge.second))->site();
		SDG2::Site_2 q = edge.first->vertex(SDG2::ccw(edge.second))->site();

		renderer.setStroke(Color(0, 255, 0), 4.0);
		draw_site(p);
		draw_site(q);
	}
}