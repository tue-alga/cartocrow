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
#include <QPushButton>
#include <filesystem>
#include <ipepath.h>
#include <ranges>
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
	setWindowTitle("Isoline simplification");

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

	std::cout << "Loading: " << fileSelector->currentText().toStdString() << std::endl;
	std::shared_ptr<ipe::Document> document = IpeReader::loadIpeFile(dir + fileSelector->currentText().toStdString() + ".ipe");
	ipe::Page* page = document->page(0);
	m_isoline_simplifier = std::make_unique<IsolineSimplifier>(isolinesInPage(page));

	auto* number_of_vertices = new QLabel(QString(("#Vertices: " + std::to_string(m_isoline_simplifier->m_current_complexity)).c_str()));
	vLayout->addWidget(number_of_vertices);

	auto* debugInfo = new QCheckBox("Debug info");
	//	debugInfo->setCheckState(Qt::Checked);
	vLayout->addWidget(debugInfo);

	auto* doCGALSimplify = new QCheckBox("CGAL simplify");
	vLayout->addWidget(doCGALSimplify);

	auto* simplificationTarget = new QSpinBox();
	simplificationTarget->setValue(50);
	simplificationTarget->setMaximum(1000000);
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

	auto* isolineIndex = new QSpinBox();
	isolineIndex->setValue(0);
	isolineIndex->setMaximum(10000);
	auto* isolineIndexLabel = new QLabel("Isoline index");
	isolineIndexLabel->setBuddy(isolineIndex);
	vLayout->addWidget(isolineIndexLabel);
	vLayout->addWidget(isolineIndex);

	auto* showGrid = new QCheckBox("Show grid");
	vLayout->addWidget(showGrid);

	auto* showVertices = new QCheckBox("Show isoline vertices");
	vLayout->addWidget(showVertices);

	auto* simplify_button = new QPushButton("Simplify");
	vLayout->addWidget(simplify_button);

	auto* step_button = new QPushButton("Step && update");
	vLayout->addWidget(step_button);

	auto* step_only_button = new QPushButton("Step");
	vLayout->addWidget(step_only_button);

	auto* update_sm_button = new QPushButton("Update matching");
	vLayout->addWidget(update_sm_button);

	auto* update_sl_button = new QPushButton("Update slope ladders");
	vLayout->addWidget(update_sl_button);

	auto* debug_text = new QLabel("");
	vLayout->addWidget(debug_text);

	m_renderer = new GeometryWidget();
	m_renderer->setDrawAxes(showGrid->checkState());
	setCentralWidget(m_renderer);

	m_renderer->setMinZoom(0.01);
	m_renderer->setMaxZoom(1000.0);

	m_recalculate = [this, debugInfo, doCGALSimplify, simplificationTarget, regionIndex, showVertices, isolineIndex, number_of_vertices]() {
		number_of_vertices->setText(QString(("#Vertices: " + std::to_string(m_isoline_simplifier->m_current_complexity)).c_str()));
		recalculate(debugInfo->checkState(), simplificationTarget->value(),
		            doCGALSimplify->checkState(), regionIndex->value(), showVertices->checkState(), isolineIndex->value());
	};

	connect(fileSelector, &QComboBox::currentTextChanged, [this, dir](const QString& name) {
		std::shared_ptr<ipe::Document> document = IpeReader::loadIpeFile(dir + name.toStdString() + ".ipe");
		ipe::Page* page = document->page(0);
		m_isoline_simplifier = std::make_unique<IsolineSimplifier>(isolinesInPage(page));
		m_debug_ladder = std::nullopt;
		m_recalculate();
	});
	connect(debugInfo, &QCheckBox::stateChanged, m_recalculate);
	connect(simplificationTarget, QOverload<int>::of(&QSpinBox::valueChanged), m_recalculate);
	connect(doCGALSimplify, &QCheckBox::stateChanged, m_recalculate);
	connect(regionIndex, QOverload<int>::of(&QSpinBox::valueChanged), m_recalculate);
	connect(showGrid, &QCheckBox::stateChanged, [this](bool v) { m_renderer->setDrawAxes(v); });
	connect(showVertices, &QCheckBox::stateChanged, m_recalculate);
	connect(isolineIndex, QOverload<int>::of(&QSpinBox::valueChanged), m_recalculate);
	connect(step_button, &QPushButton::clicked, [this]() {
		bool progress = m_isoline_simplifier->step();
		if (progress) {
			m_isoline_simplifier->update_matching();
			m_isoline_simplifier->update_ladders();
		}
	  	m_recalculate();
	});
	connect(step_only_button, &QPushButton::clicked, [this]() {
		m_isoline_simplifier->step();
		m_recalculate();
	});
	connect(update_sm_button, &QPushButton::clicked, [this]() {
		m_isoline_simplifier->update_matching();
		m_recalculate();
	});
	connect(update_sl_button, &QPushButton::clicked, [this]() {
		m_isoline_simplifier->update_ladders();
		m_recalculate();
	});
	connect(simplify_button, &QPushButton::clicked, [this, simplificationTarget]() {
		m_isoline_simplifier->simplify(simplificationTarget->value());
		m_recalculate();
	});
	connect(m_renderer, &GeometryWidget::clicked, [this, debug_text](auto pt) {
		auto it = std::find_if(m_isoline_simplifier->m_slope_ladders.begin(), m_isoline_simplifier->m_slope_ladders.end(), [&pt](const std::shared_ptr<SlopeLadder>& ladder) {
			if (ladder->m_old) return false;
			auto poly = slope_ladder_polygon(*ladder);
			return poly.has_on_bounded_side(pt);
		});
		if (it == m_isoline_simplifier->m_slope_ladders.end() || *it == m_debug_ladder) {
			m_debug_ladder = std::nullopt;
			debug_text->setText("");
		} else {
			auto ladder = *it;
			m_debug_ladder = ladder;

			auto valid_text = "Valid: " + std::to_string(ladder->m_valid);
			auto intersected_text = !ladder->m_valid ? "" : "\nIntersected: " + std::to_string(m_isoline_simplifier->check_ladder_intersections_Voronoi(*ladder).has_value());
			auto cost_text = "\nCost: " + std::to_string(ladder->m_cost);
			auto old_text = "\nOld: " + std::to_string(ladder->m_old);

			debug_text->setText(QString((valid_text + intersected_text + cost_text + old_text).c_str()));
		}

		m_recalculate();
	});

	m_recalculate();
}

