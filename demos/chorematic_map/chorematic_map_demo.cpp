#include "chorematic_map_demo.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDockWidget>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QSlider>
#include <QVBoxLayout>

#include <utility>

#include "parse_input.h"

#include "cartocrow/core/arrangement_helpers.h"
#include "cartocrow/chorematic_map/maximum_weight_disk.h"

#include <CGAL/centroid.h>

class RegionArrangementPainting : public GeometryPainting {
  private:
	std::shared_ptr<RegionArrangement> m_arr;
	std::shared_ptr<std::unordered_map<std::string, double>> m_data;
	QSlider* m_threshold;
  public:
	RegionArrangementPainting(std::shared_ptr<RegionArrangement> arr,
	                          std::shared_ptr<std::unordered_map<std::string, double>> data,
	                          QSlider* threshold) :
	      m_arr(std::move(arr)), m_data(std::move(data)), m_threshold(threshold) {};

	void paint(GeometryRenderer &renderer) const override {
		for (auto fit = m_arr->faces_begin(); fit != m_arr->faces_end(); ++fit) {
			if (!fit->has_outer_ccb()) continue;
			double w;
			auto region = fit->data();
			if (!region.empty()) {
				renderer.setMode(GeometryRenderer::fill);
			} else {
				renderer.setMode(0);
			}
			auto t = m_threshold->value();
			if (m_data->contains(region)) {
				w = m_data->at(fit->data());
			} else {
				w = t;
			}
			if (w > t) {
				renderer.setFill(Color{255, 100, 100});
			} else if (w < t) {
				renderer.setFill(Color{100, 100, 255});
			} else {
				renderer.setFill(Color{200, 200, 200});
			}
			auto poly = face_to_polygon_with_holes<Exact>(fit);
			renderer.draw(poly);
			auto c = CGAL::centroid(poly.outer_boundary().vertices_begin(), poly.outer_boundary().vertices_end());
			renderer.setMode(GeometryRenderer::stroke);
			renderer.drawText(c, fit->data());
		}
		renderer.setMode(GeometryRenderer::stroke);
		renderer.setStroke(Color{0, 0, 0}, 1.0);
		for (auto eit = m_arr->edges_begin(); eit != m_arr->edges_end(); ++eit) {
			renderer.draw(Segment<Exact>(eit->source()->point(), eit->target()->point()));
		}
	}
};

std::unordered_map<std::string, double> parseRegionDataFile(std::filesystem::path& path) {
	std::ifstream inputStream(path, std::ios_base::in);
	if (!inputStream.good()) {
		throw std::runtime_error("Failed to read input");
	}
	std::stringstream buffer;
	buffer << inputStream.rdbuf();
	return parseRegionData(buffer.str());
}

std::tuple<double, double, double, int> regionDataData(const std::unordered_map<std::string, double>& regionData) {
	std::optional<double> minValue;
	std::optional<double> maxValue;
	double totalValue = 0;
	int count = 0;
	for (auto [_, value] : regionData) {
		if (!minValue.has_value() || value < minValue) {
			minValue = value;
		}
		if (!maxValue.has_value() || value > maxValue) {
			maxValue = value;
		}
		totalValue += value;
		++count;
	}
	return {*minValue, *maxValue, totalValue, count};
}

std::string regionDataInfo(const std::unordered_map<std::string, double>& regionData) {
	auto [minValue, maxValue, totalValue, count] = regionDataData(regionData);

	std::stringstream ss;
	ss << "Region data info" << std::endl;
	ss << "Min: " << minValue << std::endl;
	ss << "Max: " << maxValue << std::endl;
	ss << "Average: " << totalValue / count << std::endl;
	return ss.str();
}

std::optional<Circle<Inexact>> circle(InducedDiskW disk) {
	auto [p1, p2, p3] = disk;
	if (p1.has_value() && p2.has_value() && p3.has_value()) {
		return Circle<Inexact>(p1->point, p2->point, p3->point);
	} else if (p1.has_value() && p2.has_value()) {
		return Circle<Inexact>(p1->point, p2->point);
	} else if (p1.has_value()) {
		return Circle<Inexact>(p1->point, 3.0);
	} else {
		return std::nullopt;
	}
}

void ChorematicMapDemo::recompute() {
	m_samples.clear();
	m_sampler->set_seed(m_seed->value());
	m_sampler->uniform_random_samples(m_nSamples->value(), std::back_inserter(m_samples));
	for (auto& pt : m_samples) {
		pt.weight -= m_threshold->value();
	}
	InducedDiskW disk = m_invert->isChecked() ?
		smallest_minimum_weight_disk(m_samples.begin(), m_samples.end()) :
		smallest_maximum_weight_disk(m_samples.begin(), m_samples.end());
	m_disk = circle(disk);
	m_renderer->repaint();
}

