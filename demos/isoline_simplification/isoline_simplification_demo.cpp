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
#include "cartocrow/isoline_simplification/ipe_bezier_wrapper.h"
#include "cartocrow/renderer/ipe_renderer.h"
#include "medial_axis_helpers.h"
#include "voronoi_drawer.h"
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

namespace fs = std::filesystem;
using namespace cartocrow;
using namespace cartocrow::renderer;
using namespace cartocrow::isoline_simplification;

IsolineSimplificationDemo::IsolineSimplificationDemo() {
	std::string dir("/home/steven/Documents/cartocrow/inputs/small/");
	std::string output_dir("/home/steven/Documents/cartocrow/output/");
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

	auto* collapse_selector = new QComboBox();
	collapse_selector->addItem("Midpoint");
	collapse_selector->addItem("Minimize symmetric difference");
	collapse_selector->addItem("Spline midpoint");
	collapse_selector->addItem("Spline min. symmetric diff.");
	vLayout->addWidget(collapse_selector);

	auto* angle_filter_input = new QDoubleSpinBox();
	angle_filter_input->setValue(M_PI/6);
	angle_filter_input->setMinimum(0);
	angle_filter_input->setMaximum(M_PI);
	auto* angle_filter_input_label = new QLabel("Angle filter");
	angle_filter_input_label->setBuddy(angle_filter_input);
	vLayout->addWidget(angle_filter_input_label);
	vLayout->addWidget(angle_filter_input);

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

	auto* reload_button = new QPushButton("Reload");
	vLayout->addWidget(reload_button);

	auto* save_button = new QPushButton("Save");
	vLayout->addWidget(save_button);

	auto* debug_text = new QLabel("");
	vLayout->addWidget(debug_text);

	auto* measure_text = new QLabel("");
	vLayout->addWidget(measure_text);

	m_renderer = new GeometryWidget();
	m_renderer->setDrawAxes(showGrid->checkState());
	setCentralWidget(m_renderer);

	m_renderer->setMinZoom(0.01);
	m_renderer->setMaxZoom(1000.0);

	m_save = [this, showVertices, output_dir, fileSelector, collapse_selector, measure_text]() {
		auto& isolines = m_isoline_simplifier->m_simplified_isolines;
	  	IpeRenderer ipe_renderer;
	    auto isolines_p = std::make_shared<IsolinePainting>(isolines, showVertices->isChecked(), false, true);
	    ipe_renderer.addPainting(isolines_p, "Simplified_isolines");
		std::string file_name(fileSelector->currentText().toStdString() + "_" +
		                      std::to_string(m_isoline_simplifier->m_current_complexity) + "_" +
		                      collapse_selector->currentText().toStdString());
	   	ipe_renderer.save(output_dir + file_name + ".ipe");
		std::ofstream meta_data_file;
		meta_data_file.open(output_dir + file_name + "_meta" + ".txt");
		meta_data_file << measure_text->text().toStdString();
		meta_data_file.close();
	};

	m_recalculate = [this, debugInfo, doCGALSimplify, simplificationTarget, regionIndex, showVertices, isolineIndex, number_of_vertices, angle_filter_input, measure_text]() {
		number_of_vertices->setText(QString(("#Vertices: " + std::to_string(m_isoline_simplifier->m_current_complexity)).c_str()));
		auto temp_simplifier = IsolineSimplifier(m_isoline_simplifier->m_simplified_isolines);
		auto measure_text_string = "Symmetric difference: " + std::to_string(m_isoline_simplifier->symmetric_difference()) +
		    "\n#Ladders: " + std::to_string(temp_simplifier.ladder_count());
	    m_isoline_simplifier->m_angle_filter = angle_filter_input->value();
		measure_text->setText(QString(measure_text_string.c_str()));
		recalculate(debugInfo->checkState(), simplificationTarget->value(),
		            doCGALSimplify->checkState(), regionIndex->value(), showVertices->checkState(), isolineIndex->value());
	};

	m_reload = [this, dir, angle_filter_input, collapse_selector, fileSelector]() {
		std::shared_ptr<ipe::Document> document = IpeReader::loadIpeFile(dir + fileSelector->currentText().toStdString() + ".ipe");
		ipe::Page* page = document->page(0);
		LadderCollapse collapse;
		switch (collapse_selector->currentIndex()) {
		case 0: {
			collapse = midpoint_collapse;
			break;
		}
		case 1: {
			collapse = min_sym_diff_collapse;
			break;
		}
		case 2: {
			collapse = spline_collapse(projected_midpoint);
			break;
		}
		case 3: {
			collapse = spline_collapse(min_sym_diff_point);
			break;
		}
		default: {
			std::cerr << "Not yet implemented: " << collapse_selector->currentText().toStdString() << " " << collapse_selector->currentIndex() << std::endl;
			break;
		}
		}
		m_isoline_simplifier = std::make_unique<IsolineSimplifier>(isolinesInPage(page), angle_filter_input->value(), collapse);
		m_debug_ladder = std::nullopt;
		m_recalculate();
	};
	connect(fileSelector, &QComboBox::currentTextChanged, m_reload);
	connect(debugInfo, &QCheckBox::stateChanged, m_recalculate);
	connect(simplificationTarget, QOverload<int>::of(&QSpinBox::valueChanged), m_recalculate);
	connect(doCGALSimplify, &QCheckBox::stateChanged, m_reload);
	connect(regionIndex, QOverload<int>::of(&QSpinBox::valueChanged), m_recalculate);
	connect(showGrid, &QCheckBox::stateChanged, [this](bool v) { m_renderer->setDrawAxes(v); });
	connect(showVertices, &QCheckBox::stateChanged, m_recalculate);
	connect(isolineIndex, QOverload<int>::of(&QSpinBox::valueChanged), m_recalculate);
	connect(angle_filter_input, QOverload<double>::of(&QDoubleSpinBox::valueChanged), m_recalculate);
	connect(collapse_selector, &QComboBox::currentTextChanged, m_reload);
	connect(step_button, &QPushButton::clicked, [this]() {
	    m_debug_ladder = std::nullopt;
		bool progress = m_isoline_simplifier->step();
		if (progress) {
			m_isoline_simplifier->update_matching();
			m_isoline_simplifier->update_ladders();
		}
	  	m_recalculate();
	});
	connect(step_only_button, &QPushButton::clicked, [this]() {
	    m_debug_ladder = std::nullopt;
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
	connect(simplify_button, &QPushButton::clicked, [this, simplificationTarget, doCGALSimplify]() {
	  	m_debug_ladder = std::nullopt;
		int target = simplificationTarget->value();
		if (doCGALSimplify->isChecked()) {
			m_isoline_simplifier->dyken_simplify(target);
		} else {
			m_isoline_simplifier->simplify(target);
		}

		m_recalculate();
	});
	connect(reload_button, &QPushButton::clicked, m_reload);
	connect(save_button, &QPushButton::clicked, m_save);
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
			auto topology_text =  !ladder->m_valid ? "" : "\nChanges topology: " + std::to_string(m_isoline_simplifier->check_ladder_collapse_topology(*ladder));
			auto cost_text = "\nCost: " + std::to_string(ladder->m_cost);
			auto old_text = "\nOld: " + std::to_string(ladder->m_old);

			debug_text->setText(QString((valid_text + intersected_text + topology_text + cost_text + old_text).c_str()));
		}

		m_recalculate();
	});

	m_recalculate();
}