void IsolineSimplificationDemo::recalculate(bool debugInfo, int target, bool cgal_simplify, int region_index, bool show_vertices, int isoline_index) {
	// todo: split into repaint and separate recalculations like simplification, debugInfo
	m_renderer->clear();
	m_cgal_simplified.clear();
	IpeRenderer ipe_renderer;

	std::vector<Isoline<K>>& original_isolines = m_isoline_simplifier->m_simplified_isolines;
	std::vector<Isoline<K>>& simplified_isolines = m_isoline_simplifier->m_simplified_isolines;

	auto medial_axis_p = std::make_shared<VoronoiPainting>(m_isoline_simplifier->m_delaunay);
	if (debugInfo) {
		m_renderer->addPainting(medial_axis_p, "Voronoi diagram");
		ipe_renderer.addPainting(medial_axis_p, "Voronoi diagram");
	}

	const auto& separator = m_isoline_simplifier->m_separator;

	auto separator_p = std::make_shared<MedialAxisSeparatorPainting>(separator, m_isoline_simplifier->m_delaunay);

	auto matching_p = std::make_shared<MatchingPainting>(m_isoline_simplifier->m_matching,
	                                                     [this, &simplified_isolines, isoline_index](Gt::Point_2 pt) {
		                                                     return m_isoline_simplifier->m_p_isoline.contains(pt) && m_isoline_simplifier->m_p_isoline.at(pt) == &simplified_isolines[std::min(static_cast<int>(simplified_isolines.size() - 1), isoline_index)];
	                                                     });

	auto slope_ladder_p = std::make_shared<SlopeLadderPainting>(m_isoline_simplifier->m_slope_ladders);
	auto collapse_p = std::make_shared<CollapsePainting>(*m_isoline_simplifier);
//	auto changed_p = std::make_shared<ChangedPainting>(*m_isoline_simplifier.);

	if (debugInfo) {
		m_renderer->addPainting(matching_p, "Matching");
		if (!m_isoline_simplifier->m_started) {
			m_renderer->addPainting(separator_p, "Separator");
			ipe_renderer.addPainting(separator_p, "Separator");
		}
		m_renderer->addPainting(slope_ladder_p, "Slope ladders");

		ipe_renderer.addPainting(slope_ladder_p, "Slope ladders");
	}

	auto original_isolines_p = std::make_shared<IsolinePainting>(m_isoline_simplifier->m_isolines, show_vertices, true);
	auto isolines_p = std::make_shared<IsolinePainting>(simplified_isolines, show_vertices, false);
	if (m_isoline_simplifier->m_started)
		m_renderer->addPainting(original_isolines_p, "Original isolines");
	m_renderer->addPainting(isolines_p, "Simplified isolines");
	ipe_renderer.addPainting(isolines_p, "Simplified isolines");

	if (cgal_simplify) {
		CT ct;

		std::vector<CT::Constraint_id> ids;

		for (const auto& isoline : original_isolines) {
			if (isoline.m_closed) {
				CT::Constraint_id id = ct.insert_constraint(isoline.polygon());
				ids.push_back(id);
			} else {
				CT::Constraint_id id = ct.insert_constraint(isoline.m_points);
				ids.push_back(id);
			}
		}

		PS::simplify(ct, Cost(), Stop(target));
		for(auto cit = ct.constraints_begin(); cit != ct.constraints_end(); ++cit) {
			CT::Constraint_id id = *cit;
			auto i = std::distance(ids.begin(), std::find(ids.begin(), ids.end(), id));

			std::vector<Point<K>> simplified_points;
			for (auto vit = ct.points_in_constraint_begin(*cit); vit != ct.points_in_constraint_end(*cit); ++vit) {
				simplified_points.push_back(*vit);
			}
			m_cgal_simplified.emplace_back(simplified_points, simplified_isolines[i].m_closed);
		}

		auto cgal_simplified_p = std::make_shared<IsolinePainting>(m_cgal_simplified, show_vertices, false);
		m_renderer->addPainting(cgal_simplified_p, "CGAL simplified isolines");
		ipe_renderer.addPainting(cgal_simplified_p, "CGAL simplified isolines");
	}

	if (!m_isoline_simplifier->m_started && debugInfo && region_index < simplified_isolines.size() && separator.contains(&simplified_isolines[region_index])) {
		auto touched_p = std::make_shared<TouchedPainting>(separator.at(&simplified_isolines[region_index]), m_isoline_simplifier->m_delaunay);
		m_renderer->addPainting(touched_p, "Touched");
	}

	if (debugInfo) {
		m_renderer->addPainting(collapse_p, "Collapse");
		ipe_renderer.addPainting(collapse_p, "Collapse");
	}

	if (m_debug_ladder.has_value()) {
		auto ladder = *m_debug_ladder;
		auto debug_ladder_p = std::make_shared<DebugLadderPainting>(*m_isoline_simplifier, *ladder);
		m_renderer->addPainting(debug_ladder_p, "Debug ladder painting");
	}

	m_renderer->update();

//	ipe_renderer.save("/home/steven/Documents/cartocrow/output.ipe");
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

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	IsolineSimplificationDemo demo;
	demo.show();
	app.exec();
}

