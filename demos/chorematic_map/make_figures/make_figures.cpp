#include "make_figures.h"

#include "cartocrow/chorematic_map/input_parsing.h"
#include "cartocrow/chorematic_map/choropleth.h"
#include "cartocrow/chorematic_map/sampler.h"
#include "cartocrow/chorematic_map/choropleth_disks.h"
#include "cartocrow/core/region_arrangement.h"
#include "cartocrow/renderer/ipe_renderer.h"

#include <CGAL/Arr_landmarks_point_location.h>

using namespace cartocrow;
using namespace cartocrow::chorematic_map;
using namespace cartocrow::renderer;

using LandmarksPl = CGAL::Arr_landmarks_point_location<RegionArrangement>;

RenderPath& operator<<(RenderPath& path, const Polygon<Inexact> p) {
    for (auto vertex = p.vertices_begin(); vertex != p.vertices_end(); vertex++) {
        if (vertex == p.vertices_begin()) {
            path.moveTo(*vertex);
        } else {
            path.lineTo(*vertex);
        }
    }
    path.close();
    return path;
}

PolygonWithHoles<Inexact> transform(const CGAL::Aff_transformation_2<Inexact>& t, const PolygonWithHoles<Inexact>& pwh) {
    Polygon<Inexact> outerT;
    if (!pwh.is_unbounded()) {
        outerT = transform(t, pwh.outer_boundary());
    }
    std::vector<Polygon<Inexact>> holesT;
    for (const auto& h : pwh.holes()) {
        holesT.push_back(transform(t, h));
    }
    return {outerT, holesT.begin(), holesT.end()};
}

int main() {
    std::string name = "dutch";
    std::filesystem::path dataPath = "data/chorematic_map/wijkenbuurten_2022_v3.gpkg";
    std::filesystem::path mapPath = "data/chorematic_map/gemeenten-2022_5000vtcs.ipe";
    int numberOfBins = 2;
    int seed = 0;
    bool applyHeuristic = true;
    int nSamples = 1000;
    bool perRegion = true;
    Point<Inexact> legendTL(70.0, 80.0);

    // Apply an orthogonal transformation (so no stretching) to position the diagram.
    CGAL::Aff_transformation_2<Inexact> transformation(CGAL::SCALING, 0.06);

    std::vector<std::pair<std::string, std::string, std::string>> names = {
        {"apotheek_gemiddelde_afstand_in_km", "Average distance to pharmacy", " km"},
    };

    auto regionMap = std::make_shared<RegionMap>(ipeToRegionMap(mapPath, true));
    auto regionWeightMap = regionDataMapFromGPKG(dataPath, "gemeenten", "gemeentecode", [](const std::string& s) {
        return s;
    });
    std::shared_ptr<RegionArrangement> regionArr = std::make_shared<RegionArrangement>(regionMapToArrangementParallel(*regionMap));

    auto sampler = std::make_shared<Sampler<LandmarksPl>>(regionArr, seed, perRegion);

    for (auto& [attribute, title, suffix] : names) {
        std::shared_ptr<RegionWeight> regionData = std::make_shared<RegionWeight>(regionWeightMap->at(attribute));
        auto choropleth = std::make_shared<Choropleth>(regionArr, regionData, 2);

        std::vector<Color> colors;
        switch(numberOfBins) {
            case 2: {
                // There are no colorbrewer 2 class colors. I have picked a light one of 3-class and a dark one from 4-class.
                colors = std::vector({Color(0xe5f5e0), Color(0x74c476)});
                break;
            }
            case 3: {
                // https://colorbrewer2.org/#type=sequential&scheme=Greens&n=3
                colors = std::vector({Color(0xe5f5e0), Color(0xa1d99b), Color(0x31a354)});
                break;
            }
            default: {
                throw std::runtime_error("No colors specific for this number of bins.");
            }
        }

        auto choroplethP = std::make_shared<ChoroplethPainting>(*choropleth, colors.begin(), colors.end(), false, Color(200, 200, 200), transformation, 0.4);
        IpeRenderer ipeRenderer;
        ipeRenderer.addPainting(choroplethP, "Choropleth");

        auto sample = sampler->voronoiUniform(nSamples, 5);
        auto disks0 = fitDisks(*choropleth, sample, false, true, applyHeuristic);
        auto disks1 = fitDisks(*choropleth, sample, false, true, applyHeuristic);
        double score0 = 0;
        for (const auto& disk : disks0) {
            score0 += *(disk.score);
        }
        double score1 = 0;
        for (const auto& disk : disks1) {
            score1 += *(disk.score);
        }

        auto& disks = score0 > score1 ? disks0 : disks1;

        ipeRenderer.addPainting([choroplethP, sampler, disks, transformation](renderer::GeometryRenderer& renderer) {
            RenderPath renderPath;
            auto&& polys = sampler->getLandmassPolys();
            for (const auto& poly : polys) {
                renderPath << transform(transformation, approximate(poly.outer_boundary()));
                for (const auto& h : poly.holes()) {
                    renderPath << transform(transformation, approximate(h));
                }
            }

            renderer.setClipping(true);
            renderer.setClipPath(renderPath);
            renderer.setMode(GeometryRenderer::fill | GeometryRenderer::stroke);
            renderer.setStroke(Color(0, 0, 0), 0.6);
            for (const auto& binDisk : disks) {
                renderer.setFill(choroplethP->m_colors[binDisk.bin]);
                renderer.setFillOpacity(200);
                auto c = binDisk.disk;
                if (c.has_value()) {
                    renderer.draw(approximate(*c).orthogonal_transform(transformation));
                }
            }
            renderer.setClipping(false);
        }, "Disks");

        ipeRenderer.addPainting([sampler, transformation](GeometryRenderer& renderer) {
            renderer.setMode(GeometryRenderer::stroke);
            renderer.setStroke(Color{0, 0, 0}, 0.6);

            auto&& polys = sampler->getLandmassPolys();
            for (const auto& poly : polys) {
                renderer.draw(transform(transformation, approximate(poly)));
            }
        }, "Outline");

        ipeRenderer.addPainting([choropleth, legendTL, choroplethP](GeometryRenderer& renderer) {
            auto intervals = choropleth->getIntervals();
            for (int bin = 0; bin < choropleth->numberOfBins(); ++bin) {
                auto& low = intervals[bin];
                auto& high = intervals[bin + 1];
                std::stringstream ss;
                ss << "\\textsf{" << low << "--" << high << "}" << suffix << std::endl;
                Point<Inexact> pos = legendTL - Vector<Inexact>(0, 10) * bin;
                Rectangle<Inexact> r(pos, pos + Vector<Inexact>(6.5, -6.5));
                renderer.setMode(GeometryRenderer::stroke | GeometryRenderer::fill);
                renderer.setStroke(Color{0, 0, 0}, 0.8);
                renderer.setFill(choroplethP->m_colors[bin]);
                renderer.draw(r);
                renderer.drawText(pos + Vector<Inexact>(10, 0), ss.str());
            }
        }, "Legend");

        ipeRenderer.addPainting([](GeometryRenderer& renderer) {
            renderer.drawText(, title);
        }, "Title");

        std::stringstream outputFileName;
        outputFileName << name << "_" << attribute << ".ipe";
        ipeRenderer.save(outputFileName.str());
    }
}