void IsolineSimplificationDemo::recalculate(bool debugInfo, int target, bool cgal_simplify, int region_index,
                                            bool show_vertices, int isoline_index) {
	// todo: split into repaint and separate recalculations like simplification, debugInfo
	m_renderer->clear();
	IpeRenderer ipe_renderer;

	std::vector<Isoline<K>>& original_isolines = m_isoline_simplifier->m_simplified_isolines;
	std::vector<Isoline<K>>& simplified_isolines = m_isoline_simplifier->m_simplified_isolines;

	auto medial_axis_p = std::make_shared<VoronoiPainting>(m_isoline_simplifier->m_delaunay);
	if (debugInfo) {
		m_renderer->addPainting(medial_axis_p, "Voronoi diagram");
//		ipe_renderer.addPainting(medial_axis_p, "Voronoi diagram");
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
//			ipe_renderer.addPainting(separator_p, "Separator");
		}
		m_renderer->addPainting(slope_ladder_p, "Slope ladders");

		ipe_renderer.addPainting(slope_ladder_p, "Slope_ladders");
	}

//	auto vdp = std::make_shared<VoronoiExceptMedialPainting>(*m_isoline_simplifier);
//	auto map = std::make_shared<MedialAxisExceptSeparatorPainting>(*m_isoline_simplifier);
//	m_renderer->addPainting(vdp, "Voronoi");
//	m_renderer->addPainting(map, "Medial axis");
//	m_renderer->addPainting(separator_p, "Separator");