//int main() {
//	auto dir = std::string("/home/steven/Documents/cartocrow/inputs/");
//
//	std::shared_ptr<ipe::Document> document = IpeReader::loadIpeFile(dir + "output.ipe");
//	ipe::Page* page = document->page(0);
//	auto simplifier = IsolineSimplifier(isolinesInPage(page));
//
//	auto start = std::chrono::system_clock::now().time_since_epoch();
//	simplifier.simplify(30000);
//	auto end = std::chrono::system_clock::now().time_since_epoch();
//
//	const std::chrono::duration<double> duration = end - start;
//
//	std::cout << duration.count() << std::endl;
//}

VoronoiPainting::VoronoiPainting(const SDG2& delaunay): m_delaunay(delaunay) {}

void VoronoiPainting::paint(GeometryRenderer& renderer) const {
	renderer.setStroke(Color(150, 150, 150), 1);
	renderer.setMode(GeometryRenderer::stroke);

	renderer.setStroke(Color(150, 150, 150), 1);
	auto voronoiDrawer = VoronoiDrawer<Gt>(&renderer);
	draw_dual<VoronoiDrawer<Gt>, K>(m_delaunay, voronoiDrawer);
}

IsolinePainting::IsolinePainting(const std::vector<Isoline<K>>& isolines, bool show_vertices, bool light):
      m_isolines(isolines), m_show_vertices(show_vertices), m_light(light) {}

