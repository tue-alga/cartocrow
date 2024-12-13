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

#include <utility>

#include "parse_input.h"

#include "cartocrow/core/arrangement_helpers.h"
#include "cartocrow/core/cs_polygon_helpers.h"
#include "cartocrow/chorematic_map/maximum_weight_disk.h"
#include "cartocrow/chorematic_map/disk_area.h"

#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/Boolean_set_operations_2/oriented_side.h>
#include <CGAL/General_polygon_set_2.h>
#include <CGAL/centroid.h>

#include <gdal/ogrsf_frmts.h>

std::shared_ptr<RegionMap> dutchMunicipalities() {
    GDALAllRegister();
    GDALDataset       *poDS;

    poDS = (GDALDataset*) GDALOpenEx( "data/chorematic_map/wijkenbuurten_2024_v1.gpkg", GDAL_OF_VECTOR, nullptr, nullptr, nullptr );
    if( poDS == nullptr )
    {
        printf( "Open failed.\n" );
        exit( 1 );
    }
    OGRLayer* poLayer = poDS->GetLayerByName( "gemeenten" );

    auto regionMap = std::make_shared<RegionMap>();
    RegionMap& regions = *regionMap;
    poLayer->ResetReading();

    for (auto& poFeature : *poLayer) {
        std::string water = poFeature->GetFieldAsString(poFeature->GetFieldIndex("water"));
        if (water != "NEE") continue;
        std::string regionId = poFeature->GetFieldAsString(0);
        OGRGeometry *poGeometry;

        poGeometry = poFeature->GetGeometryRef();
        if( wkbFlatten(poGeometry->getGeometryType()) == wkbMultiPolygon ){
            OGRMultiPolygon* poMultiPolygon = poGeometry->toMultiPolygon();

            PolygonSet<Exact> polygonSet;
            for (auto& poly : *poMultiPolygon) {
                for (auto& linearRing : *poly) {
                    Polygon<Exact> polygon;
                    for (auto& pt : *linearRing) {
                        polygon.push_back({pt.getX(), pt.getY()});
                    }
                    // if the begin and end vertices are equal, remove one of them
                    if (polygon.container().front() == polygon.container().back()) {
                        polygon.container().pop_back();
                    }
                    if (polygon.is_clockwise_oriented()) {
                        polygon.reverse_orientation();
                    }
                    polygonSet.symmetric_difference(polygon);
                }
            }
            Region region;
            region.shape = polygonSet;
            region.name = regionId;
            regions[regionId] = region;
        } else {
            printf( "no multipolygon geometry\n" );
        }
    }

    std::cout << "Read Dutch municipalities" << std::endl;
    return regionMap;
}

class ArrangementPainting : public GeometryPainting {
  private:
	std::shared_ptr<Arrangement<Exact>> m_arr;

  public:
	ArrangementPainting(std::shared_ptr<Arrangement<Exact>> arr) : m_arr(std::move(arr)) {};

