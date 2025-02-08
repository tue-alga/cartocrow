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
#include <QProgressDialog>
#include <QColorDialog>

#include <utility>

#include "cartocrow/chorematic_map/input_parsing.h"
#include "cartocrow/chorematic_map/maximum_weight_disk.h"
#include "cartocrow/circle_segment_helpers/cs_polygon_helpers.h"
#include "cartocrow/core/arrangement_helpers.h"
#include "cartocrow/core/arrangement_map.h"
#include "cartocrow/core/centroid.h"
#include "cartocrow/core/rectangle_helpers.h"

#include "cartocrow/renderer/ipe_renderer.h"

#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/Boolean_set_operations_2/oriented_side.h>
#include <CGAL/General_polygon_set_2.h>

class RegionArrangementPainting : public GeometryPainting {
  private:
	std::shared_ptr<RegionArrangement> m_arr;

  public:
	RegionArrangementPainting(std::shared_ptr<RegionArrangement> arr) : m_arr(std::move(arr)) {};

	void paint(GeometryRenderer &renderer) const override {
        for (auto fit = m_arr->faces_begin(); fit != m_arr->faces_end(); ++fit) {
            if (!fit->has_outer_ccb()) continue;
            auto region = fit->data();
            if (region.empty() || region == "#") {
                continue;
            }
            auto poly = approximate(face_to_polygon_with_holes<Exact>(fit));
            renderer.draw(poly);
            auto c = centroid(poly);
            renderer.setMode(GeometryRenderer::stroke);
            renderer.setStroke(Color{0, 0, 0}, 1.0);
            renderer.drawText(c, region);
        }
        renderer.setMode(GeometryRenderer::stroke);
        renderer.setStroke(Color{0, 0, 0}, 1.0);
		for (auto eit = m_arr->edges_begin(); eit != m_arr->edges_end(); ++eit) {
			Segment<Exact> seg = eit->curve();
			renderer.draw(seg);
		}
	}
};


class RegionMapPainting : public GeometryPainting {
private:
    std::shared_ptr<RegionMap> m_map;

public:
    RegionMapPainting(std::shared_ptr<RegionMap> map) : m_map(std::move(map)) {};

    void paint(GeometryRenderer &renderer) const override {
        for (auto& [name, region] : *m_map) {
            renderer.setMode(GeometryRenderer::stroke | GeometryRenderer::fill);
            renderer.setFill(Color{200, 200, 200});
            renderer.setStroke(Color{0, 0, 0}, 1.0);

            renderer.draw(region.shape);

            renderer.setMode(GeometryRenderer::stroke);
            renderer.setStroke(Color{0, 0, 0}, 1.0);
            auto c = centroid(region.shape);
            renderer.drawText(c, region.name);
        }
    }
};

//class VoronoiRegionArrangementPainting : public GeometryPainting {
//  private:
//	std::shared_ptr<VoronoiRegionArrangement> m_arr;
//
//  public:
//	VoronoiRegionArrangementPainting(std::shared_ptr<VoronoiRegionArrangement> arr) : m_arr(std::move(arr)) {};
//
//	void paint(GeometryRenderer &renderer) const override {
//		for (auto eit = m_arr->edges_begin(); eit != m_arr->edges_end(); ++eit) {
//			Segment<Exact> seg = eit->curve();
//			renderer.draw(seg);
//		}
//		for (auto fit = m_arr->faces_begin(); fit != m_arr->faces_end(); ++fit) {
//			if (fit->is_unbounded()) continue;
//			if (fit->data().site.has_value()) {
//				renderer.draw(*(fit->data().site));
//			}
//		}
//	}
//};

void VoronoiRegionArrangementPainting::paint(GeometryRenderer& renderer) const {
	for (auto eit = m_arr->edges_begin(); eit != m_arr->edges_end(); ++eit) {
		Segment<Exact> seg = eit->curve();
		renderer.draw(seg);
	}
	for (auto fit = m_arr->faces_begin(); fit != m_arr->faces_end(); ++fit) {
		if (fit->is_unbounded()) continue;
		if (fit->data().site.has_value()) {
			renderer.draw(*(fit->data().site));
		}
	}
}

