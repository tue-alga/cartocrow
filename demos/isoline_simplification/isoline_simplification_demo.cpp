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

#include "isoline_simplification_demo.h"
#include "cartocrow/core/ipe_reader.h"
#include "cartocrow/isoline_simplification/isoline.h"
#include "cartocrow/isoline_simplification/ipe_isolines.h"
#include "cartocrow/isoline_simplification/voronoi_helpers.h"
#include "cartocrow/isoline_simplification/simple_smoothing.h"
#include "cartocrow/renderer/ipe_renderer.h"
#include "medial_axis_helpers.h"
#include "voronoi_drawer.h"
#include <CGAL/bounding_box.h>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDockWidget>
#include <QFileDialog>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QMessageBox>
#include <filesystem>
#include <ipepath.h>
#include <ranges>
#include <utility>

namespace fs = std::filesystem;
using namespace cartocrow;
using namespace cartocrow::renderer;
using namespace cartocrow::isoline_simplification;

IsolineSimplificationDemo::IsolineSimplificationDemo() {
	setWindowTitle("Isoline simplification");

	auto* dockWidget = new QDockWidget();
	addDockWidget(Qt::RightDockWidgetArea, dockWidget);
	auto* vWidget = new QWidget();
	auto* vLayout = new QVBoxLayout(vWidget);
	vLayout->setAlignment(Qt::AlignTop);
	dockWidget->setWidget(vWidget);

	auto* basicOptions = new QLabel("<h3>Basic options</h3>");
	vLayout->addWidget(basicOptions);
	auto* directorySelector = new QPushButton("Select input directory");
	vLayout->addWidget(directorySelector);
	auto* fileSelector = new QComboBox();
	auto* fileSelectorLabel = new QLabel("Input file");
	fileSelectorLabel->setBuddy(fileSelector);
	vLayout->addWidget(fileSelectorLabel);
	vLayout->addWidget(fileSelector);

	m_isoline_simplifier = std::make_unique<IsolineSimplifier>();

	    auto* number_of_vertices = new QLabel(QString(("#Vertices: " + std::to_string(m_isoline_simplifier->m_current_complexity)).c_str()));
	vLayout->addWidget(number_of_vertices);

	auto* simplificationTarget = new QSpinBox();
	simplificationTarget->setValue(50);
	simplificationTarget->setMaximum(1000000);
	auto* simplificationTargetLabel = new QLabel("#Target vertices");
	simplificationTargetLabel->setBuddy(simplificationTarget);
	vLayout->addWidget(simplificationTargetLabel);
	vLayout->addWidget(simplificationTarget);

	auto* simplify_button = new QPushButton("Simplify");
	vLayout->addWidget(simplify_button);

	auto* reload_button = new QPushButton("Reload");
	vLayout->addWidget(reload_button);

	auto* spacer = new QSpacerItem(1, 30);
	vLayout->addItem(spacer);

	auto* detailedOptions = new QLabel("<h3>Detailed options</h3>");
	vLayout->addWidget(detailedOptions);
	auto* debugInfo = new QCheckBox("Debug info");
	vLayout->addWidget(debugInfo);

	auto* doDykenSimplify = new QCheckBox("Use Dyken et al. method");
	vLayout->addWidget(doDykenSimplify);

	auto* disableLadders = new QCheckBox("Disable ladders");
	vLayout->addWidget(disableLadders);

	auto* applySmoothing = new QCheckBox("Apply basic smoothing");
	vLayout->addWidget(applySmoothing);

	auto* showGrid = new QCheckBox("Show grid");
	vLayout->addWidget(showGrid);

	auto* showVertices = new QCheckBox("Show isoline vertices");
	vLayout->addWidget(showVertices);

	auto* collapse_label = new QLabel("Ladder collapse method");
	auto* collapse_selector = new QComboBox();
	collapse_label->setBuddy(collapse_selector);
	vLayout->addWidget(collapse_label);
	collapse_selector->addItem("Midpoint");
	collapse_selector->addItem("Minimize symmetric difference");
	collapse_selector->addItem("Spline");
	collapse_selector->addItem("Harmony line");
	collapse_selector->addItem("(Sp)line hybrid");
	collapse_selector->setCurrentIndex(4);
	vLayout->addWidget(collapse_selector);

	auto* sampleCount = new QSpinBox();
	sampleCount->setValue(15);
	sampleCount->setMaximum(500);
	auto* sampleCountLabel = new QLabel("Sample count");
	sampleCountLabel->setBuddy(sampleCount);
	vLayout->addWidget(sampleCountLabel);
	vLayout->addWidget(sampleCount);

	auto* splineRepetitions = new QSpinBox();
	splineRepetitions->setValue(3);
	splineRepetitions->setMaximum(10);
	auto* splineRepetitionsLabel = new QLabel("Spline repetitions");
	splineRepetitionsLabel->setBuddy(splineRepetitions);
	vLayout->addWidget(splineRepetitionsLabel);
	vLayout->addWidget(splineRepetitions);

	auto* dyken_r = new QDoubleSpinBox();
	dyken_r->setValue(1);
	dyken_r->setMinimum(0);
	dyken_r->setMaximum(10);
	auto* dyken_r_label = new QLabel("Dyken's parameter R");
	dyken_r_label->setBuddy(dyken_r);
	vLayout->addWidget(dyken_r_label);
	vLayout->addWidget(dyken_r);

	auto* step_button = new QPushButton("Step");
	vLayout->addWidget(step_button);

	auto* outputDirectorySelector = new QPushButton("Select output directory");
	vLayout->addWidget(outputDirectorySelector);

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

	m_save = [this, debugInfo, showVertices, fileSelector, collapse_selector, measure_text, applySmoothing]() {
		if (!m_output_dir.has_value() || m_output_dir == "") {
			QMessageBox msgBox;
			msgBox.setText("Please select an output directory.");
			msgBox.exec();
			return;
		}

		auto& isolines = m_isoline_simplifier->m_simplified_isolines;
	  	IpeRenderer ipe_renderer;
	  	auto isolines_p = std::make_shared<IsolinePainting>(isolines, showVertices->isChecked(), false, true,
		                                                    applySmoothing->isChecked());

	  	if (debugInfo->isChecked()) {
			auto separator_p = std::make_shared<MedialAxisSeparatorPainting>(
			    m_isoline_simplifier->m_separator, m_isoline_simplifier->m_delaunay);
			auto slope_ladder_p =
			    std::make_shared<SlopeLadderPainting>(m_isoline_simplifier->m_slope_ladders);
			auto matching_p =
			    std::make_shared<CompleteMatchingPainting>(m_isoline_simplifier->m_matching);

			auto vdp = std::make_shared<VoronoiExceptMedialPainting>(*m_isoline_simplifier);
			auto map = std::make_shared<MedialAxisExceptSeparatorPainting>(*m_isoline_simplifier);

			ipe_renderer.addPainting(vdp, "Voronoi");
			ipe_renderer.addPainting(map, "Medial_axis");
			if (!m_isoline_simplifier->m_started) {
				ipe_renderer.addPainting(separator_p, "Separator");
			}
			ipe_renderer.addPainting(slope_ladder_p, "Slope_ladders");
			ipe_renderer.addPainting(matching_p, "Matching");
		}
	    ipe_renderer.addPainting(isolines_p, "Simplified_isolines");

		if (m_debug_ladder.has_value()) {
			auto ladder = *m_debug_ladder;
			auto debug_ladder_p = std::make_shared<DebugLadderPainting>(*m_isoline_simplifier, *ladder);
			ipe_renderer.addPainting(debug_ladder_p, "Debug_ladder_painting");
		}

		// Output path
		auto input_name = fileSelector->currentText().toStdString();
		auto complexity = std::to_string(m_isoline_simplifier->m_current_complexity);
		auto collapse_name = collapse_selector->currentText().toStdString();
		std::filesystem::path base_path = *m_output_dir;
		base_path /= input_name + "_" + complexity + "_" + collapse_name;

		// Save ipe
		std::filesystem::path ipe_path = base_path;
		ipe_path += ".ipe";
	   	ipe_renderer.save(ipe_path);

		// Save metadata
		std::ofstream meta_data_file;
	    std::filesystem::path meta_data_path = base_path;
		meta_data_path += "_meta.txt";
		meta_data_file.open(meta_data_path);
		meta_data_file << measure_text->text().toStdString();
		meta_data_file.close();
	};

	m_recalculate = [this, debugInfo, doDykenSimplify, simplificationTarget, showVertices, number_of_vertices, applySmoothing]() {
		number_of_vertices->setText(QString(("#Vertices: " + std::to_string(m_isoline_simplifier->m_current_complexity)).c_str()));
		recalculate(debugInfo->checkState(), showVertices->checkState(), applySmoothing->checkState());
	};

	m_reload = [this, collapse_selector, fileSelector, splineRepetitions, sampleCount,
	            disableLadders]() {
		if (!m_dir.has_value()) return;
	    if (m_dir == "") return;
		if (fileSelector->currentText().toStdString() == "") return;
		std::shared_ptr<LadderCollapse> collapse;
		switch (collapse_selector->currentIndex()) {
		case 0: {
			collapse = std::make_shared<MidpointCollapse>();
			break;
		}
		case 1: {
			collapse = std::make_shared<MinSymDiffCollapse>();
			break;
		}
		case 2: {
			collapse = std::make_shared<SplineCollapse>(splineRepetitions->value(), sampleCount->value());
			break;
		}
		case 3: {
			collapse = std::make_shared<HarmonyLineCollapse>(sampleCount->value());
			break;
		}
		case 4: {
			collapse = std::make_shared<LineSplineHybridCollapse>(SplineCollapse(splineRepetitions->value(), sampleCount->value()), HarmonyLineCollapse(sampleCount->value()));
			break;
		}
		default: {
			std::cerr << "Not yet implemented: " << collapse_selector->currentText().toStdString() << " " << collapse_selector->currentIndex() << std::endl;
			break;
		}
		}
		auto ipe_file = *m_dir + "/" + fileSelector->currentText().toStdString() + ".ipe";
		if (disableLadders->isChecked()) {
			m_isoline_simplifier = std::make_unique<IsolineSimplifier>(ipeToIsolines(ipe_file), collapse, -1.0, -1.0);
		} else {
			m_isoline_simplifier = std::make_unique<IsolineSimplifier>(ipeToIsolines(ipe_file), collapse);
		}
		m_debug_ladder = std::nullopt;
		m_recalculate();
	};

	connect(directorySelector, &QPushButton::clicked, [this, fileSelector]() {
		QString start_dir;
		if (m_dir.has_value() && m_dir != "") {
			start_dir = QString::fromStdString(*m_dir);
		} else {
			start_dir = ".";
		}

		m_dir = QFileDialog::getExistingDirectory(this, tr("Select directory with input isolines"),
		                                                     start_dir,
		                                                     QFileDialog::ShowDirsOnly
															 | QFileDialog::DontResolveSymlinks).toStdString();

		if (m_dir == "") return;

		fileSelector->clear();
		for (auto &p : fs::directory_iterator(*m_dir)) {
			if (p.path().extension() == ".ipe")
				fileSelector->addItem(QString::fromStdString(p.path().stem().string()));
		}
	});
	connect(outputDirectorySelector, &QPushButton::clicked, [this]() {
		QString start_dir;
		if (m_output_dir.has_value() && m_output_dir != "") {
			start_dir = QString::fromStdString(*m_output_dir);
		} else {
			start_dir = ".";
		}

		m_output_dir = QFileDialog::getExistingDirectory(this, tr("Select directory in which to place output files"),
		                                          start_dir,
		                                          QFileDialog::ShowDirsOnly
		                                              | QFileDialog::DontResolveSymlinks).toStdString();
	});
	connect(fileSelector, &QComboBox::currentTextChanged, m_reload);
	connect(debugInfo, &QCheckBox::stateChanged, m_recalculate);
	connect(simplificationTarget, QOverload<int>::of(&QSpinBox::valueChanged), m_recalculate);
	connect(doDykenSimplify, &QCheckBox::stateChanged, m_reload);
	connect(disableLadders, &QCheckBox::stateChanged, m_reload);
	connect(applySmoothing, &QCheckBox::stateChanged, m_recalculate);
	connect(showGrid, &QCheckBox::stateChanged, [this](bool v) { m_renderer->setDrawAxes(v); });
	connect(showVertices, &QCheckBox::stateChanged, m_recalculate);
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
	connect(simplify_button, &QPushButton::clicked, [this, simplificationTarget, doDykenSimplify, measure_text, dyken_r]() {
	  	m_debug_ladder = std::nullopt;
		int target = simplificationTarget->value();
		if (doDykenSimplify->isChecked()) {
			m_isoline_simplifier->dyken_simplify(target, dyken_r->value());
		} else {
			m_isoline_simplifier->simplify(target);
		}

		auto temp_simplifier = IsolineSimplifier(m_isoline_simplifier->m_simplified_isolines);

		auto [avg_align, max_align] = temp_simplifier.average_max_vertex_alignment();
		auto measure_text_string = "Symmetric difference: " + std::to_string(m_isoline_simplifier->total_symmetric_difference()) +
		                           "\n#Ladders: " + std::to_string(temp_simplifier.ladder_count()) +
		                           "\nAvg alignment: " + std::to_string(avg_align) +
		                           "\nMax alignment: " + std::to_string(max_align);

		measure_text->setText(QString(measure_text_string.c_str()));
		m_recalculate();
	});
	connect(reload_button, &QPushButton::clicked, m_reload);
	connect(save_button, &QPushButton::clicked, m_save);
	connect(m_renderer, &GeometryWidget::clicked, [this, debug_text, debugInfo](auto pt) {
		if (!debugInfo->isChecked()) return;
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

	m_reload();
}

void IsolineSimplificationDemo::recalculate(bool debugInfo, bool show_vertices, bool applySmoothing) {
	m_renderer->clear();

	std::vector<Isoline<K>>& simplified_isolines = m_isoline_simplifier->m_simplified_isolines;

	auto medial_axis_p = std::make_shared<VoronoiPainting>(m_isoline_simplifier->m_delaunay);
	if (debugInfo) {
		m_renderer->addPainting(medial_axis_p, "Voronoi diagram");
	}

	const auto& separator = m_isoline_simplifier->m_separator;

	auto separator_p = std::make_shared<MedialAxisSeparatorPainting>(separator, m_isoline_simplifier->m_delaunay);
	auto matching_p = std::make_shared<CompleteMatchingPainting>(m_isoline_simplifier->m_matching);
	auto slope_ladder_p = std::make_shared<SlopeLadderPainting>(m_isoline_simplifier->m_slope_ladders);
	auto collapse_p = std::make_shared<CollapsePainting>(*m_isoline_simplifier);

	if (debugInfo) {
		m_renderer->addPainting(matching_p, "Matching");
		if (!m_isoline_simplifier->m_started) {
			m_renderer->addPainting(separator_p, "Separator");
		}
		m_renderer->addPainting(slope_ladder_p, "Slope ladders");
	}

	auto original_isolines_p = std::make_shared<IsolinePainting>(m_isoline_simplifier->m_isolines, show_vertices, true,
	                                                             false, applySmoothing);
	auto isolines_p = std::make_shared<IsolinePainting>(simplified_isolines, show_vertices, false, false, applySmoothing);
	if (m_isoline_simplifier->m_started)
		m_renderer->addPainting(original_isolines_p, "Original isolines");
	m_renderer->addPainting(isolines_p, "Simplified isolines");

	if (debugInfo) {
		m_renderer->addPainting(collapse_p, "Ladder collapse");
	}

	if (m_debug_ladder.has_value()) {
		auto ladder = *m_debug_ladder;
		auto debug_ladder_p = std::make_shared<DebugLadderPainting>(*m_isoline_simplifier, *ladder);
		m_renderer->addPainting(debug_ladder_p, "Debug ladder painting");
	}

	m_renderer->update();
}

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	IsolineSimplificationDemo demo;
	demo.show();
	app.exec();
}

VoronoiPainting::VoronoiPainting(const SDG2& delaunay): m_delaunay(delaunay) {}

void VoronoiPainting::paint(GeometryRenderer& renderer) const {
	renderer.setStroke(Color{150, 150, 150}, 1);
	renderer.setMode(GeometryRenderer::stroke);
	auto voronoiDrawer = VoronoiDrawer<Gt>(&renderer);
	draw_dual<VoronoiDrawer<Gt>, K>(m_delaunay, voronoiDrawer);
}

IsolinePainting::IsolinePainting(const std::vector<Isoline<K>>& isolines, bool show_vertices, bool light, bool ipe,
                                 bool apply_smoothing):
      m_isolines(isolines), m_show_vertices(show_vertices), m_light(light), m_ipe(ipe),
      m_apply_smoothing(apply_smoothing) {}

void IsolinePainting::paint(GeometryRenderer& renderer) const {
	if (m_show_vertices) {
		renderer.setMode(GeometryRenderer::stroke | GeometryRenderer::vertices);
	} else {
		renderer.setMode(GeometryRenderer::stroke);
	}

	double stroke_weight = m_ipe ? 0.4 : 1;

	if (m_light) {
		renderer.setStroke(Color{150, 150, 150}, stroke_weight);
	} else {
		renderer.setStroke(Color{0, 0, 0}, stroke_weight);
	}
	for (const auto& isoline : m_isolines) {
		if (m_apply_smoothing) {
			renderer.draw(simple_smoothing(isoline));
		} else {
			std::visit([&](auto&& v) { renderer.draw(v); }, isoline.drawing_representation());
		}
	}

}

MedialAxisSeparatorPainting::MedialAxisSeparatorPainting(const Separator& separator, const SDG2& delaunay):
      m_separator(separator), m_delaunay(delaunay) {}

void MedialAxisSeparatorPainting::paint(GeometryRenderer& renderer) const {
	auto voronoiDrawer = VoronoiDrawer<Gt>(&renderer);
	for (const auto& [_, edges] : m_separator)
	for (const auto& edge : edges) {
		renderer.setStroke(Color{30, 119, 179}, 2.5);
		renderer.setMode(GeometryRenderer::stroke);
		draw_dual_edge<VoronoiDrawer<Gt>, K>(m_delaunay, edge, voronoiDrawer);
	}
}

MatchingPainting::MatchingPainting(Matching& matching, std::function<bool(Point<K>)> predicate):
      m_matching(matching), m_predicate(std::move(predicate)) {}

void MatchingPainting::paint(GeometryRenderer& renderer) const {
	renderer.setMode(GeometryRenderer::stroke);
	for (const auto& [p, matched_to] : m_matching) {
		if (!m_predicate(p)) continue;
		renderer.setStroke(Color{50, 200, 200}, 3);
		if (matched_to.contains(CGAL::LEFT_TURN)) {
			for (const auto& [_, pts] : matched_to.at(CGAL::LEFT_TURN))
			for (const auto& q : pts) {
				renderer.draw(Segment<K>(p, q));
			}
		}
		renderer.setStroke(Color{200, 50, 200}, 3);
		if (matched_to.contains(CGAL::RIGHT_TURN)) {
			for (const auto& [_, pts] : matched_to.at(CGAL::RIGHT_TURN))
			for (const auto& q : pts) {
				renderer.draw(Segment<K>(p, q));
			}
		}
	}
}

SlopeLadderPainting::SlopeLadderPainting(const Heap& slope_ladders):
      m_slope_ladders(slope_ladders) {}

Polygon<K> slope_ladder_polygon(const SlopeLadder& slope_ladder) {
	std::vector<Point<K>> pts;
	for (const auto& p : slope_ladder.m_rungs) {
		pts.push_back(p.source());
	}
	if (slope_ladder.m_cap.contains(CGAL::RIGHT_TURN)) {
		pts.push_back(slope_ladder.m_cap.at(CGAL::RIGHT_TURN));
	}
	for (int i = slope_ladder.m_rungs.size() - 1; i >= 0; i--) {
		pts.push_back(slope_ladder.m_rungs[i].target());
	}
	if (slope_ladder.m_cap.contains(CGAL::LEFT_TURN)) {
		pts.push_back(slope_ladder.m_cap.at(CGAL::LEFT_TURN));
	}
	return Polygon<K>(pts.begin(), pts.end());
}

void draw_slope_ladder(GeometryRenderer& renderer, const SlopeLadder& slope_ladder) {
	auto poly = slope_ladder_polygon(slope_ladder);
	renderer.setFill(Color{100, 0, 0});
	renderer.setFillOpacity(20);
	renderer.setStroke(Color{255, 20, 20}, 1.0);
	renderer.setMode(GeometryRenderer::fill | GeometryRenderer::stroke);
	renderer.draw(poly);
}

void draw_ladder_collapse(GeometryRenderer& renderer, IsolineSimplifier& simplifier, const SlopeLadder& ladder) {
	simplifier.m_collapse_ladder->painting(ladder, simplifier.m_p_prev, simplifier.m_p_next)->paint(renderer);

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
		renderer.setStroke(Color{60, 60, 60}, 2.0);
		renderer.draw(l);

		renderer.setStroke(Color{255, 165, 0}, 4.0);
		renderer.draw(Segment<K>(s, p));
		renderer.draw(Segment<K>(p, v));
		renderer.draw(p);
	}
}