	void paint(GeometryRenderer &renderer) const override {
		for (auto eit = m_arr->edges_begin(); eit != m_arr->edges_end(); ++eit) {
			Segment<Exact> seg = eit->curve();
			renderer.draw(seg);
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

void ChorematicMapDemo::resample() {
	m_samples.clear();
	m_disk = std::nullopt;
	m_sampler->reweight_triangulation();
	m_sampler->set_seed(m_seed->value());

	switch (m_samplingStrategy->currentIndex()) {
	case 0: {
		m_sampler->uniform_random_samples(m_nSamples->value(), std::back_inserter(m_samples));
		break;
	}
	case 1: {
		m_sampler->uniform_random_samples_weighted(m_nSamples->value(),
		                                           std::back_inserter(m_samples));
		break;
	}
	case 2: {
		m_sampler->centroids(std::back_inserter(m_samples));
		break;
	}
	case 3: {
		QProgressDialog progress("Voronoi iterations...", "Abort", 0, m_voronoiIters->value(), this);
		progress.setWindowModality(Qt::WindowModal);
		progress.setMinimumDuration(1000);
		m_sampler->voronoiUniform(m_nSamples->value(), m_voronoiIters->value(),
		                          std::back_inserter(m_samples), m_compArrs, m_pls, m_bbs,
		                          m_outerPolys, [&progress](int i) { progress.setValue(i); },
		                          [&progress]() { return progress.wasCanceled(); });
		break;
	}
	case 4: {
		m_sampler->squareGrid(m_gridSize->value() / 10, m_bb, std::back_inserter(m_samples));
		break;
	}
	case 5: {
		m_sampler->hexGrid(m_gridSize->value() / 10, m_bb, std::back_inserter(m_samples));
		break;
	}
	default: {
		std::cerr << "Unimplemented sampling strategy: " << m_samplingStrategy->currentIndex()
		          << " " << m_samplingStrategy->currentText().toStdString() << std::endl;
		return;
	}
	}
}

void ChorematicMapDemo::rebin() {
	std::vector<double> thresholds({static_cast<double>(m_threshold->value())});
	m_choropleth->setThresholds(thresholds.begin(), thresholds.end());
	m_choropleth->rebin();
	int binToFit = m_binFit->value();
	m_regionWeight->clear();

	for (const auto& [region, _] : *(m_choropleth->m_data)) {
		auto bin = m_choropleth->regionToBin(region);
		if (!bin.has_value()) continue;
		m_regionWeight->operator[](region) = *bin == binToFit ? 1 : -1;
	}
}

void ChorematicMapDemo::reweight() {
	std::vector<Point<Exact>> points;
	for (const auto& s : m_samples) {
		points.push_back(makeExact(s.point));
	}
	m_samples.clear();
	m_sampler->assignWeightsToPoints(points.begin(), points.end(), std::back_inserter(m_samples));
}

void ChorematicMapDemo::refit() {
	// This progress dialog does not work yet
	QProgressDialog progress("Fitting disk...", "Abort", 0, m_samples.size(), this);
	progress.setWindowModality(Qt::WindowModal);
	progress.setMinimumDuration(1000);
	InducedDiskW iDisk = smallest_maximum_weight_disk(m_samples.begin(), m_samples.end(),
	                                                  [&progress, this](int i) {
		                                                  progress.setValue(i);
	                                                  },
	                                                  [&progress]() { return progress.wasCanceled(); });
	m_disk = circle(iDisk);
	double score;
	if (m_disk.has_value()) {
		auto disk = makeExact(*m_disk);
		auto circleCS = circleToCSPolygon(disk);

		Number<Inexact> total = 0;
		for (auto fit = m_choropleth->m_arr->faces_begin(); fit != m_choropleth->m_arr->faces_end(); ++fit) {
			if (fit->is_unbounded()) continue;
			auto w = m_regionWeight->contains(fit->data()) ? m_regionWeight->at(fit->data()) : 0;
			if (w == 0) continue;
			auto pwh = face_to_polygon_with_holes<Exact>(fit);
			CSPolygonWithHoles pwhCS = polygonToCSPolygon(pwh);
			std::vector<CSPolygonWithHoles> inters;
			CGAL::intersection(circleCS, pwhCS, std::back_inserter(inters));
			for (const auto& inter : inters) {
				total += abs(area(inter)) * w;
			}
		}
		score = total;
	} else {
		score = 0;
	}
	m_diskCostLabel->setText(QString::fromStdString(std::to_string(score)));
}

void ChorematicMapDemo::loadMap(const std::filesystem::path& mapPath) {
	m_samples.clear();
	m_diskCostLabel->setText("");
	m_disk = std::nullopt;
	auto regionMap = ipeToRegionMap(mapPath);
	auto newArr = std::make_shared<RegionArrangement>(regionMapToArrangement(regionMap));
	m_pl = std::make_shared<LandmarksPl>(*newArr);
	m_sampler->m_regionArr = newArr;
	m_choropleth->m_arr = newArr;
	m_sampler->m_pl = m_pl;
	m_sampler->initialize_triangulation();
	computeComponentInfo();
}

void ChorematicMapDemo::computeComponentInfo() {
	m_comps = connectedComponents(*m_choropleth->m_arr, [](RegionArrangement::Face_handle fh) {
		return !fh->data().empty();
	});
	m_compArrs.clear();
	m_pls.clear();
	m_bbs.clear();
	m_outerPolys.clear();
	for (const auto& comp : m_comps) {
		auto compArr = std::make_shared<RegionArrangement>(comp.arrangement());
		copyBoundedFaceData(*m_choropleth->m_arr, *compArr);
		m_compArrs.push_back(compArr);
		m_pls.push_back(std::make_shared<LandmarksPl>(*(m_compArrs.back())));
		std::vector<Point<Exact>> points;
		for (auto vit = compArr->vertices_begin(); vit != compArr->vertices_end(); ++vit) {
			points.push_back(vit->point());
		}
		auto bb = CGAL::bbox_2(points.begin(), points.end());
		m_bbs.emplace_back(bb);
		m_outerPolys.push_back(comp.surface_polygon());
	}
	m_bb = CGAL::bbox_2(m_bbs.begin(), m_bbs.end());
}

void ChorematicMapDemo::loadData(const std::filesystem::path& dataPath) {
	m_choropleth->m_data = std::make_shared<std::unordered_map<std::string, double>>(parseRegionDataFile(dataPath));
	m_regionWeight = std::make_shared<std::unordered_map<std::string, double>>(*m_choropleth->m_data);
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

	m_dataInfoLabel = new QLabel();
	vLayout->addWidget(m_dataInfoLabel);

	auto* binningSettings = new QLabel("<h3>Binning</h3>");
	vLayout->addWidget(binningSettings);

	auto* thresholdLabel = new QLabel("Threshold");
	m_threshold = new QSlider();
	m_threshold->setOrientation(Qt::Horizontal);

	vLayout->addWidget(thresholdLabel);
	vLayout->addWidget(m_threshold);

	auto* samplingSettings = new QLabel("<h3>Sampling</h3>");
	vLayout->addWidget(samplingSettings);

	auto* sampleStrategyLabel = new QLabel("Sample strategy");
	m_samplingStrategy = new QComboBox();
	sampleStrategyLabel->setBuddy(m_samplingStrategy);
	vLayout->addWidget(sampleStrategyLabel);
	m_samplingStrategy->addItem("Uniform random");
	m_samplingStrategy->addItem("Uniform random weighted");
	m_samplingStrategy->addItem("Centroids");
	m_samplingStrategy->addItem("Voronoi");
	m_samplingStrategy->addItem("Square grid");
	m_samplingStrategy->addItem("Hexagonal grid");
	m_samplingStrategy->setCurrentIndex(0);
	vLayout->addWidget(m_samplingStrategy);

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

	auto* moveToCentroids = new QPushButton("Move to centroids");
	vLayout->addWidget(moveToCentroids);

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
	auto* binFitLabel = new QLabel("Bin");
	vLayout->addWidget(binFitLabel);
	m_binFit = new QSpinBox();
	m_binFit->setValue(0);
	m_binFit->setMinimum(0);
	m_binFit->setMaximum(5);
	vLayout->addWidget(m_binFit);

	auto* scoreLabel = new QLabel("Score:");
	vLayout->addWidget(scoreLabel);
	m_diskCostLabel = new QLabel();
	vLayout->addWidget(m_diskCostLabel);

	auto* miscellaneous = new QLabel("<h3>Miscellaneous</h3>");
	vLayout->addWidget(miscellaneous);
	auto* recomputeAutomaticallyLabel = new QLabel("Recompute automatically");
	m_recomputeAutomatically = new QCheckBox();
	m_recomputeAutomatically->setChecked(true);
	recomputeAutomaticallyLabel->setBuddy(m_recomputeAutomatically);
	vLayout->addWidget(recomputeAutomaticallyLabel);
	vLayout->addWidget(m_recomputeAutomatically);

	auto* recomputeButton = new QPushButton("Recompute");
	vLayout->addWidget(recomputeButton);
	auto* refitButton = new QPushButton("Refit");
	vLayout->addWidget(refitButton);

	std::filesystem::path dataPath = "data/chorematic_map/test_data.txt";
	std::filesystem::path mapPath = "data/chorematic_map/test_region_arrangement.ipe";
	auto regionData =std::make_shared<std::unordered_map<std::string, double>>(parseRegionDataFile(dataPath));
//	auto regionMap = dutchMunicipalities();
//	auto regionArr = std::make_shared<RegionArrangement>(regionMapToArrangementParallel(*regionMap));
    // Write the arrangement to a file.
//    std::ofstream out_file("regionArr.dat");
    CGAL::Arr_face_extended_text_formatter<RegionArrangement> formatter;
//    CGAL::IO::write(*regionArr, out_file, formatter);
//    out_file.close();
//
    RegionArrangement regionArr;
    std::ifstream in_file("regionArr.dat");
    CGAL::IO::read(regionArr, in_file, formatter);
    in_file.close();
    m_renderer->setMinZoom(0.00000001);
    m_renderer->setMaxZoom(1000000000);
    m_renderer->fitInView(Box(9000, 300000, 300000, 615000));

    std::vector<double> thresholds({0});
	m_choropleth = std::make_unique<Choropleth>(std::make_shared<RegionArrangement>(regionArr), std::move(regionData),
	                                            thresholds.begin(), thresholds.end());
	m_regionWeight = std::make_shared<std::unordered_map<std::string, double>>(*m_choropleth->m_data);
//	computeComponentInfo();

	m_pl = std::make_shared<LandmarksPl>(*m_choropleth->m_arr);
//	m_sampler = std::make_unique<Sampler<LandmarksPl>>(m_choropleth->m_arr, m_pl, m_regionWeight, m_seed->value());
	std::vector<Color> colors({Color(150, 150, 150), Color(200, 200, 200)});
	m_choroplethP = std::make_shared<ChoroplethPainting>(*m_choropleth, colors.begin(), colors.end(), true);
	m_renderer->addPainting(m_choroplethP, "Choropleth");

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
				Color c(1 + weight * 254 / maxWeight, 0, 0);
				renderer.setStroke(c, 1.0);
				renderer.draw(point);
			} else if (weight < 0) {
				Color c(0, 0, 1 + weight * 254 / minWeight);
				renderer.setStroke(c, 1.0);
				renderer.draw(point);
			} else {
				renderer.setStroke(Color{100, 100, 100}, 1.0);
				renderer.draw(point);
			}
		}
	}, "Samples");

