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

#include "parse_input.h"

#include "cartocrow/core/arrangement_map.h"
#include "cartocrow/core/arrangement_helpers.h"
#include "cartocrow/core/centroid.h"
#include "cartocrow/core/cs_polygon_helpers.h"
#include "cartocrow/core/rectangle_helpers.h"
#include "cartocrow/chorematic_map/maximum_weight_disk.h"
#include "cartocrow/simplification/vertex_removal/visvalingam_whyatt.h"
//#include "cartocrow/chorematic_map/n

#include "cartocrow/renderer/ipe_renderer.h"

#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/Boolean_set_operations_2/oriented_side.h>
#include <CGAL/General_polygon_set_2.h>

#include <gdal/ogrsf_frmts.h>

std::shared_ptr<std::unordered_map<std::string, RegionWeights>>
regionDataMapFromGPKG(const std::filesystem::path& path, const std::string& layerName, const std::string& regionNameAttribute,
                      std::function<std::string(std::string)> regionNameTransform) {
    GDALAllRegister();
    GDALDataset *poDS;

    poDS = (GDALDataset*) GDALOpenEx( path.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr );
    if( poDS == nullptr )
    {
        printf( "Open failed.\n" );
        exit( 1 );
    }
    OGRLayer* poLayer = poDS->GetLayerByName( layerName.c_str() );

    auto regionDataMap = std::make_shared<std::unordered_map<std::string, RegionWeights>>();
    std::unordered_map<std::string, RegionWeights>& dataMap = *regionDataMap;
    poLayer->ResetReading();

    for (auto& poFeature : *poLayer) {
        // todo remove
//        std::string water = poFeature->GetFieldAsString(poFeature->GetFieldIndex("water"));
//        if (water != "NEE") continue;

        std::string regionId = regionNameTransform(poFeature->GetFieldAsString(poFeature->GetFieldIndex(regionNameAttribute.c_str())));
        int i = 0;
        for( auto&& oField: *poFeature ) {
            std::string name = poFeature->GetDefnRef()->GetFieldDefn(i)->GetNameRef();
            double w;
            switch(oField.GetType()) {
                case OFTInteger:
                    w = oField.GetInteger();
                    break;
                case OFTReal:
                    w = oField.GetDouble();
                    break;
            }
            if (w != -99999999) {
                dataMap[name][regionId] = w;
            }
            ++i;
        }
    }

    return regionDataMap;
}

