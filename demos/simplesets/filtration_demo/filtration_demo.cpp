#include "filtration_demo.h"
#include "cartocrow/core/core.h"
#include "cartocrow/renderer/geometry_painting.h"
#include "cartocrow/renderer/geometry_widget.h"
#include "cartocrow/renderer/ipe_renderer.h"
#include "cartocrow/simplesets/parse_input.h"
#include "cartocrow/simplesets/partition.h"
#include "cartocrow/simplesets/partition_algorithm.h"
#include "cartocrow/simplesets/partition_painting.h"
#include "cartocrow/simplesets/types.h"
#include "demos/simplesets/colors/colors.h"
#include "demos/widgets/double_slider.h"
#include <CGAL/Bbox_2.h>
#include <QApplication>
#include <QDockWidget>
#include <QFileDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

using namespace cartocrow;
using namespace cartocrow::renderer;
using namespace cartocrow::simplesets;

// These are parameters for exporting a 'continuous' sequence of frames to ipe.
double timeStep = 0.1;
double maxTime = 40.0;

FiltrationDemo::FiltrationDemo() {
	std::vector<Color> cbColors({CB::light_blue, CB::light_red, CB::light_green, CB::light_orange, CB::light_purple});

	// These settings currently need to be set manually.
	m_gs = GeneralSettings{1.5, 2, M_PI, 70.0 / 180 * M_PI};
	m_ds = DrawSettings{cbColors, 0.7};
	m_ps = PartitionSettings{true, true, false, false, 0.5};

	setWindowTitle("Filtration");
	m_renderer = new GeometryWidget();
	m_renderer->setDrawAxes(false);
	setCentralWidget(m_renderer);

	auto* dockWidget = new QDockWidget();
	addDockWidget(Qt::RightDockWidgetArea, dockWidget);
	auto* vWidget = new QWidget();
	auto* vLayout = new QVBoxLayout(vWidget);
	vLayout->setAlignment(Qt::AlignTop);
	dockWidget->setWidget(vWidget);

	auto* basicOptions = new QLabel("<h3>Input</h3>");
	vLayout->addWidget(basicOptions);
	auto* fileSelector = new QPushButton("Select file");
	vLayout->addWidget(fileSelector);

	auto* settingsLabel = new QLabel("<h3>Settings</h3>");
	vLayout->addWidget(settingsLabel);
	auto* coverLabel = new QLabel("Cover");
	vLayout->addWidget(coverLabel);
	auto* coverSlider = new DoubleSlider(Qt::Orientation::Horizontal);
	vLayout->addWidget(coverSlider);
	coverSlider->setMinimum(0);
	coverSlider->setMaximum(8);

	auto* exportLabel = new QLabel("<h3>Export to ipe</h3>");
	vLayout->addWidget(exportLabel);
	auto* discreteExport = new QPushButton("Discrete");
	vLayout->addWidget(discreteExport);
	auto* continuousExport = new QPushButton("Continuous");
	vLayout->addWidget(continuousExport);

	auto diskPainting = [this](double cover) {
		std::vector<Point<Inexact>> pts;
		std::transform(m_points.begin(), m_points.end(), std::back_inserter(pts), [](CatPoint& cp) { return cp.point; });

		Box box = CGAL::bbox_2(pts.begin(), pts.end());
		Box boxX(box.xmin() - maxTime - 1, box.ymin() - maxTime - 1, box.xmax() + maxTime + 1, box.ymax() + maxTime + 1);

		return [boxX, this, cover](GeometryRenderer& renderer) {
			renderer.setMode(GeometryRenderer::stroke);
			renderer.setStroke(Color{255, 255, 255}, 1.0);
			renderer.draw(boxX);

			if (cover > 0) {
				renderer.setMode(GeometryRenderer::fill);
				renderer.setFill(Color{232, 232, 232});
				for (const auto& pt : m_points) {
					Circle<Inexact> c(pt.point, squared(cover));
					renderer.draw(c);
				}
			}
		};
	};

	connect(fileSelector, &QPushButton::clicked, [this, fileSelector, coverSlider]() {
	  	m_renderer->clear();
		QString startDir = "data/";
		std::filesystem::path filePath = QFileDialog::getOpenFileName(this, tr("Select SimpleSets input"), startDir).toStdString();
		if (filePath == "") return;
		loadFile(filePath);
		fileSelector->setText(QString::fromStdString(filePath.filename()));
		emit coverSlider->valueChanged(coverSlider->value());
	});
	connect(discreteExport, &QPushButton::clicked, [this, diskPainting]() {
	  	IpeRenderer ipeRenderer;
		int i = 0;
		for (const auto& pair : m_partitions) {
			auto cover = pair.first;
			auto& patterns = pair.second;
			if (i > 0) {
				ipeRenderer.nextPage();
			}
			auto name = "Disks" + std::to_string(i);
			ipeRenderer.addPainting(diskPainting(cover), "Disks");
			auto pp = std::make_shared<PartitionPainting>(patterns, m_gs, m_ds);
			ipeRenderer.addPainting(pp, "Partition");
			++i;
		}

	  	std::filesystem::path filePath = QFileDialog::getSaveFileName(this, tr("Save file"), ".").toStdString();
	  	ipeRenderer.save(filePath);
	});
	connect(continuousExport, &QPushButton::clicked, [this, diskPainting]() {
	  IpeRenderer ipeRenderer;
		for (int i = 1; i < maxTime / timeStep; ++i) {
			auto cover = i * timeStep;
			Partition* thePartition;
			bool found = false;
			for (auto& [time, partition] : m_partitions) {
				if (time < cover) {
					thePartition = &partition;
					found = true;
				}
			}
			auto& patterns = *(found ? thePartition : &m_partitions.front().second);
			if (i > 1) {
				ipeRenderer.nextPage();
			}
			if (i > 0) {
				auto name = "Disks" + std::to_string(i);
				ipeRenderer.addPainting(diskPainting(cover), "Disks");

				auto pp = std::make_shared<PartitionPainting>(patterns, m_gs, m_ds);
				ipeRenderer.addPainting(pp, "Partition");
			}
			++i;
		}

	  	std::filesystem::path filePath = QFileDialog::getSaveFileName(this, tr("Save file"), ".").toStdString();
	  	ipeRenderer.save(filePath);
	});
	connect(coverSlider, &DoubleSlider::valueChanged, [this, coverSlider, diskPainting] {
		m_cover = coverSlider->value() * CGAL::to_double(m_gs.dilationRadius());

		Partition* thePartition;
		bool found = false;
		for (auto& [time, partition] : m_partitions) {
			if (time < m_cover) {
				thePartition = &partition;
				found = true;
			}
		}
		auto& patterns = *(found ? thePartition : &m_partitions.front().second);

		m_renderer->clear();
		m_renderer->addPainting(diskPainting(m_cover), "Disks");
		auto pp = std::make_shared<PartitionPainting>(patterns, m_gs, m_ds);
		m_renderer->addPainting(pp, "Partition");
	});

	loadFile("data/mills.txt");
	coverSlider->setValue(4.6);
}

void FiltrationDemo::loadFile(const std::filesystem::path& filePath) {
	std::ifstream inputStream(filePath, std::ios_base::in);
	if (!inputStream.good()) {
		throw std::runtime_error("Failed to read input");
	}
	std::stringstream buffer;
	buffer << inputStream.rdbuf();
	m_points = parseCatPoints(buffer.str());

	std::vector<Point<Inexact>> pts;
	std::transform(m_points.begin(), m_points.end(), std::back_inserter(pts), [](CatPoint& cp) { return cp.point; });

	Box box = CGAL::bbox_2(pts.begin(), pts.end());
	Box boxX(box.xmin() - maxTime - 1, box.ymin() - maxTime - 1, box.xmax() + maxTime + 1, box.ymax() + maxTime + 1);

	for (auto& p : m_points) {
		p.point -= {boxX.xmin(), boxX.ymin()};
	}

	m_partitions = partition(m_points, m_gs, m_ps, 8 * CGAL::to_double(m_gs.dilationRadius()));
}

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	FiltrationDemo demo;
	demo.show();
	app.exec();
}