//	rebin();
//	resample();
//	refit();

	m_dataInfoLabel->setText(QString::fromStdString(regionDataInfo(*m_choropleth->m_data)));
	auto [rdMin, rdMax, _, blah] = regionDataData(*m_choropleth->m_data);
	m_threshold->setMinimum(rdMin);
	m_threshold->setMaximum(rdMax);
	m_threshold->setValue(0);

	m_renderer->addPainting([this](renderer::GeometryRenderer& renderer) {
		if (!m_disk.has_value()) return;
		renderer.setMode(GeometryRenderer::fill | GeometryRenderer::stroke);
		renderer.setStroke(Color(0, 0, 0), 2.0);
		renderer.setFill(0x000000);
		renderer.setFillOpacity(50);
		renderer.draw(*m_disk);
	}, "Circle");

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
	  	reweight();
	  	if (m_recomputeAutomatically->isChecked()) {
			refit();
	  	}
	  	m_renderer->repaint();
	});
	connect(m_gridSize, &QSlider::valueChanged, [this]() {
		resample();
	  	if ((m_samplingStrategy->currentIndex() == 4 || m_samplingStrategy->currentIndex() == 5) && m_recomputeAutomatically->isChecked()) {
			  refit();
	  	}
	  	m_renderer->repaint();
	});
	connect(m_binFit, QOverload<int>::of(&QSpinBox::valueChanged), [this]() {
	  	rebin();
	  	reweight();
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
		reweight();
		refit();
		m_renderer->repaint();
	});
	connect(moveToCentroids, &QPushButton::clicked, [this]() {
		std::vector<Point<Exact>> newSamples;
		for (int i = 0; i < m_comps.size(); ++i) {
			auto& compArr = *m_compArrs[i];
			auto& pl = *m_pls[i];
			// Determine sample points in each connected component.
			// Create and draw Voronoi diagram of the points.
			std::vector<Point<Exact>> samplesInComponent;
			PolygonWithHoles<Exact>& componentPolygon = m_outerPolys[i];
			for (const auto& pt : m_samples) {
				Point<Exact> exactPoint(pt.point.x(), pt.point.y());
				if (CGAL::oriented_side(exactPoint, componentPolygon) != CGAL::NEGATIVE) {
					samplesInComponent.push_back(exactPoint);
				}
			}
			if (samplesInComponent.empty()) continue;
			voronoiMoveToCentroid(compArr, pl, samplesInComponent.begin(), samplesInComponent.end(), std::back_inserter(newSamples), m_bbs[i]);
		}
		m_samples.clear();
		for (const auto& pt : newSamples) {
			m_samples.emplace_back(approximate(pt), 0);
		}
		reweight();
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
	});
	connect(loadDataButton, &QPushButton::clicked, [this, loadDataButton]() {
		QString startDir = "data/chorematic_map";
		std::filesystem::path filePath = QFileDialog::getOpenFileName(this, tr("Select region data file"), startDir).toStdString();
		if (filePath == "") return;
		loadData(filePath);
	  	m_sampler->m_regionWeight = m_regionWeight;
		loadDataButton->setText(QString::fromStdString(filePath.filename()));
		auto [rdMin, rdMax, _, blah] = regionDataData(*m_choropleth->m_data);
		m_threshold->setMinimum(rdMin);
		m_threshold->setMaximum(rdMax);
		m_threshold->setValue(0);
		rebin();
	  	reweight();
	});
}

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	ChorematicMapDemo demo;
	demo.show();
	app.exec();
}
