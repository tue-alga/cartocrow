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
#include <QVBoxLayout>
#include <QProgressDialog>
#include <QColorDialog>

#include <utility>

#include "cartocrow/chorematic_map/input_parsing.h"
#include "cartocrow/chorematic_map/maximum_weight_disk.h"
#include "cartocrow/circle_segment_helpers/cs_polygon_helpers.h"
#include "cartocrow/core/arrangement_map.h"
#include "cartocrow/renderer/ipe_renderer.h"
#include "cartocrow/reader/gdal_conversion.h"

#include "demos/widgets/double_slider.h"

#include <CGAL/General_polygon_set_2.h>

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
		if (m_searchForGridSize->isChecked()) {
			m_sample = m_sampler->squareGrid(m_nSamples->value());
		} else {
			m_sample = m_sampler->squareGrid(m_gridSize->value() / 10.0);
		}
		break;
	}
	case 3: {
		if (m_searchForGridSize->isChecked()) {
			m_sample = m_sampler->hexGrid(m_nSamples->value());
		} else {
			m_sample = m_sampler->hexGrid(m_gridSize->value() / 10.0);
		}
		break;
	}
	default: {
		std::cerr << "Unimplemented sampling strategy: " << m_samplingStrategy->currentIndex()
		          << " " << m_samplingStrategy->currentText().toStdString() << std::endl;
		return;
	}
	}
	if (m_sample.m_points.size() != m_nSamples->value() && m_searchForGridSize->isChecked()) {
		std::cerr << m_sample.m_points.size() << " samples instead of the target " << m_nSamples->value() << std::endl;
	}
}

void ChorematicMapDemo::rebin() {
	if (m_useNaturalBreaks->isChecked()) {
		m_choropleth->naturalBreaks(m_numberOfBins->value());
	} else {
		auto ts = {m_threshold->value()};
		m_choropleth->setThresholds(ts.begin(), ts.end());
	}
	m_choropleth->rebin();
//    m_colorBinSelector->setMaximum(m_choropleth->numberOfBins()-1);
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
	auto ext = mapPath.extension();
	if (ext != ".ipe" && ext != ".gpkg") {
		std::cerr << "Cannot load map from file of type " << ext << std::endl;
	}
	m_sample.m_points.clear();
	m_diskScoreLabel->setText("");
	m_disks.clear();
	std::shared_ptr<RegionArrangement> newArr;
	if (ext == ".ipe") {
		auto regionMap = ipeToRegionMap(mapPath, m_labelAtCentroid->isChecked());
		newArr = std::make_shared<RegionArrangement>(regionMapToArrangementParallel(regionMap));
	} else {
		auto regionMap = regionMapFromGPKG(mapPath, m_regionNameAttribute->text().toStdString());
		newArr = std::make_shared<RegionArrangement>(regionMapToArrangementParallel(*regionMap));
	}
	m_sampler->setRegionArr(newArr);
	m_choropleth->m_arr = newArr;
}

void ChorematicMapDemo::loadData(const std::filesystem::path& dataPath) {
	auto ext = dataPath.extension();
	if (ext == ".csv") {
		m_choropleth->m_data =
		    std::make_shared<std::unordered_map<std::string, double>>(parseRegionDataFile(dataPath));
		m_dataInfoLabel->setText(QString::fromStdString(regionDataInfo(*m_choropleth->m_data)));
	} else {
		m_regionWeightMap = regionDataMapFromGPKG(dataPath, m_regionNameAttribute->text().toStdString());
		m_dataAttribute->clear();
		for (auto& kv : *m_regionWeightMap) {
    	    m_dataAttribute->addItem(QString::fromStdString(kv.first));
    	}
    	m_dataAttribute->model()->sort(0);
		m_dataAttribute->setCurrentIndex(0);
		auto regionData = std::make_shared<std::unordered_map<std::string, double>>((*m_regionWeightMap)[m_dataAttribute->currentText().toStdString()]);
		m_choropleth->m_data = regionData;
	}
}