void IsolinePainting::paint(GeometryRenderer& renderer) const {
	// Draw isolines
	if (m_show_vertices) {
		renderer.setMode(GeometryRenderer::stroke | GeometryRenderer::vertices);
	} else {
		renderer.setMode(GeometryRenderer::stroke);
	}
	if (m_light) {
		renderer.setStroke(Color(150, 150, 150), 1);
	} else {
		renderer.setStroke(Color(0, 0, 0), 1);
	}
	for (const auto& isoline : m_isolines) {
		std::visit([&](auto&& v){
			renderer.draw(v);
		}, isoline.drawing_representation());
	}
}

MedialAxisSeparatorPainting::MedialAxisSeparatorPainting(const Separator& separator, const SDG2& delaunay):
      m_separator(separator), m_delaunay(delaunay) {}

void MedialAxisSeparatorPainting::paint(GeometryRenderer& renderer) const {
	auto voronoiDrawer = VoronoiDrawer<Gt>(&renderer);
	for (const auto& [_, edges] : m_separator)
	for (const auto& edge : edges) {
		renderer.setStroke(Color(0, 0, 255), 2.5);
		renderer.setMode(GeometryRenderer::stroke);
		draw_dual_edge<VoronoiDrawer<Gt>, K>(m_delaunay, edge, voronoiDrawer);
	}
}

MatchingPainting::MatchingPainting(Matching& matching, std::function<bool(Gt::Point_2)> predicate):
      m_matching(matching), m_predicate(std::move(predicate)) {}

void MatchingPainting::paint(GeometryRenderer& renderer) const {
	renderer.setMode(GeometryRenderer::stroke);
	for (const auto& [p, matched_to] : m_matching) {
		if (!m_predicate(p)) continue;
		renderer.setStroke(Color(50, 200, 200), 3);
		if (matched_to.contains(CGAL::LEFT_TURN)) {
			for (const auto& [_, pts] : matched_to.at(CGAL::LEFT_TURN))
			for (const auto& q : pts) {
				renderer.draw(Segment<K>(p, q));
			}
		}
		renderer.setStroke(Color(200, 50, 200), 3);
		if (matched_to.contains(CGAL::RIGHT_TURN)) {
			for (const auto& [_, pts] : matched_to.at(CGAL::RIGHT_TURN))
			for (const auto& q : pts) {
				renderer.draw(Segment<K>(p, q));
			}
		}
	}
}

TouchedPainting::TouchedPainting(std::vector<SDG2::Edge> edges, const SDG2& delaunay):
      m_edges(std::move(edges)), m_delaunay(delaunay) {}

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

SlopeLadderPainting::SlopeLadderPainting(const std::vector<std::shared_ptr<SlopeLadder>>& slope_ladders):
      m_slope_ladders(slope_ladders) {}