ChorematicMapDemo::ChorematicMapDemo() {
	setWindowTitle("Chorematic map");
	m_renderer = new GeometryWidget();
	m_renderer->setDrawAxes(false);
	setCentralWidget(m_renderer);

	auto* dockWidget = new QDockWidget();
	addDockWidget(Qt::RightDockWidgetArea, dockWidget);
	auto* vWidget = new QWidget();
	auto* vLayout = new QVBoxLayout(vWidget);
	vLayout->setAlignment(Qt::AlignTop);
	dockWidget->setWidget(vWidget);

	auto* seedLabel = new QLabel("Seed");
	m_seed = new QSpinBox();
	m_seed->setValue(0);
	m_seed->setMaximum(100);
	vLayout->addWidget(seedLabel);
	vLayout->addWidget(m_seed);

	auto* nSamplesLabel = new QLabel("#Samples");
	m_nSamples = new QSpinBox();
	m_nSamples->setMinimum(1);
	m_nSamples->setMaximum(10000);
	m_nSamples->setValue(100);
	vLayout->addWidget(nSamplesLabel);
	vLayout->addWidget(m_nSamples);

	m_invert = new QCheckBox();
	m_invert->setChecked(true);
	vLayout->addWidget(m_invert);

//	std::ifstream inputStream("data/chorematic_map/test_data.txt", std::ios_base::in);

	std::filesystem::path dataPath = "data/chorematic_map/europe_data.txt";
	std::filesystem::path mapPath = "data/europe.ipe";
	m_regionData = std::make_shared<std::unordered_map<std::string, double>>(parseRegionDataFile(dataPath));
//	auto regionMap = ipeToRegionMap("data/chorematic_map/test_region_arrangement.ipe");
	auto regionMap = ipeToRegionMap(mapPath);
//	auto regionMap = ipeToRegionMap("data/chorematic_map/gem_2017_simplified.ipe");
	m_regionArr = std::make_shared<RegionArrangement>(regionMapToArrangement(regionMap));



	auto* infoText = new QLabel();
	infoText->setText(QString::fromStdString(regionDataInfo(*m_regionData)));
	vLayout->addWidget(infoText);

	auto* thresholdLabel = new QLabel("Threshold");
	m_threshold = new QSlider();
	m_threshold->setOrientation(Qt::Horizontal);

	auto [rdMin, rdMax, _, blah] = regionDataData(*m_regionData);
	m_threshold->setMinimum(rdMin);
	m_threshold->setMaximum(rdMax);
	m_threshold->setValue(0);

	vLayout->addWidget(thresholdLabel);
	vLayout->addWidget(m_threshold);

	m_pl = std::make_shared<Landmarks_pl>(*m_regionArr);
	m_sampler = std::make_unique<Sampler<Landmarks_pl>>(*m_regionArr, *m_pl, *m_regionData, m_seed->value());

	auto rap = std::make_shared<RegionArrangementPainting>(m_regionArr, m_regionData, m_threshold);
	m_renderer->addPainting(rap, "Region arrangement");

	m_renderer->addPainting([this](renderer::GeometryRenderer& renderer) {
	  	double minWeight = 0;
		double maxWeight = 0;
	  	for (const auto& [point, weight] : m_samples) {
			if (weight > maxWeight) {
				maxWeight = weight;
			}
			if (weight < minWeight) {
				minWeight = weight;
			}
	  	}

	  renderer.setMode(GeometryRenderer::stroke);
		for (const auto& [point, weight] : m_samples) {
			if (weight > 0) {
				Color c(1 + weight * 155 / maxWeight, 0, 0);
				renderer.setStroke(c, 1.0);
				renderer.draw(point);
			}
			if (weight < 0) {
				Color c(0, 0, 1 + weight * 155 / minWeight);
				renderer.setStroke(c, 1.0);
				renderer.draw(point);
			}
		}
	}, "Samples");

	recompute();

	m_renderer->addPainting([this](renderer::GeometryRenderer& renderer) {
		if (!m_disk.has_value()) return;
		renderer.setMode(GeometryRenderer::fill | GeometryRenderer::stroke);
		renderer.setStroke(Color(0, 0, 0), 2.0);
		renderer.setFill(0x000000);
		renderer.setFillOpacity(50);
		renderer.draw(*m_disk);
	}, "Circle");

	connect(m_seed, QOverload<int>::of(&QSpinBox::valueChanged), [this](){
		recompute();
	});
	connect(m_nSamples, QOverload<int>::of(&QSpinBox::valueChanged), [this](){
	  recompute();
	});
	connect(m_threshold, &QSlider::valueChanged, [this]() {
		recompute();
	});
	connect(m_invert, &QCheckBox::stateChanged, [this]() {
		recompute();
	});
}

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	ChorematicMapDemo demo;
	demo.show();
	app.exec();
}