void ChorematicMapDemo::exportToGpkg(const std::filesystem::path& outputPath) {
	const char *pszDriverName = "GPKG";
	GDALDriver *poDriver;

	GDALAllRegister();

	poDriver = GetGDALDriverManager()->GetDriverByName(pszDriverName);
	if( poDriver == nullptr )
	{
		printf("%s driver not available.\n", pszDriverName);
		exit( 1 );
	}

	GDALDataset *poDS;

	poDS = poDriver->Create(outputPath.string().c_str(), 0, 0, 0, GDT_Unknown, nullptr);
	if( poDS == nullptr )
	{
		printf( "Creation of output file failed.\n" );
		exit( 1 );
	}

	OGRLayer *poLayer;

	poLayer = poDS->CreateLayer( "regions", NULL, wkbMultiPolygon, NULL );
	if( poLayer == NULL )
	{
		printf( "Layer creation failed.\n" );
		exit( 1 );
	}

	OGRFieldDefn oNameField( "name", OFTString );

	if( poLayer->CreateField( &oNameField ) != OGRERR_NONE )
	{
		printf( "Creating name field failed.\n" );
		exit( 1 );
	}

    for (const auto& [attribute, _] : *m_regionWeightMap) {
        OGRFieldDefn oField(attribute.c_str(), OFTReal);

        if (poLayer->CreateField(&oField) != OGRERR_NONE) {
            printf("Creating field failed.\n");
            exit(1);
        }
    }

	for (const auto& region : m_sampler->getRegions()) {
		std::vector<Component<RegionArrangement>> comps;
		connectedComponents(*m_choropleth->m_arr, std::back_inserter(comps), [region](RegionArrangement::Face_handle fh) {
			return fh->data() == region;
		});
		OGRMultiPolygon mPgn;
		for (const auto& comp : comps) {
			auto cgalPgnWH = comp.surface_polygon();
            auto ogrPolygon = polygonWithHolesToOGRPolygon(cgalPgnWH);
            mPgn.addGeometry(&ogrPolygon);
		}
		OGRFeature *poFeature;

		poFeature = OGRFeature::CreateFeature( poLayer->GetLayerDefn() );
		poFeature->SetField( "name", region.c_str() );
        for (const auto& [attribute, dataMap] : *m_regionWeightMap) {
            if (dataMap.contains(region)) {
                poFeature->SetField(attribute.c_str(), dataMap.at(region));
            } else {
                poFeature->SetField(attribute.c_str(), -1);
            }
        }

		poFeature->SetGeometry( &mPgn );

		if( poLayer->CreateFeature( poFeature ) != OGRERR_NONE )
		{
			printf( "Failed to create feature in shapefile.\n" );
			exit( 1 );
		}

		OGRFeature::DestroyFeature( poFeature );
	}

    GDALClose( poDS );
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
	auto* loadMapButton = new QPushButton("Load map (ipe or gpkg)");
	auto* loadDataButton = new QPushButton("Load data (gpkg or single-attribute csv)");
	vLayout->addWidget(inputSettings);
	vLayout->addWidget(loadMapButton);
	vLayout->addWidget(loadDataButton);

	auto* regionNameAttributeLabel = new QLabel("Region name attribute");
	m_regionNameAttribute = new QLineEdit();
	vLayout->addWidget(regionNameAttributeLabel);
	vLayout->addWidget(m_regionNameAttribute);
	m_regionNameAttribute->setText("name");

	m_labelAtCentroid = new QCheckBox("Ipe map: label at centroid?");
	m_labelAtCentroid->setChecked(true);
	vLayout->addWidget(m_labelAtCentroid);

    m_dataAttribute = new QComboBox();
    vLayout->addWidget(m_dataAttribute);

	auto* exportOptions = new QLabel("<h3>Export</h3>");
	vLayout->addWidget(exportOptions);
	auto* exportDataButton = new QPushButton();
	exportDataButton->setText("Export data");
	vLayout->addWidget(exportDataButton);
	auto* exportToGpkg = new QPushButton("Export data and map to GeoPackage");
	vLayout->addWidget(exportToGpkg);

	m_dataInfoLabel = new QLabel();
	vLayout->addWidget(m_dataInfoLabel);

	auto* binningSettings = new QLabel("<h3>Binning</h3>");
	vLayout->addWidget(binningSettings);

	m_useNaturalBreaks = new QCheckBox("Use natural breaks");
	m_useNaturalBreaks->setChecked(true);
	vLayout->addWidget(m_useNaturalBreaks);

	auto* thresholdLabel = new QLabel("Threshold");
	m_threshold = new DoubleSlider();
	m_threshold->setOrientation(Qt::Horizontal);

	vLayout->addWidget(thresholdLabel);
	vLayout->addWidget(m_threshold);

    auto* choroplethSettings = new QLabel("<h3>Choropleth</h3>");
    vLayout->addWidget(choroplethSettings);
    auto* showLabels = new QCheckBox("Show labels");
    vLayout->addWidget(showLabels);
    showLabels->setChecked(false);

	// Currently more than 2 bins are not fully supported, so we do not add this option to the UI.
//    auto* numberOfBinsLabel = new QLabel("Number of bins");
    m_numberOfBins = new QSpinBox();
    m_numberOfBins->setMinimum(2);
    m_numberOfBins->setValue(2);
    m_numberOfBins->setMaximum(7);
//    vLayout->addWidget(numberOfBinsLabel);
//    vLayout->addWidget(m_numberOfBins);
//
//    auto* colorLabel = new QLabel("Color of bin");
//    m_colorBinSelector = new QSpinBox();
//    auto* colorPickerButton = new QPushButton("Choose color");
//    vLayout->addWidget(colorLabel);
//    vLayout->addWidget(m_colorBinSelector);
//    vLayout->addWidget(colorPickerButton);

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

	m_searchForGridSize = new QCheckBox("Search for grid size");
	m_searchForGridSize->setChecked(true);
	vLayout->addWidget(m_searchForGridSize);

	auto* gridSizeLabel = new QLabel("Grid size");
	m_gridSize = new DoubleSlider();
	m_gridSize->setOrientation(Qt::Horizontal);
	m_gridSize->setMinimum(1);
	m_gridSize->setMaximum(1000);
	m_gridSize->setValue(100);
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

    std::filesystem::path dutch = "data/chorematic_map/dutch_selected.gpkg";
    std::filesystem::path hessen = "data/chorematic_map/hessen_selected.gpkg";

    // Dutch
    auto regionMap = regionMapFromGPKG(dutch, "name");
    m_regionWeightMap = regionDataMapFromGPKG(dutch, "name");

    // Hessen
//    auto regionMap = regionMapFromGPKG(hessen, "name");
//    m_regionWeightMap = regionDataMapFromGPKG(hessen, "name");

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
	m_sampler = std::make_unique<Sampler>(m_choropleth->m_arr, m_seed->value());
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

	rebin();
	recolor();
	resample();
	refit();

	m_dataInfoLabel->setText(QString::fromStdString(regionDataInfo(*m_choropleth->m_data)));
	auto [rdMin, rdMax, _, blah] = regionDataData(*m_choropleth->m_data);
	m_threshold->setMinimum(rdMin);
	m_threshold->setMaximum(std::ceil(rdMax));
	m_threshold->setValue(0);

	m_renderer->addPainting([this, offBlack](GeometryRenderer& renderer) {
		renderer.setMode(GeometryRenderer::stroke);
		renderer.setStroke(offBlack, 2);

		auto&& polys = m_sampler->getLandmassPolys();
		for (const auto& poly : polys) {
			renderer.draw(poly);
		}
	}, "Outline");

	m_renderer->addPainting([this](renderer::GeometryRenderer& renderer) {
		renderer.setMode(GeometryRenderer::stroke);
		for (const auto& point : m_sample.m_points) {
			renderer.setStroke(Color{0, 0, 0}, 1.0);
			renderer.draw(point);
		}
	}, "Samples");

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
	connect(m_threshold, &DoubleSlider::valueChanged, [this]() {
	  	rebin();
	  	if (m_recomputeAutomatically->isChecked()) {
			refit();
	  	}
	  	m_renderer->repaint();
	});
	connect(m_gridSize, &DoubleSlider::valueChanged, [this]() {
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
		loadMapButton->setText(QString::fromStdString(filePath.filename().string()));
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
		loadDataButton->setText(QString::fromStdString(filePath.filename().string()));
		auto [rdMin, rdMax, _, blah] = regionDataData(*m_choropleth->m_data);
		m_threshold->setMinimum(rdMin);
		m_threshold->setMaximum(std::ceil(rdMax));
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
		if (!m_regionWeightMap->contains(newAttribute.toStdString())) return;
        m_choropleth->m_data = std::make_shared<RegionWeight>(m_regionWeightMap->at(newAttribute.toStdString()));
        m_dataInfoLabel->setText(QString::fromStdString(regionDataInfo(*m_choropleth->m_data)));
        auto [rdMin, rdMax, _, blah] = regionDataData(*m_choropleth->m_data);
        m_threshold->setMinimum(rdMin);
        m_threshold->setMaximum(std::ceil(rdMax));
        m_threshold->setValue(0);
        rebin();
	  	if (m_recomputeAutomatically->isChecked()) {
			refit();
	  	}
	  	m_renderer->repaint();
    });
//    connect(colorPickerButton, &QPushButton::clicked, [this]() {
//        auto bin = m_colorBinSelector->value();
//        QColor qColor = QColorDialog::getColor();
//        m_choroplethP->m_colors[bin] = Color(qColor.red(), qColor.green(), qColor.blue());
//        m_renderer->repaint();
//    });
//    connect(m_numberOfBins, QOverload<int>::of(&QSpinBox::valueChanged), [this]() {
//        rebin();
//		recolor();
//		if (m_recomputeAutomatically->isChecked()) {
//			refit();
//		}
//        m_renderer->repaint();
//    });
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
	connect(m_useNaturalBreaks, &QCheckBox::stateChanged, [this]() {
		rebin();
		if (m_recomputeAutomatically->isChecked()) {
			refit();
			m_renderer->repaint();
		}
	});
	connect(m_searchForGridSize, &QCheckBox::stateChanged, [this]() {
		if (m_recomputeAutomatically->isChecked()) {
			resample();
			refit();
		}
		m_renderer->repaint();
	});
    connect(exportDataButton, &QPushButton::clicked, [this]() {
        auto csv = regionDataToCSV(*(m_choropleth->m_data));
        QString startDir = "data/chorematic_map";
        std::filesystem::path filePath = QFileDialog::getSaveFileName(this, tr("Save region data"), startDir).toStdString();
        std::ofstream csvFile(filePath);
        csvFile << csv;
        csvFile.close();
    });
	connect(exportToGpkg, &QPushButton::clicked, [this]() {
	  	std::filesystem::path filePath = QFileDialog::getSaveFileName(this, tr("Save region data"), ".").toStdString();
		this->exportToGpkg(filePath);
	});
}

int main(int argc, char* argv[]) {
	QApplication app(argc, argv);
	ChorematicMapDemo demo;
	demo.show();
	app.exec();
}