//	ipe_renderer.addPainting(vdp, "Voronoi");
//	ipe_renderer.addPainting(map, "Medial_axis");
//	ipe_renderer.addPainting(separator_p, "Separator");

	auto original_isolines_p = std::make_shared<IsolinePainting>(m_isoline_simplifier->m_isolines, show_vertices, true, false);
	auto isolines_p = std::make_shared<IsolinePainting>(simplified_isolines, show_vertices, false, false);
	if (m_isoline_simplifier->m_started)
		m_renderer->addPainting(original_isolines_p, "Original isolines");
	m_renderer->addPainting(isolines_p, "Simplified isolines");
//	ipe_renderer.addPainting(isolines_p, "Simplified_isolines");

	if (cgal_simplify) {


//		auto cgal_simplified_p = std::make_shared<IsolinePainting>(m_cgal_simplified, show_vertices, false);
//		m_renderer->addPainting(cgal_simplified_p, "CGAL simplified isolines");
//		ipe_renderer.addPainting(cgal_simplified_p, "CGAL simplified isolines");
	}

	if (!m_isoline_simplifier->m_started && debugInfo && region_index < simplified_isolines.size() && separator.contains(&simplified_isolines[region_index])) {
		auto touched_p = std::make_shared<TouchedPainting>(separator.at(&simplified_isolines[region_index]), m_isoline_simplifier->m_delaunay);
		m_renderer->addPainting(touched_p, "Touched");
	}

	if (debugInfo) {
		m_renderer->addPainting(collapse_p, "Ladder collapse");
//		ipe_renderer.addPainting(collapse_p, "LadderCollapse");
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
//	std::shared_ptr<ipe::Document> document = IpeReader::loadIpeFile(dir + "large-western-island.ipe");
//	ipe::Page* page = document->page(0);
//	auto simplifier = IsolineSimplifier(isolinesInPage(page), M_PI/6, min_sym_diff_collapse);
//
//	auto start = std::chrono::system_clock::now().time_since_epoch();
//	simplifier.simplify(1);
//	auto end = std::chrono::system_clock::now().time_since_epoch();
//
//	const std::chrono::duration<double> duration = end - start;
//
//	std::cout << std::endl << duration.count() << std::endl;
//}

//int main() {
//	std::vector<CGAL::Point_2<K>> points1;
//	std::vector<CGAL::Point_2<K>> points2;
//	std::vector<CGAL::Point_2<K>> points3;
//
////	points1.emplace_back(22.3969, 93.1914);
////	points1.emplace_back(23.6548, 95.6021);
////	points1.emplace_back(27.8419, 94.5151);
////	points1.emplace_back(26.7312, 88.9916);
////
////	points2.emplace_back(22.1502, 94.4831);
////	Gt::Point_2 s(23.9491, 97.2822);
////	points2.push_back(s);
////	Gt::Point_2 t(26.2174, 95.7656);
////	points2.push_back(t);
////	Gt::Point_2 u(26.7982, 96.4668);
////	points2.push_back(u);
////	Gt::Point_2 v(29.5491, 101.4);
////	points2.push_back(v);
////
////	points3.emplace_back(20.6136, 95.8987);
////	points3.emplace_back(23.3992, 98.7507);
////	points3.emplace_back(24.8233, 102.603);
//
//	points1.emplace_back(22.3969, 94.1914);
//	points1.emplace_back(23.6548, 96.6021);
//	points1.emplace_back(27.8419, 94.5151);
//	points1.emplace_back(26.7312, 88.9916);
//
//	points2.emplace_back(22.1502, 94.4831);
//	Gt::Point_2 s(23.9491, 97.2822);
//	points2.push_back(s);
//	Gt::Point_2 t(26.2174, 95.7656);
//	points2.push_back(t);
//	Gt::Point_2 u(26.7982, 96.4668);
//	points2.push_back(u);
//	Gt::Point_2 v(29.5491, 101.4);
//	points2.push_back(v);
//
//	points3.emplace_back(21.6136, 94.8987);
//	points3.emplace_back(24.3992, 97.7507);
//	points3.emplace_back(25.8233, 101.603);
//
//	SDG2 delaunay;
//	auto insert_polyline = [&delaunay](const std::vector<Gt::Point_2>& pts) {
//		std::vector<Gt::Segment_2> segments;
//		for (int i = 0; i < pts.size()-1; i++) {
//			segments.emplace_back(pts[i], pts[i+1]);
//		}
//		delaunay.insert_segments(segments.begin(), segments.end());
//	};
//	insert_polyline(points1);
//	insert_polyline(points2);
//	insert_polyline(points3);
//
//	std::unordered_map<Gt::Point_2, SDG2::Vertex_handle> p_vertex;
//	std::unordered_map<Gt::Segment_2, SDG2::Vertex_handle> e_vertex;
//
//	for (auto vit = delaunay.finite_vertices_begin(); vit != delaunay.finite_vertices_end(); vit++) {
//		auto site = vit->site();
//		if (site.is_point()) {
//			p_vertex[site.point()] = vit;
//		} else {
//			e_vertex[site.segment()] = vit;
//		}
//	}
//
//	Gt::Segment_2 st(s, t);
//	Gt::Segment_2 tu(t, u);
//	Gt::Segment_2 uv(u, v);
//
//	auto print_incident_vertices = [&](SDG2::Vertex_handle vertex) {
////	  	auto fit_start = delaunay.incident_faces(p_vertex.at(u));
////		auto fit = fit_start;
////	  	std::cout << "-------------" << std::endl;
////		do {
////			auto vit_start = delaunay.incident_vertices(vertex, fit);
//	 	    auto vit_start = delaunay.incident_vertices(vertex);
//			auto vit = vit_start;
//
//
//			std::cout << "-------------" << std::endl;
//			do {
//				if (!delaunay.is_infinite(vit)) {
//					auto v = *vit;
////				    if (v.is_segment() && v.site().segment() == uv) {
////						std::cout << "---special---" << std::endl;
////					    auto f = v.face();
////					    for (int i = 0; i < 3; i++) {
////						    std::cout << f->vertex(i)->site() << std::endl;
////					    }
////						std::cout << "---special end---" << std::endl;
////				    }
//					std::cout << v.site() << std::endl;
//				}
//				++vit;
//			} while (vit != vit_start);
////
////			++fit;
////		} while (fit != fit_start);
//	};
//
//	print_incident_vertices(p_vertex.at(u));
//
//	std::cout << "Removing: " << tu << std::endl;
//	std::cout << delaunay.remove(e_vertex.at(tu)) << std::endl;
//	std::cout << "Removing: " << st << std::endl;
//	std::cout << delaunay.remove(e_vertex.at(st)) << std::endl;
//
//	std::cout << "normal?" << std::endl;
//	for (int i = 0; i < 3; i++) {
//		std::cout << p_vertex.at(t)->face()->neighbor(2)->vertex(i)->site() << std::endl;
//	}
//	std::cout << "Removing: " << uv << std::endl;
//	std::cout << delaunay.remove(e_vertex.at(uv)) << std::endl;
////	print_incident_vertices(e_vertex.at(uv));
////	print_incident_vertices(p_vertex.at(u));
//
//	std::cout << "Weird" << std::endl;
//	for (int i = 0; i < 3; i++) {
//		std::cout << p_vertex.at(t)->face()->neighbor(2)->vertex(i)->site() << std::endl;
//	}
////	std::cout << "Removing: " << t << std::endl;
////	std::cout << delaunay.remove(p_vertex.at(t)) << std::endl;
//
//
////	print_incident_vertices(p_vertex.at(u));
//
//
////	std::cout << "Removing: " << u << std::endl;
////	std::cout << delaunay.remove(p_vertex.at(u)) << std::endl;
//
//	return 0;
//}

VoronoiPainting::VoronoiPainting(const SDG2& delaunay): m_delaunay(delaunay) {}

void VoronoiPainting::paint(GeometryRenderer& renderer) const {
	renderer.setStroke(Color(150, 150, 150), 1);
	renderer.setMode(GeometryRenderer::stroke);
	auto voronoiDrawer = VoronoiDrawer<Gt>(&renderer);
	draw_dual<VoronoiDrawer<Gt>, K>(m_delaunay, voronoiDrawer);
}

IsolinePainting::IsolinePainting(const std::vector<Isoline<K>>& isolines, bool show_vertices, bool light, bool ipe):
      m_isolines(isolines), m_show_vertices(show_vertices), m_light(light), m_ipe(ipe) {}

void IsolinePainting::paint(GeometryRenderer& renderer) const {
	// Draw isolines
	if (m_show_vertices) {
		renderer.setMode(GeometryRenderer::stroke | GeometryRenderer::vertices);
	} else {
		renderer.setMode(GeometryRenderer::stroke);
	}

	double stroke_weight = m_ipe ? 0.4 : 1;

	if (m_light) {
		renderer.setStroke(Color(150, 150, 150), stroke_weight);
	} else {
		renderer.setStroke(Color(0, 0, 0), stroke_weight);
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
		renderer.setStroke(Color(30, 119, 179), 2.5);
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

void draw_ladder_collapse(GeometryRenderer& renderer, IsolineSimplifier& simplifier, const SlopeLadder& ladder) {
//	for (int i = 0; i < ladder.m_rungs.size(); i++) {
//		const auto& rung = ladder.m_rungs.at(i);
//		const auto& p = ladder.m_collapsed.at(i);
//		auto reversed = simplifier.m_p_next.contains(rung.target()) && simplifier.m_p_next.at(rung.target()) == rung.source();
//		auto t = reversed ? rung.target() : rung.source();
//		auto u = reversed ? rung.source() : rung.target();
//		auto s = simplifier.m_p_prev.at(t);
//		auto v = simplifier.m_p_next.at(u);
//		auto l = area_preservation_line(s, t, u, v);
//		renderer.setStroke(Color(60, 60, 60), 2.0);
//		renderer.draw(l);
//
//		renderer.setStroke(Color(255, 165, 0), 4.0);
//		renderer.draw(Gt::Segment_2(s, p));
//		renderer.draw(Gt::Segment_2(p, v));
//		renderer.draw(p);
//
//	}

	std::vector<ipe::Vector> control_points;
	if (ladder.m_cap.contains(CGAL::LEFT_TURN)) {
		control_points.push_back(pv(ladder.m_cap.at(CGAL::LEFT_TURN)));
	}
	for (const auto& rung : ladder.m_rungs) {
		auto& p_next = simplifier.m_p_next;
		auto& p_prev = simplifier.m_p_prev;
		auto reversed = p_next.contains(rung.target()) && p_next.at(rung.target()) == rung.source();
		auto t = reversed ? rung.target() : rung.source();
		auto u = reversed ? rung.source() : rung.target();
		Gt::Point_2 s = p_prev.at(t);
		Gt::Point_2 v = p_next.at(u);
//		control_points.push_back(pv(min_sym_diff_point(s, t, u, v)));
		control_points.push_back(pv(projected_midpoint(s, t, u, v)));
	}
	if (ladder.m_cap.contains(CGAL::RIGHT_TURN)) {
		control_points.push_back(pv(ladder.m_cap.at(CGAL::RIGHT_TURN)));
	}

	if (control_points.size() > 1) {
		ipe::Curve curve;
		curve.appendSpline(control_points);
		if (curve.countSegments() > 1) {
			throw std::runtime_error("Expected only one segment in spline.");
		}
		std::vector<ipe::Bezier> bzs;
		auto curved_segment = curve.segment(0);
		curved_segment.beziers(bzs);

		BezierSpline spline;
		for (const auto& bz : bzs) {
			spline.appendCurve(vp(bz.iV[0]), vp(bz.iV[1]), vp(bz.iV[2]), vp(bz.iV[3]));
		}
		renderer.setMode(GeometryRenderer::stroke);
		renderer.setStroke(Color(20, 20, 255), 3.0);
		renderer.draw(spline);
	}

	for (auto& cp : control_points) {
		renderer.setStroke(Color(100, 0, 0), 3.0);
		renderer.draw(vp(cp));
	}

	for (int i = 0; i < ladder.m_rungs.size(); i++) {
		const auto& rung = ladder.m_rungs.at(i);
		const auto& p = ladder.m_collapsed.at(i);

		auto reversed = simplifier.m_p_next.contains(rung.target()) &&
						simplifier.m_p_next.at(rung.target()) == rung.source();
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

//		auto vhs = simplifier.intersected_region(rung, p);
//		auto [boundaries, outer] = simplifier.boundaries(vhs);
//
//		auto voronoi_drawer = VoronoiDrawer<Gt>(&renderer);
//
//		for (const auto& vh : vhs) {
//			auto eit_start = simplifier.m_delaunay.incident_edges(vh);
//			auto eit = eit_start;
//			do {
//				renderer.setStroke(Color(0, 0, 0), 2.0);
//				draw_dual_edge<VoronoiDrawer<Gt>, K>(simplifier.m_delaunay, *eit, voronoi_drawer);
//				++eit;
//			} while (eit != eit_start);
//		}
//
//		for (const auto& e : boundaries[outer]) {
//			renderer.setStroke(Color(200, 0, 0), 4.0);
//			draw_dual_edge<VoronoiDrawer<Gt>, K>(simplifier.m_delaunay, e, voronoi_drawer);
//		}
//		for (int j = 0; j < boundaries.size(); ++j) {
//			if (j == outer)
//				continue;
//			for (const auto& e : boundaries[j]) {
//				renderer.setStroke(Color(0, 200, 0), 4.0);
//				draw_dual_edge<VoronoiDrawer<Gt>, K>(simplifier.m_delaunay, e, voronoi_drawer);
//			}
//		}
	}
}

void SlopeLadderPainting::paint(GeometryRenderer& renderer) const {
	std::unordered_set<Gt::Segment_2> edges;
	for (const auto& slope_ladder : m_slope_ladders) {
		if (slope_ladder->m_old) continue;
		auto poly = slope_ladder_polygon(*slope_ladder);
		for (auto eit = poly.edges_begin(); eit != poly.edges_end(); eit++) {
			Gt::Segment_2 e = *eit;
			if (!edges.contains(e) && !edges.contains(e.opposite())) {
				edges.insert(e);
			}
		}
		renderer.setMode(GeometryRenderer::fill);
		renderer.setFill(Color(252, 190, 110));
		renderer.setFillOpacity(25);
		renderer.draw(poly);
//		draw_slope_ladder(renderer, *slope_ladder);
	}

	for (const auto& e : edges) {
		renderer.setStroke(Color(255, 126, 0), 1.0);
		renderer.setMode(GeometryRenderer::stroke);
		renderer.draw(e);
	}
}

CollapsePainting::CollapsePainting(IsolineSimplifier& simplifier):
      m_simplifier(simplifier) {}

void CollapsePainting::paint(GeometryRenderer& renderer) const {
	if (!m_simplifier.m_slope_ladders.empty()) {
		auto next = m_simplifier.get_next_ladder();
		if (!next.has_value()) return;
		const auto& slope_ladder = *next;
		if (!slope_ladder->m_old) {
			renderer.setStroke(Color(255, 20, 20), 2.0);
			draw_slope_ladder(renderer, *slope_ladder);
			draw_ladder_collapse(renderer, m_simplifier, *slope_ladder);
		}
	}

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

MedialAxisExceptSeparatorPainting::MedialAxisExceptSeparatorPainting(IsolineSimplifier& simplifier):
      m_simplifier(simplifier) {}

void MedialAxisExceptSeparatorPainting::paint(GeometryRenderer& renderer) const {
	auto& del = m_simplifier.m_delaunay;
	for (auto eit = del.finite_edges_begin(); eit != del.finite_edges_end(); eit++) {
		auto vd = VoronoiDrawer<Gt>(&renderer);
		auto edge = *eit;
		auto [p, q] = defining_sites(edge);
		SDG2::Point_2 p_point = point_of_site(p);
		SDG2::Point_2 q_point = point_of_site(q);

		bool is_endpoint_of_seg =
		    ( p.is_segment() && q.is_point() &&
		     is_endpoint_of_segment<K>(del, q, p) ) ||
		    ( p.is_point() && q.is_segment() &&
		     is_endpoint_of_segment<K>(del, p, q) );

		if (!is_endpoint_of_seg && m_simplifier.m_p_isoline.at(p_point) == m_simplifier.m_p_isoline.at(q_point)) {
			renderer.setStroke(Color(210, 210, 210), 1.0);
			draw_dual_edge<VoronoiDrawer<Gt>, K>(del, *eit, vd);
		}
	}
}

VoronoiExceptMedialPainting::VoronoiExceptMedialPainting(IsolineSimplifier& simplifier):
      m_simplifier(simplifier) {

}

void VoronoiExceptMedialPainting::paint(GeometryRenderer& renderer) const {
	auto& del = m_simplifier.m_delaunay;
	for (auto eit = del.finite_edges_begin(); eit != del.finite_edges_end(); eit++) {
		auto vd = VoronoiDrawer<Gt>(&renderer);
		auto edge = *eit;
		auto [p, q] = defining_sites(edge);

		bool is_endpoint_of_seg =
		    (p.is_segment() && q.is_point() && is_endpoint_of_segment<K>(del, q, p)) ||
		    (p.is_point() && q.is_segment() && is_endpoint_of_segment<K>(del, p, q));

		if (is_endpoint_of_seg) {
			renderer.setStroke(Color(210, 210, 210), 1);
			draw_dual_edge<VoronoiDrawer<Gt>, K>(del, *eit, vd);
		}
	}
}