Polygon<K> slope_ladder_polygon(const SlopeLadder& slope_ladder) {
	std::vector<Gt::Point_2> pts;
	for (const auto& p : slope_ladder.m_rungs) {
		pts.push_back(p.source());
	}
	if (slope_ladder.m_cap.contains(CGAL::RIGHT_TURN)) {
		pts.push_back(slope_ladder.m_cap.at(CGAL::RIGHT_TURN));
	}
	for (auto& rung : std::ranges::reverse_view(slope_ladder.m_rungs)) {
		pts.push_back(rung.target());
	}
	if (slope_ladder.m_cap.contains(CGAL::LEFT_TURN)) {
		pts.push_back(slope_ladder.m_cap.at(CGAL::LEFT_TURN));
	}
	return Polygon<K>(pts.begin(), pts.end());
}

void draw_slope_ladder(GeometryRenderer& renderer, const SlopeLadder& slope_ladder) {
	auto poly = slope_ladder_polygon(slope_ladder);
	renderer.setFill(Color(100, 0, 0));
	renderer.setFillOpacity(20);
	renderer.setStroke(Color(255, 20, 20), 1.0);
	renderer.setMode(GeometryRenderer::fill | GeometryRenderer::stroke);
	renderer.draw(poly);
}

void draw_ladder_collapse(GeometryRenderer& renderer, IsolineSimplifier simplifier, const SlopeLadder& slope_ladder) {
	for (int i = 0; i < slope_ladder.m_rungs.size(); i++) {
		const auto& rung = slope_ladder.m_rungs.at(i);
		const auto& p = slope_ladder.m_collapsed.at(i);
		auto reversed = simplifier.m_p_next.contains(rung.target()) && simplifier.m_p_next.at(rung.target()) == rung.source();
		auto t = reversed ? rung.target() : rung.source();
		auto u = reversed ? rung.source() : rung.target();
		auto s = simplifier.m_p_prev.at(t);
		auto v = simplifier.m_p_next.at(u);
		auto l = area_preservation_line(s, t, u, v);
		renderer.setStroke(Color(60, 60, 60), 2.0);
		renderer.draw(l);

		renderer.setStroke(Color(255, 165, 0), 4.0);
		renderer.draw(Gt::Segment_2(s, p));
		renderer.draw(Gt::Segment_2(p, v));
		renderer.draw(p);
	}
}

void SlopeLadderPainting::paint(GeometryRenderer& renderer) const {
	for (const auto& slope_ladder : m_slope_ladders) {
		if (slope_ladder->m_old) continue;
		draw_slope_ladder(renderer, *slope_ladder);
	}
}

CollapsePainting::CollapsePainting(IsolineSimplifier& simplifier):
      m_simplifier(simplifier) {}

void CollapsePainting::paint(GeometryRenderer& renderer) const {
//	if (!m_simplifier.m_slope_ladders.empty()) {
//		auto next = m_simplifier.next_ladder();
//		if (!next.has_value()) return;
//		const auto& slope_ladder = *next;
//		if (!slope_ladder->m_old) {
//			renderer.setStroke(Color(255, 20, 20), 2.0);
//			draw_slope_ladder(renderer, *slope_ladder);
//			draw_ladder_collapse(renderer, m_simplifier, *slope_ladder);
//		}
//	}

	for (const auto& v : m_simplifier.m_changed_vertices) {
		const auto& site = v->site();
		renderer.setStroke(Color(0, 200, 0), 5.0);
		if (site.is_point()) {
			renderer.draw(site.point());
		} else {
			renderer.draw(site.segment());
		}
	}
}

DebugLadderPainting::DebugLadderPainting(IsolineSimplifier& simplifier, SlopeLadder& ladder):
	m_simplifier(simplifier), m_ladder(ladder) {}

void DebugLadderPainting::paint(GeometryRenderer& renderer) const {
	draw_slope_ladder(renderer, m_ladder);
	if (!m_ladder.m_valid) return;
	draw_ladder_collapse(renderer, m_simplifier, m_ladder);
}