void SlopeLadderPainting::paint(GeometryRenderer& renderer) const {
	std::unordered_set<Segment<K>> edges;
	for (const auto& slope_ladder : m_slope_ladders) {
		if (slope_ladder->m_old) continue;
		auto poly = slope_ladder_polygon(*slope_ladder);
		for (auto eit = poly.edges_begin(); eit != poly.edges_end(); eit++) {
			Segment<K> e = *eit;
			if (!edges.contains(e) && !edges.contains(e.opposite())) {
				edges.insert(e);
			}
		}
		renderer.setMode(GeometryRenderer::fill);
		renderer.setFill(Color{252, 190, 110});
		renderer.setFillOpacity(25);
		renderer.draw(poly);
	}
}

CompleteMatchingPainting::CompleteMatchingPainting(Matching& matching): m_matching(matching) {}

void CompleteMatchingPainting::paint(GeometryRenderer& renderer) const {
	std::unordered_set<Segment<K>> edges;

	for (const auto& [p, matched_to] : m_matching) {
		if (matched_to.contains(CGAL::LEFT_TURN)) {
			for (const auto& [_, pts] : matched_to.at(CGAL::LEFT_TURN))
				for (const auto& q : pts) {
					Segment<K> seg(p, q);
					if (!edges.contains(seg) && !edges.contains(seg.opposite())) {
						edges.insert(seg);
					}
				}
		}
		if (matched_to.contains(CGAL::RIGHT_TURN)) {
			for (const auto& [_, pts] : matched_to.at(CGAL::RIGHT_TURN))
				for (const auto& q : pts) {
					Segment<K> seg(p, q);
					if (!edges.contains(seg) && !edges.contains(seg.opposite())) {
						edges.insert(seg);
					}
				}
		}
	}

	for (const auto& e : edges) {
		renderer.setStroke(Color{255, 126, 0}, 1.0);
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
			renderer.setStroke(Color{255, 20, 20}, 2.0);
			draw_slope_ladder(renderer, *slope_ladder);
			draw_ladder_collapse(renderer, m_simplifier, *slope_ladder);
		}
	}

	for (const auto& v : m_simplifier.m_changed_vertices) {
		const auto& site = v->site();
		renderer.setStroke(Color{0, 200, 0}, 5.0);
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
			renderer.setStroke(Color{210, 210, 210}, 1.0);
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
			renderer.setStroke(Color{210, 210, 210}, 1);
			draw_dual_edge<VoronoiDrawer<Gt>, K>(del, *eit, vd);
		}
	}
}