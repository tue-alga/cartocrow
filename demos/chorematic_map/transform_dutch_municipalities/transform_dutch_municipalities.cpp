#include "transform_dutch_municipalities.h"

#include <QApplication>

#include <gdal/ogrsf_frmts.h>
#include "cartocrow/core/core.h"
#include "cartocrow/core/boundary_map.h"
#include "cartocrow/core/arrangement_map.h"
#include "cartocrow/core/timer.h"
#include "cartocrow/renderer/geometry_painting.h"
#include "cartocrow/renderer/geometry_widget.h"
#include "cartocrow/renderer/ipe_renderer.h"
#include "cartocrow/simplification/painting.h"
#include "cartocrow/simplification/vertex_removal/visvalingam_whyatt.h"

#include <QPushButton>

using namespace cartocrow::simplification;

//RegionArrangement
//simplifyRegionArrangement(RegionArrangement& arr) {
//    using VRTraits = VWTraits<std::string>;
//    VRTraits::Map map = regionArrangementToArrangementMap(arr);
//    auto hist = HistoricArrangement<VWTraits<std::string>>(*(this->map));
//    VWSimplificationWithHistory<std::string> simplification(*hist);
//}

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

VWDemo::VWDemo() {
//    CGAL::Arr_face_extended_text_formatter<RegionArrangement> formatter;
//    RegionArrangement regionArr;
//    std::ifstream in_file("regionArr.dat");
//    CGAL::IO::read(regionArr, in_file, formatter);

//    auto regionMap = dutchMunicipalities();
//    auto regionArr = regionMapToArrangementParallel(*regionMap);
//
    setWindowTitle("Load, transform and save Dutch municipality data and geometry");

    m_renderer = new GeometryWidget();
    setCentralWidget(m_renderer);

    std::filesystem::path file =
            std::filesystem::absolute(std::filesystem::path("data/europe.ipe"));
    std::cout << "reading file " << file << "\n";

    // step 1: create a RegionMap
//	this->inputmap = std::make_shared<BoundaryMap>(ipeToBoundaryMap(file));
//    this->inputmap = std::make_shared<RegionArrangement>(regionArr);
    this->inputmap = std::make_shared<RegionArrangement>(regionMapToArrangement(ipeToRegionMap(file)));

    std::cout << "creating arrangement\n";

    // step 2: convert this to an arrangement with the VWTraits
    // and wrap it in a historic arrangement to allow for quickly recovering all
    // solution
//	this->map =
//	    std::make_shared<VWTraits<>::Map>(boundaryMapToArrangementMap<VWVertex<>, VWEdge<>, std::monostate>(*(this->inputmap)));
    this->map = std::make_shared<VWTraits<std::string>::Map>(regionArrangementToArrangementMap<VWVertex<std::string>, VWEdge<std::string>>(*(this->inputmap)));
    auto regionArr = arrangementMapToRegionArrangement(*map);
    std::cout << this->map->is_valid() << std::endl;
    std::cout << regionArr.is_valid() << std::endl;
    this->hist = new HistoricArrangement<VWTraits<std::string>>(*(this->map));

    int incnt = this->map->number_of_edges();
    std::cout << "in count " << incnt << "\n";

    Timer t;
    // step 3: initialize the algorithm
    VWSimplificationWithHistory<std::string> simplification(*hist);
    simplification.initialize();
    t.stamp("Initialization");

    // step 4: simplify until no more vertices can be removed
    simplification.simplify(0);
    t.stamp("Simplification done");
    t.output();

    int outcnt = this->map->number_of_edges();

    std::cout << "out count " << outcnt << "\n";

    // compare running time vs no history
    //std::cout << "creating arrangement\n";
    //VWTraits::Map map2 =regionMapToArrangement<VWVertex, VWEdge>(*regions);
    //ObliviousArrangement<VWTraits> mapmod(map2);
    //int incnt2 = map2.number_of_edges();
    //std::cout << "in count " << incnt2 << "\n";
    //Timer t2;
    //VWSimplification simplification2(mapmod);
    //simplification2.initialize();
    //t2.stamp("Initialization");
    //simplification2.simplify(0);
    //t2.stamp("Simplification done");
    //t2.output();
    //int outcnt2 = map2.number_of_edges();
    //std::cout << "out count " << outcnt2 << "\n";

    // initialize a gui with a slider to retrieve all intermediate solutions
    this->c = (3 * outcnt + incnt) / 4;
    QToolBar* toolBar = new QToolBar();
    toolBar->addWidget(new QLabel("c = "));
    m_cSlider = new QSlider(Qt::Horizontal);
    m_cSlider->setMinimum(outcnt);
    m_cSlider->setMaximum(incnt);
    m_cSlider->setValue(this->c);
    toolBar->addWidget(this->m_cSlider);
    addToolBar(toolBar);
    m_cLabel = new QLabel(QString::number(this->c));
    toolBar->addWidget(this->m_cLabel);
    connect(m_cSlider, &QSlider::valueChanged, [&](int value) {
        this->c = value;
        m_cLabel->setText(QString::number(this->c));
        recalculate();
    });

    QPushButton* save = new QPushButton();
    toolBar->addWidget(save);
    connect(save, &QPushButton::clicked, [this]() {
//        this->map
        // Write the arrangement to a file.
        std::stringstream ss;
        ss << "municipalities_" << this->c << ".dat";
        std::ofstream out_file(ss.str());
        auto regionArr = arrangementMapToRegionArrangement(*map);
        CGAL::Arr_face_extended_text_formatter<RegionArrangement> formatter;
        CGAL::IO::write(regionArr, out_file, formatter);
        out_file.close();
    });

    BoundaryPainting::Options in_options;
    in_options.line_width = 1;
//	auto in_painting = std::make_shared<ArrangementPainting>(this->inputmap, in_options);
//    auto out_painting = std::make_shared<ArrangementPainting<VWTraits<std::string>::Map>>(this->map, out_options);

    ArrangementPainting<VWTraits<std::string>::Map>::Options out_options;
    out_options.color = Color{200, 10, 50};
    out_options.line_width = 2;
    auto out_painting = std::make_shared<ArrangementPainting<VWTraits<std::string>::Map>>(this->map, out_options);

    m_renderer->clear();
//	m_renderer->addPainting(in_painting, "Input map");
    m_renderer->addPainting(out_painting, "Output map");

    recalculate();
}

void VWDemo::recalculate() {
    hist->recallComplexity(c);
    std::cout << map->is_valid() << std::endl;
    m_renderer->update();
}

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    VWDemo demo;
    demo.show();
    app.exec();
}