std::unordered_map<std::string, double> parseRegionDataFile(const std::filesystem::path& path) {
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
        if (value == -99999999) continue;
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
	ss << "<h3>Region data info</h3>" << std::endl;
	ss << "Min: " << minValue << "<br>" << std::endl;
	ss << "Max: " << maxValue << "<br>" << std::endl;
	ss << "Average: " << totalValue / count << std::endl;
	return ss.str();
}

void ChorematicMapDemo::resample() {
	m_disks.clear();
    m_sampler->setSeed(m_seed->value());

	switch (m_samplingStrategy->currentIndex()) {
	case 0: {
        m_sample = m_sampler->uniformRandomSamples(m_nSamples->value());
		break;
	}
	case 1: {
		QProgressDialog progress("Voronoi iterations...", "Abort", 0, m_voronoiIters->value(), this);
		progress.setWindowModality(Qt::WindowModal);
		progress.setMinimumDuration(1000);
		m_sample = m_sampler->voronoiUniform(m_nSamples->value(), m_voronoiIters->value(),
		                          		   [&progress](int i) { progress.setValue(i); },
		                          		   [&progress]() { return progress.wasCanceled(); });
		break;
	}
	case 2: {
//		m_sample = m_sampler->squareGrid(m_gridSize->value() / 10.0);
		m_sample = m_sampler->squareGrid(m_nSamples->value());
		break;
	}
	case 3: {
//		m_sample = m_sampler->hexGrid(m_gridSize->value() / 10.0);
		m_sample = m_sampler->hexGrid(m_nSamples->value());
		break;
	}
	default: {
		std::cerr << "Unimplemented sampling strategy: " << m_samplingStrategy->currentIndex()
		          << " " << m_samplingStrategy->currentText().toStdString() << std::endl;
		return;
	}
	}
	if (m_sample.m_points.size() != m_nSamples->value()) {
		std::cout << m_sample.m_points.size() << " samples instead of the target " << m_nSamples->value() << std::endl;
	}
}

void ChorematicMapDemo::rebin() {
    m_choropleth->naturalBreaks(m_numberOfBins->value());
	m_choropleth->rebin();
    m_colorBinSelector->setMaximum(m_choropleth->numberOfBins()-1);
}

void ChorematicMapDemo::recolor() {
	std::vector<Color> colors;
	// colors from https://colorbrewer2.org/#type=sequential&scheme=Greens
	switch(m_numberOfBins->value()) {
	case 2: {
		colors = std::vector({Color(160, 217, 155), Color(48, 162, 83)});
		break;
	}
	case 3: {
		// https://colorbrewer2.org/#type=sequential&scheme=Greens&n=3
		colors = std::vector({Color(0xe5f5e0), Color(0xa1d99b), Color(0x31a354)});
		break;
	}
	case 4: {
		// https://colorbrewer2.org/#type=sequential&scheme=Greens&n=4
		colors = std::vector({Color(0xedf8e9), Color(0xbae4b3), Color(0x74c476), Color(0x238b45)});
		break;
	}
	case 5: {
		// https://colorbrewer2.org/#type=sequential&scheme=Greens&n=5
		colors = std::vector({Color(0xedf8e9), Color(0xbae4b3), Color(0x74c476), Color(0x31a354), Color(0x006d2c)});
		break;
	}
	}
	m_choroplethP->setColors(colors.begin(), colors.end());
}

void ChorematicMapDemo::refit() {
	m_disks = fitDisks(*m_choropleth, m_sample, m_invertFittingOrder->isChecked(), m_numberOfBins->value() == 2,
                       m_applyHeuristic->isChecked(), m_useSymDiff->isChecked());
	if (m_disks[0].score.has_value()) {
		m_diskScoreLabel->setText(QString::fromStdString(std::to_string(m_disks[0].score.value())));
	}
}

void ChorematicMapDemo::loadMap(const std::filesystem::path& mapPath) {
	m_sample.m_points.clear();
	m_diskScoreLabel->setText("");
	m_disks.clear();
	auto regionMap = ipeToRegionMap(mapPath);
	auto newArr = std::make_shared<RegionArrangement>(regionMapToArrangement(regionMap));
	m_sampler->setRegionArr(newArr);
	m_choropleth->m_arr = newArr;
}

void ChorematicMapDemo::loadData(const std::filesystem::path& dataPath) {
	m_choropleth->m_data = std::make_shared<std::unordered_map<std::string, double>>(parseRegionDataFile(dataPath));
	m_dataInfoLabel->setText(QString::fromStdString(regionDataInfo(*m_choropleth->m_data)));
}