std::shared_ptr<RegionMap> regionMapFromGPKG(const std::filesystem::path& path,
                                             const std::string& layerName,
                                             const std::string& regionNameAttribute) {
    GDALAllRegister();
    GDALDataset       *poDS;

    poDS = (GDALDataset*) GDALOpenEx( path.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr );
    if( poDS == nullptr )
    {
        printf( "Open failed.\n" );
        exit( 1 );
    }
    OGRLayer* poLayer = poDS->GetLayerByName( layerName.c_str() );

    auto regionMap = std::make_shared<RegionMap>();
    RegionMap& regions = *regionMap;
    poLayer->ResetReading();

    for (auto& poFeature : *poLayer) {
//        std::string water = poFeature->GetFieldAsString(poFeature->GetFieldIndex("water"));
//        if (water != "NEE") continue;
        std::string regionId = poFeature->GetFieldAsString(poFeature->GetFieldIndex(regionNameAttribute.c_str()));
        OGRGeometry *poGeometry;

        PolygonSet<Exact> polygonSet;
        poGeometry = poFeature->GetGeometryRef();
        if( wkbFlatten(poGeometry->getGeometryType()) == wkbMultiPolygon ) {
            OGRMultiPolygon *poMultiPolygon = poGeometry->toMultiPolygon();

            for (auto &poly: *poMultiPolygon) {
                for (auto &linearRing: *poly) {
                    Polygon<Exact> polygon;
                    for (auto &pt: *linearRing) {
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
        } else if (wkbFlatten(poGeometry->getGeometryType()) == wkbPolygon) {
            OGRPolygon* poly = poGeometry->toPolygon();

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
        } else {
            std::cout << "Did not handle this type of geometry: " << poGeometry->getGeometryName() << std::endl;
        }
        if (regions.contains(regionId)) {
            Region& existingRegion = regions.at(regionId);
            PolygonSet<Exact>& existingPolygonSet = existingRegion.shape;
            existingPolygonSet.join(polygonSet);
        } else {
            Region region;
            region.shape = polygonSet;
            region.name = regionId;
            regions[regionId] = region;
        }
    }

    return regionMap;
}

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
    Rectangle<Exact> m_bb;

public:
    RegionMapPainting(std::shared_ptr<RegionMap> map) : m_map(std::move(map)) {
//        std::vector<Box> bboxes;
//        for (auto& [name, region] : m_map) {
//            std::vector<PolygonWithHoles<Exact>> pwhs;
//            region.shape.polygons_with_holes(std::back_inserter(pwhs));
//            for (auto& pwh : pwhs) {
//                bboxes.push_back(pwh.bbox());
//            }
//        }
//        m_bb = CGAL::bbox_2(bboxes.begin(), bboxes.end());
    };

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
    m_sampler->reweightTriangulation();
    m_sampler->setSeed(m_seed->value());

	switch (m_samplingStrategy->currentIndex()) {
	case 0: {
        m_sampler->uniformRandomSamples(m_nSamples->value(), std::back_inserter(m_samples));
		break;
	}
	case 1: {
        m_sampler->uniformRandomSamplesWeighted(m_nSamples->value(),
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
		                          std::back_inserter(m_samples),
		                          [&progress](int i) { progress.setValue(i); },
		                          [&progress]() { return progress.wasCanceled(); });
		break;
	}
	case 4: {
		m_sampler->squareGrid(m_gridSize->value() / 10, std::back_inserter(m_samples));
		break;
	}
	case 5: {
		m_sampler->hexGrid(m_gridSize->value() / 10, std::back_inserter(m_samples));
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
//	std::vector<double> thresholds({static_cast<double>(m_threshold->value())});
//	m_choropleth->setThresholds(thresholds.begin(), thresholds.end());
    m_choropleth->naturalBreaks(m_numberOfBins->value());
	m_choropleth->rebin();
	int binToFit = m_binFit->value();
    m_binFit->setMaximum(m_choropleth->numberOfBins()-1);
    m_colorBinSelector->setMaximum(m_choropleth->numberOfBins()-1);
	m_regionWeight->clear();

    Number<Exact> totalArea = 0;
    auto binAreas = m_choropleth->binAreas();
    for (const auto& binArea : binAreas) {
        totalArea += binArea;
    }
    auto posNegRatio = binAreas[binToFit] / totalArea;
	for (const auto& [region, _] : *(m_choropleth->m_data)) {
		auto bin = m_choropleth->regionToBin(region);
		if (!bin.has_value()) continue;

		m_regionWeight->operator[](region) = *bin == binToFit ? 1 : -CGAL::to_double(posNegRatio);
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
	m_sampler->setRegionArr(newArr);
	m_choropleth->m_arr = newArr;
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

    m_dataAttribute = new QComboBox();
    vLayout->addWidget(m_dataAttribute);

	m_dataInfoLabel = new QLabel();
	vLayout->addWidget(m_dataInfoLabel);

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
    m_numberOfBins->setMinimum(0);
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
	m_samplingStrategy->addItem("Uniform random weighted");
	m_samplingStrategy->addItem("Centroids");
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

//	auto* moveToCentroids = new QPushButton("Move to centroids");
//	vLayout->addWidget(moveToCentroids);

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
	m_recomputeAutomatically = new QCheckBox("Recompute automatically");
	m_recomputeAutomatically->setChecked(true);
	vLayout->addWidget(m_recomputeAutomatically);

	auto* recomputeButton = new QPushButton("Recompute");
	vLayout->addWidget(recomputeButton);
	auto* refitButton = new QPushButton("Refit");
	vLayout->addWidget(refitButton);

//	std::filesystem::path dataPath = "data/chorematic_map/test_data.txt";
//	std::filesystem::path mapPath = "data/chorematic_map/test_region_arrangement.ipe";
//	auto regionData =std::make_shared<std::unordered_map<std::string, double>>(parseRegionDataFile(dataPath));
    std::filesystem::path gpkg1 = "data/chorematic_map/HE_NL_NUTS_TUe.gpkg";
    std::filesystem::path gpkg2 = "data/chorematic_map/wijkenbuurten_2020_v3.gpkg";
    std::filesystem::path gpkg3 = "data/chorematic_map/wijkenbuurten_2022_v3.gpkg";
//    std::filesystem::path dutch = "data/chorematic_map/gemeenten-2022_92959vtcs.ipe";
    std::filesystem::path dutch = "data/chorematic_map/gemeenten-2022_19282vtcs.ipe";

    auto regionMap = std::make_shared<RegionMap>(ipeToRegionMap(dutch, true));
    m_regionWeightMap = regionDataMapFromGPKG(gpkg3, "gemeenten", "gemeentecode", [](const std::string& s) {
        return s;
    });

    // Non-generalized Dutch municipalities
//	auto regionMapDM = regionMapFromGPKG(gpkg3, "gemeenten", "gemeentecode");
//    auto regionArrangementDM = std::make_shared<RegionArrangement>(regionMapToArrangementParallel(*regionMapDM));
//    auto rp = std::make_shared<RegionMapPainting>(regionMapDM);
//    IpeRenderer ipeRenderer;
//    ipeRenderer.addPainting(rp);
//    ipeRenderer.save("gemeenten-2022.ipe");

//    m_regionWeightMap = regionDataMapFromGPKG(gpkg2, "gemeenten", "jrstatcode", [](const std::string& s) {
//        return s.substr(4);
//    });


    // Dutch municipalities
//	auto regionMap = regionMapFromGPKG(gpkg1, "NL_DP1k_LAU_1M", "LAU_ID");
//    m_regionWeightMap = regionDataMapFromGPKG(gpkg2, "gemeenten", "jrstatcode", [](const std::string& s) {
//        return s.substr(4);
//    });
    auto regionData = std::make_shared<std::unordered_map<std::string, double>>(m_regionWeightMap->begin()->second);
    // Hessen
//    auto regionMap = regionMapFromGPKG(gpkg1, "HE_1000k_stats", "GEN");
//    m_regionWeightMap = regionDataMapFromGPKG(gpkg1, "HE_1000k_stats", "GEN", [](const std::string& s) {
//        return s;
//    });
//    auto regionData = std::make_shared<std::unordered_map<std::string, double>>(m_regionWeightMap->begin()->second);
    for (auto& kv : *m_regionWeightMap) {
        m_dataAttribute->addItem(QString::fromStdString(kv.first));
    }
    m_dataAttribute->model()->sort(0);
//	auto regionArr = std::make_shared<RegionArrangement>(regionMapToArrangementParallel(*regionMap));
//    auto regionArr = regionMapToArrangementParallel(*regionMap);
    auto regionArr = regionMapToArrangementParallel(*regionMap);

    IpeRenderer ipeRenderer;
    ipeRenderer.addPainting([regionArr](GeometryRenderer& renderer) {
        auto ubf = regionArr.unbounded_face();

        for (auto ccb = ubf->inner_ccbs_begin(); ccb != ubf->inner_ccbs_end(); ++ccb) {
            Polygon<Exact> poly = ccb_to_polygon<Exact>(*ccb);
            poly.reverse_orientation();
            renderer.setMode(GeometryRenderer::stroke);
            renderer.setStroke(Color{0, 0, 0}, 1.0);
            renderer.draw(poly);
        }
    }, "Outline");
    ipeRenderer.save("hessen-outline.ipe");


    // Write the arrangement to a file.
//    std::ofstream out_file("regionArr.dat");
    CGAL::Arr_face_extended_text_formatter<RegionArrangement> formatter;
//    CGAL::IO::write(*regionArr, out_file, formatter);
//    out_file.close();
//
//    RegionArrangement regionArr;
//    std::ifstream in_file("regionArr.dat");
//    CGAL::IO::read(regionArr, in_file, formatter);
//    in_file.close();
    m_renderer->setMinZoom(0.00000001);
    m_renderer->setMaxZoom(1000000000);
//    m_renderer->fitInView(Box(9000, 300000, 300000, 615000));
//    m_renderer->fitInView(Box(3858769, 3079039,4135284, 3388358));

//    std::vector<double> thresholds({0});
//	m_choropleth = std::make_unique<Choropleth>(std::make_shared<RegionArrangement>(regionArr), std::move(regionData),
//	                                            thresholds.begin(), thresholds.end());
    m_choropleth = std::make_unique<Choropleth>(std::make_shared<RegionArrangement>(regionArr), std::move(regionData), 2);
	m_regionWeight = std::make_shared<std::unordered_map<std::string, double>>(*m_choropleth->m_data);

//	m_pl = std::make_shared<LandmarksPl>(*m_choropleth->m_arr);
	m_sampler = std::make_unique<Sampler<LandmarksPl>>(m_choropleth->m_arr, m_regionWeight, m_seed->value());
	std::vector<Color> colors({Color(200, 200, 200), Color(150, 150, 150)});
	m_choroplethP = std::make_shared<ChoroplethPainting>(*m_choropleth, colors.begin(), colors.end(), showLabels->isChecked(), Color(255, 0, 0));
	m_renderer->addPainting(m_choroplethP, "Choropleth");

    Rectangle<Inexact> abb = approximate(m_sampler->getArrBoundingBox());
    auto minDim = fmin(abb.xmax() - abb.xmin(), abb.ymax() - abb.ymin());
    m_gridSize->setMinimum(minDim / 100);
    m_gridSize->setMaximum(minDim);
    m_gridSize->setValue(minDim);
    Box box(abb.xmin(), abb.ymin(), abb.xmax(), abb.ymax());
    m_renderer->fitInView(box);

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

	rebin();
	resample();
	refit();

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
//	connect(moveToCentroids, &QPushButton::clicked, [this]() {
//		std::vector<Point<Exact>> newSamples;
//		for (int i = 0; i < m_comps.size(); ++i) {
//			auto& compArr = *m_compArrs[i];
//			auto& pl = *m_pls[i];
//			// Determine sample points in each connected component.
//			// Create and draw Voronoi diagram of the points.
//			std::vector<Point<Exact>> samplesInComponent;
//			PolygonWithHoles<Exact>& componentPolygon = m_outerPolys[i];
//			for (const auto& pt : m_samples) {
//				Point<Exact> exactPoint(pt.point.x(), pt.point.y());
//				if (CGAL::oriented_side(exactPoint, componentPolygon) != CGAL::NEGATIVE) {
//					samplesInComponent.push_back(exactPoint);
//				}
//			}
//			if (samplesInComponent.empty()) continue;
//			voronoiMoveToCentroid(compArr, pl, samplesInComponent.begin(), samplesInComponent.end(), std::back_inserter(newSamples), m_bbs[i]);
//		}
//		m_samples.clear();
//		for (const auto& pt : newSamples) {
//			m_samples.emplace_back(approximate(pt), 0);
//		}
//		reweight();
//		refit();
//	  	m_renderer->repaint();
//	});
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
	  	m_sampler->setRegionWeight(m_regionWeight);
		loadDataButton->setText(QString::fromStdString(filePath.filename()));
		auto [rdMin, rdMax, _, blah] = regionDataData(*m_choropleth->m_data);
		m_threshold->setMinimum(rdMin);
		m_threshold->setMaximum(rdMax);
		m_threshold->setValue(0);
		rebin();
	  	reweight();
        m_renderer->repaint();
	});
    connect(showLabels, &QCheckBox::stateChanged, [this, showLabels]() {
        m_choroplethP->m_drawLabels = showLabels->isChecked();
        m_renderer->repaint();
    });
    connect(m_dataAttribute, &QComboBox::currentTextChanged, [this]() {
        QString newAttribute = m_dataAttribute->currentText();
        m_choropleth->m_data = std::make_shared<RegionWeights>(m_regionWeightMap->at(newAttribute.toStdString()));
        m_regionWeight = std::make_shared<std::unordered_map<std::string, double>>(*m_choropleth->m_data);
        m_dataInfoLabel->setText(QString::fromStdString(regionDataInfo(*m_choropleth->m_data)));
        m_sampler->setRegionWeight(m_regionWeight);
        auto [rdMin, rdMax, _, blah] = regionDataData(*m_choropleth->m_data);
        m_threshold->setMinimum(rdMin);
        m_threshold->setMaximum(rdMax);
        m_threshold->setValue(0);
        rebin();
        reweight();
    });
    connect(colorPickerButton, &QPushButton::clicked, [this]() {
        auto bin = m_colorBinSelector->value();
        QColor qColor = QColorDialog::getColor();
        m_choroplethP->m_colors[bin] = Color(qColor.red(), qColor.green(), qColor.blue());
        m_renderer->repaint();
    });
    connect(m_numberOfBins, QOverload<int>::of(&QSpinBox::valueChanged), [this]() {
        rebin();
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
}

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	ChorematicMapDemo demo;
	demo.show();
	app.exec();
}