ChorematicMapDemo::ChorematicMapDemo() {
	setWindowTitle("Chorematic choropleth");
	m_renderer = new GeometryWidget();
	m_renderer->setDrawAxes(false);
	setCentralWidget(m_renderer);

	auto* dockWidget = new QDockWidget();
	addDockWidget(Qt::RightDockWidgetArea, dockWidget);
	auto* vWidget = new QWidget();
	auto* vLayout = new QVBoxLayout(vWidget);
	vLayout->setAlignment(Qt::AlignTop);
	dockWidget->setWidget(vWidget);

	auto* inputSettings = new QLabel("<h3>Input</h3>");
	auto* loadMapButton = new QPushButton("Load map");
	auto* loadDataButton = new QPushButton("Load data");
	vLayout->addWidget(inputSettings);
	vLayout->addWidget(loadMapButton);
	vLayout->addWidget(loadDataButton);

    m_dataAttribute = new QComboBox();
    vLayout->addWidget(m_dataAttribute);

	m_dataInfoLabel = new QLabel();
	vLayout->addWidget(m_dataInfoLabel);
    auto* exportDataButton = new QPushButton();
    exportDataButton->setText("Export data");
    vLayout->addWidget(exportDataButton);

	auto* binningSettings = new QLabel("<h3>Binning</h3>");
	vLayout->addWidget(binningSettings);

	auto* thresholdLabel = new QLabel("Threshold");
	m_threshold = new QSlider();
	m_threshold->setOrientation(Qt::Horizontal);

	vLayout->addWidget(thresholdLabel);
	vLayout->addWidget(m_threshold);

    auto* choroplethSettings = new QLabel("<h3>Choropleth</h3>");
    vLayout->addWidget(choroplethSettings);
    auto* showLabels = new QCheckBox("Show labels");
    vLayout->addWidget(showLabels);
    showLabels->setChecked(false);

    auto* numberOfBinsLabel = new QLabel("Number of bins");
    m_numberOfBins = new QSpinBox();
    m_numberOfBins->setMinimum(2);
    m_numberOfBins->setValue(2);
    m_numberOfBins->setMaximum(7);
    vLayout->addWidget(numberOfBinsLabel);
    vLayout->addWidget(m_numberOfBins);

    auto* colorLabel = new QLabel("Color of bin");
    m_colorBinSelector = new QSpinBox();
    auto* colorPickerButton = new QPushButton("Choose color");
    vLayout->addWidget(colorLabel);
    vLayout->addWidget(m_colorBinSelector);
    vLayout->addWidget(colorPickerButton);

	auto* samplingSettings = new QLabel("<h3>Sampling</h3>");
	vLayout->addWidget(samplingSettings);

	auto* sampleStrategyLabel = new QLabel("Sample strategy");
	m_samplingStrategy = new QComboBox();
	sampleStrategyLabel->setBuddy(m_samplingStrategy);
	vLayout->addWidget(sampleStrategyLabel);
	m_samplingStrategy->addItem("Uniform random");
	m_samplingStrategy->addItem("Voronoi");
	m_samplingStrategy->addItem("Square grid");
	m_samplingStrategy->addItem("Hexagonal grid");
	m_samplingStrategy->setCurrentIndex(0);
	vLayout->addWidget(m_samplingStrategy);

    auto* samplePerRegion = new QCheckBox("Sample per region");
    vLayout->addWidget(samplePerRegion);

	auto* nSamplesLabel = new QLabel("#Samples");
	m_nSamples = new QSpinBox();
	m_nSamples->setMinimum(1);
	m_nSamples->setMaximum(10000);
	m_nSamples->setValue(100);
	vLayout->addWidget(nSamplesLabel);
	vLayout->addWidget(m_nSamples);

	auto* seedLabel = new QLabel("Seed");
	m_seed = new QSpinBox();
	m_seed->setValue(0);
	m_seed->setMaximum(100);
	vLayout->addWidget(seedLabel);
	vLayout->addWidget(m_seed);

	auto* voronoiItersLabel = new QLabel("Voronoi iterations");
	m_voronoiIters = new QSpinBox();
	m_voronoiIters->setMinimum(1);
	m_voronoiIters->setMaximum(100);
	m_voronoiIters->setValue(5);
	vLayout->addWidget(voronoiItersLabel);
	vLayout->addWidget(m_voronoiIters);

	auto* gridSizeLabel = new QLabel("Grid size");
	m_gridSize = new QSlider();
	m_gridSize->setMinimum(1);
	m_gridSize->setMaximum(1000);
	m_gridSize->setValue(100);
	m_gridSize->setOrientation(Qt::Horizontal);
	vLayout->addWidget(gridSizeLabel);
	vLayout->addWidget(m_gridSize);

	auto* diskFittingSettings = new QLabel("<h3>Disk fitting</h3>");
	vLayout->addWidget(diskFittingSettings);

	m_invertFittingOrder = new QCheckBox("Invert fitting order");
	vLayout->addWidget(m_invertFittingOrder);

    m_applyHeuristic = new QCheckBox("Apply heuristic");
    vLayout->addWidget(m_applyHeuristic);

    m_useSymDiff = new QCheckBox("Symmetric difference cost");
    vLayout->addWidget(m_useSymDiff);

	auto* scoreLabel = new QLabel("Score:");
	vLayout->addWidget(scoreLabel);
	m_diskScoreLabel = new QLabel();
	vLayout->addWidget(m_diskScoreLabel);

	auto* miscellaneous = new QLabel("<h3>Miscellaneous</h3>");
	vLayout->addWidget(miscellaneous);
	m_recomputeAutomatically = new QCheckBox("Recompute automatically");
	m_recomputeAutomatically->setChecked(true);
	vLayout->addWidget(m_recomputeAutomatically);

	auto* recomputeButton = new QPushButton("Recompute");
	vLayout->addWidget(recomputeButton);
	auto* refitButton = new QPushButton("Refit");
	vLayout->addWidget(refitButton);

	std::filesystem::path gpkg0 = "data/chorematic_map/hessen.gpkg";
    std::filesystem::path gpkg3 = "data/chorematic_map/wijkenbuurten_2022_v3.gpkg";
    std::filesystem::path dutch = "data/chorematic_map/gemeenten-2022_5000vtcs.ipe";

    // Dutch
    auto regionMap = std::make_shared<RegionMap>(ipeToRegionMap(dutch, true));
	std::cout << "#Regions: " << regionMap->size() << std::endl;
    m_regionWeightMap = regionDataMapFromGPKG(gpkg3, "gemeenten", "gemeentecode", [](const std::string& s) {
        return s;
    });

    // Hessen
//    auto regionMap = regionMapFromGPKG(gpkg0, "Hessen", "GEN");
//	std::cout << "#Regions: " << regionMap->size() << std::endl;
//    m_regionWeightMap = regionDataMapFromGPKG(gpkg0, "Hessen", "GEN", [](const std::string& s) {
//        return s;
//    });

    for (auto& kv : *m_regionWeightMap) {
        m_dataAttribute->addItem(QString::fromStdString(kv.first));
    }
    m_dataAttribute->model()->sort(0);
	m_dataAttribute->setCurrentIndex(0);
	auto regionData = std::make_shared<std::unordered_map<std::string, double>>((*m_regionWeightMap)[m_dataAttribute->currentText().toStdString()]);

    auto regionArr = regionMapToArrangementParallel(*regionMap);

    m_renderer->setMinZoom(0.00000001);
    m_renderer->setMaxZoom(1000000000);

    m_choropleth = std::make_unique<Choropleth>(std::make_shared<RegionArrangement>(regionArr), std::move(regionData), 2);
	m_sampler = std::make_unique<Sampler<LandmarksPl>>(m_choropleth->m_arr, m_seed->value());
	std::vector<Color> colors({Color(0xe5f5e0), Color(0xa1d99b), Color(0x31a354)});
	ChoroplethPainting::Options choroplethOptions;
	Color offBlack(68, 68, 68);
	Color offWhite(230, 230, 230);
	choroplethOptions.drawLabels = showLabels->isChecked();
	choroplethOptions.noDataColor = Color(255, 0, 0);
	choroplethOptions.strokeColor = offWhite;
	choroplethOptions.strokeWidth = 0.75;
	m_choroplethP = std::make_shared<ChoroplethPainting>(*m_choropleth, colors.begin(), colors.end(), choroplethOptions);
	m_renderer->addPainting(m_choroplethP, "Choropleth");

    Rectangle<Inexact> abb = approximate(m_sampler->getArrBoundingBox());
    auto minDim = fmin(abb.xmax() - abb.xmin(), abb.ymax() - abb.ymin());
    m_gridSize->setMinimum(minDim / 100);
    m_gridSize->setMaximum(minDim);
    m_gridSize->setValue(minDim);
    Box box(abb.xmin(), abb.ymin(), abb.xmax(), abb.ymax());
    m_renderer->fitInView(box);

	m_renderer->addPainting([this](renderer::GeometryRenderer& renderer) {
		renderer.setMode(GeometryRenderer::stroke);
		   for (const auto& point : m_sample.m_points) {
		    renderer.setStroke(Color{0, 0, 0}, 1.0);
			renderer.draw(point);
		}
	}, "Samples");

	rebin();
	recolor();
	resample();
	refit();

	m_dataInfoLabel->setText(QString::fromStdString(regionDataInfo(*m_choropleth->m_data)));
	auto [rdMin, rdMax, _, blah] = regionDataData(*m_choropleth->m_data);
	m_threshold->setMinimum(rdMin);
	m_threshold->setMaximum(rdMax);
	m_threshold->setValue(0);

	m_renderer->addPainting([this, offWhite](renderer::GeometryRenderer& renderer) {
        RenderPath renderPath;
        auto&& polys = m_sampler->getLandmassPolys();
        for (const auto& poly : polys) {
            renderPath << approximate(poly.outer_boundary());
            for (const auto& h : poly.holes()) {
                renderPath << approximate(h);
            }
        }

        renderer.setClipping(true);
        renderer.setClipPath(renderPath);
		renderer.setMode(GeometryRenderer::fill | GeometryRenderer::stroke);
		renderer.setStroke(offWhite, 2.0);
		for (const auto& binDisk : m_disks) {
			renderer.setFill(m_choroplethP->m_colors[binDisk.bin]);
			renderer.setFillOpacity(200);
			auto c = binDisk.disk;
			if (c.has_value()) {
				auto gc = *c;
				if (gc.is_circle()) {
					renderer.draw(gc.get_circle());
				} else {
					renderer.draw(gc.get_halfplane());
				}
			}
		}
        renderer.setClipping(false);
	}, "Disks");

    m_renderer->addPainting([this, offBlack](GeometryRenderer& renderer) {
        renderer.setMode(GeometryRenderer::stroke);
        renderer.setStroke(offBlack, 2);

        auto&& polys = m_sampler->getLandmassPolys();
        for (const auto& poly : polys) {
            renderer.draw(poly);
        }
    }, "Outline");

	connect(m_seed, QOverload<int>::of(&QSpinBox::valueChanged), [this](){
		if (m_recomputeAutomatically->isChecked()) {
			resample();
			refit();
		}
	  	m_renderer->repaint();
	});
	connect(m_nSamples, QOverload<int>::of(&QSpinBox::valueChanged), [this](){
	  	if (m_recomputeAutomatically->isChecked()) {
			resample();
			refit();
	  	}
	  	m_renderer->repaint();
	});
	connect(m_threshold, &QSlider::valueChanged, [this]() {
	  	rebin();
	  	if (m_recomputeAutomatically->isChecked()) {
			refit();
	  	}
	  	m_renderer->repaint();
	});
	connect(m_gridSize, &QSlider::valueChanged, [this]() {
	  	if (m_samplingStrategy->currentIndex() == 2 || m_samplingStrategy->currentIndex() == 3) {
			resample();
			if (m_recomputeAutomatically->isChecked()) {
				refit();
			}
	  	}
	  	m_renderer->repaint();
	});
	connect(m_invertFittingOrder, &QCheckBox::stateChanged, [this]() {
	  	if (m_recomputeAutomatically->isChecked()) {
			refit();
	  	}
	  	m_renderer->repaint();
	});
	connect(m_samplingStrategy, &QComboBox::currentTextChanged, [this]() {
		if (m_recomputeAutomatically->isChecked()) {
			resample();
			refit();
		}
		m_renderer->repaint();
	});
	connect(recomputeButton, &QPushButton::clicked, [this]() {
		rebin();
	  	resample();
	  	refit();
	  	m_renderer->repaint();
	});
	connect(refitButton, &QPushButton::clicked, [this]() {
		rebin();
		refit();
		m_renderer->repaint();
	});
	connect(loadMapButton, &QPushButton::clicked, [this, loadMapButton]() {
		QString startDir = "data/chorematic_map";
		std::filesystem::path filePath = QFileDialog::getOpenFileName(this, tr("Select region map file"), startDir).toStdString();
		if (filePath == "") return;
		loadMap(filePath);
		loadMapButton->setText(QString::fromStdString(filePath.filename()));
		resample();
        auto abb = approximate(m_sampler->getArrBoundingBox());
        Box box(abb.xmin(), abb.ymin(), abb.xmax(), abb.ymax());
        m_renderer->fitInView(box);
        auto minDim = fmin(abb.xmax() - abb.xmin(), abb.ymax() - abb.ymin());
        m_gridSize->setMinimum(minDim / 100);
        m_gridSize->setMaximum(minDim);
        m_gridSize->setValue(minDim);
        m_renderer->repaint();
	});
	connect(loadDataButton, &QPushButton::clicked, [this, loadDataButton]() {
		QString startDir = "data/chorematic_map";
		std::filesystem::path filePath = QFileDialog::getOpenFileName(this, tr("Select region data file"), startDir).toStdString();
		if (filePath == "") return;
		loadData(filePath);
		loadDataButton->setText(QString::fromStdString(filePath.filename()));
		auto [rdMin, rdMax, _, blah] = regionDataData(*m_choropleth->m_data);
		m_threshold->setMinimum(rdMin);
		m_threshold->setMaximum(rdMax);
		m_threshold->setValue(0);
		rebin();
	  	if (m_recomputeAutomatically->isChecked()) {
			refit();
			m_renderer->repaint();
	  	}
        m_renderer->repaint();
	});
    connect(showLabels, &QCheckBox::stateChanged, [this, showLabels]() {
        m_choroplethP->m_options.drawLabels = showLabels->isChecked();
        m_renderer->repaint();
    });
    connect(m_dataAttribute, &QComboBox::currentTextChanged, [this]() {
        QString newAttribute = m_dataAttribute->currentText();
        m_choropleth->m_data = std::make_shared<RegionWeight>(m_regionWeightMap->at(newAttribute.toStdString()));
        m_dataInfoLabel->setText(QString::fromStdString(regionDataInfo(*m_choropleth->m_data)));
        auto [rdMin, rdMax, _, blah] = regionDataData(*m_choropleth->m_data);
        m_threshold->setMinimum(rdMin);
        m_threshold->setMaximum(rdMax);
        m_threshold->setValue(0);
        rebin();
	  	if (m_recomputeAutomatically->isChecked()) {
			refit();
	  	}
	  	m_renderer->repaint();
    });
    connect(colorPickerButton, &QPushButton::clicked, [this]() {
        auto bin = m_colorBinSelector->value();
        QColor qColor = QColorDialog::getColor();
        m_choroplethP->m_colors[bin] = Color(qColor.red(), qColor.green(), qColor.blue());
        m_renderer->repaint();
    });
    connect(m_numberOfBins, QOverload<int>::of(&QSpinBox::valueChanged), [this]() {
        rebin();
		recolor();
		if (m_recomputeAutomatically->isChecked()) {
			refit();
		}
        m_renderer->repaint();
    });
    connect(samplePerRegion, &QCheckBox::stateChanged, [this, samplePerRegion]() {
        m_sampler->setSamplePerRegion(samplePerRegion->isChecked());
        if (m_recomputeAutomatically->isChecked()) {
            resample();
            refit();
            m_renderer->repaint();
        }
    });
    connect(m_applyHeuristic, &QCheckBox::stateChanged, [this]() {
        if (m_recomputeAutomatically->isChecked()) {
            refit();
            m_renderer->repaint();
        }
    });
    connect(m_useSymDiff, &QCheckBox::stateChanged, [this]() {
        if (m_recomputeAutomatically->isChecked()) {
            refit();
            m_renderer->repaint();
        }
    });
    connect(exportDataButton, &QPushButton::clicked, [this]() {
        auto csv = regionDataToCSV(*(m_choropleth->m_data));
        QString startDir = "data/chorematic_map";
        std::filesystem::path filePath = QFileDialog::getSaveFileName(this, tr("Save region data"), startDir).toStdString();
        std::ofstream csvFile(filePath);
        csvFile << csv;
        csvFile.close();
    });
}

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	ChorematicMapDemo demo;
	demo.show();
	app.exec();
}
