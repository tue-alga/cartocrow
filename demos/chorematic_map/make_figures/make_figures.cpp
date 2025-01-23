#include "make_figures.h"

#include "cartocrow/chorematic_map/input_parsing.h"
#include "cartocrow/chorematic_map/choropleth.h"
#include "cartocrow/chorematic_map/sampler.h"
#include "cartocrow/chorematic_map/choropleth_disks.h"
#include "cartocrow/core/arrangement_helpers.h"
#include "cartocrow/core/region_arrangement.h"
#include "cartocrow/core/ipe_reader.h"
#include "cartocrow/renderer/ipe_renderer.h"

#include <CGAL/Arr_landmarks_point_location.h>

using namespace cartocrow;
using namespace cartocrow::chorematic_map;
using namespace cartocrow::renderer;

using LandmarksPl = CGAL::Arr_landmarks_point_location<RegionArrangement>;

RenderPath& operator<<(RenderPath& path, const Polygon<Inexact>& p) {
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

RenderPath convertShapeToRenderPath(const ipe::Shape& shape, const ipe::Matrix& matrix) {
	RenderPath renderPath;
	for (int i = 0; i < shape.countSubPaths(); ++i) {
		if (shape.subPath(i)->type() != ipe::SubPath::ECurve) {
			throw std::runtime_error("Encountered closed ellipse or B-spline; unimplemented");
		}
		const ipe::Curve* curve = shape.subPath(i)->asCurve();
		for (int j = 0; j < curve->countSegments(); ++j) {
			ipe::CurveSegment segment = curve->segment(j);
			Point<Inexact> last;
			if (segment.type() == ipe::CurveSegment::ESegment || segment.type() == ipe::CurveSegment::EArc) {
				if (j == 0) {
					ipe::Vector v = matrix * segment.cp(0);
					Point<Inexact> pt(v.x, v.y);
					last = pt;
					renderPath.moveTo(pt);
				}
				ipe::Vector v = matrix * segment.last();
				Point<Inexact> pt(v.x, v.y);
				if (pt != last) {
					last = pt;
					if (segment.type() == ipe::CurveSegment::ESegment) {
						renderPath.lineTo(pt);
					} else {
						auto m = segment.matrix();
						auto clockwise = m.a[3] < 0;
						auto center = matrix * ipe::Vector(m.a[4], m.a[5]);
						renderPath.arcTo({center.x, center.y}, clockwise, pt);
					}
				}
			}
		}

		renderPath.close();
	}
	return renderPath;
}

renderer::RenderPath transform(const CGAL::Aff_transformation_2<Inexact>& t, const RenderPath& p) {
	RenderPath tp;
	for (auto& cmd : p.commands()) {
		if (auto* c = std::get_if<RenderPath::MoveTo>(&cmd)) {
			tp.moveTo(c->m_to.transform(t));
		} else if (auto* c = std::get_if<RenderPath::LineTo>(&cmd)) {
			tp.lineTo(c->m_to.transform(t));
		} else if (auto* c = std::get_if<RenderPath::ArcTo>(&cmd)) {
			tp.arcTo(c->m_center.transform(t), c->m_clockwise, c->m_to.transform(t)); // todo check clockwise
		} else if (auto* c = std::get_if<RenderPath::Close>(&cmd)) {
			tp.close();
		} else {
			throw std::runtime_error("Unknown render path command");
		}
	}
	return tp;
}

RenderPath loadIpePath(const std::filesystem::path& ipeFile) {
	std::shared_ptr<ipe::Document> document = IpeReader::loadIpeFile(ipeFile);

	if (document->countPages() == 0) {
		throw std::runtime_error("Cannot read map from an Ipe file with no pages");
	} else if (document->countPages() > 1) {
		throw std::runtime_error("Cannot read map from an Ipe file with more than one page");
	}

	ipe::Page* page = document->page(0);

	for (int i = 0; i < page->count(); ++i) {
		ipe::Object* object = page->object(i);
		ipe::Object::Type type = object->type();
		if (type != ipe::Object::Type::EPath) {
			continue;
		}
		ipe::Path* path = object->asPath();
		ipe::Matrix matrix = path->matrix();
		ipe::Shape ipeShape = path->shape();
		// interpret filled paths as regions
		auto renderPath = convertShapeToRenderPath(ipeShape, matrix);
		return renderPath;
	}

	throw std::runtime_error("Could not find a path in the ipe file");
}

CGAL::Aff_transformation_2<Inexact> fitInto(const Rectangle<Inexact>& toFit, const Rectangle<Inexact>& into) {
	CGAL::Aff_transformation_2<Inexact> move1(CGAL::TRANSLATION, CGAL::ORIGIN - centroid(toFit));
	CGAL::Aff_transformation_2<Inexact> move2(CGAL::TRANSLATION, centroid(into) - CGAL::ORIGIN);
	CGAL::Aff_transformation_2<Inexact> scale(CGAL::SCALING, std::min(width(into) / width(toFit), height(into) / height(toFit)));
	return move2 * scale * move1;
//	return CGAL::IDENTITY;
}

int main() {
    std::string name = "dutch";
    std::filesystem::path dataPath = "data/chorematic_map/wijkenbuurten_2022_v3.gpkg";
    std::filesystem::path mapPath = "data/chorematic_map/gemeenten-2022_5000vtcs.ipe";
	std::filesystem::path schematizationPath = "data/chorematic_map/netherlands-schematization.ipe";

		std::vector<std::tuple<std::string, std::string, std::string, double>> names = {
	        {"apotheek_gemiddelde_afstand_in_km", "Average distance to pharmacy", " km", 1},
		    {"brandweerkazerne_gemiddelde_afstand_in_km", "Average distance to fire station", " km", 1},
			{"kunstijsbaan_gemiddelde_afstand_in_km", "Average distance to ice-rink", " km", 1},
			{"percentage_huishoudens_met_hoog_inkomen", "Percentage of high-income households", "\\%", 1},
			{"percentage_werknemers", "Percentage employee", "\\%", 1},
			{"percentage_huurwoningen", "Percentage rental properties", "\\%", 1},
	    };

//	 Apply an orthogonal transformation (so no stretching) to position the diagram.
	CGAL::Aff_transformation_2<Inexact> trans(CGAL::SCALING, 0.06);
	CGAL::Aff_transformation_2<Inexact> sScale(CGAL::SCALING, 0.4);
	CGAL::Aff_transformation_2<Inexact> sMove(CGAL::TRANSLATION, Vector<Inexact>(18.5, 129.0));
	CGAL::Aff_transformation_2<Inexact> sTrans = sMove * sScale;

	auto regionMap = std::make_shared<RegionMap>(ipeToRegionMap(mapPath, true));
	auto regionWeightMap = regionDataMapFromGPKG(dataPath, "gemeenten", "gemeentecode", [](const std::string& s) {
	  return s;
	});

	std::shared_ptr<RegionArrangement> regionArr = std::make_shared<RegionArrangement>(regionMapToArrangementParallel(*regionMap));
	Point<Inexact> legendTL(70.0, 80.0);

//	std::string name = "hessen";
//	std::filesystem::path dataPath = "data/chorematic_map/hessen.gpkg";
//	std::filesystem::path mapPath = "data/chorematic_map/hessen.gpkg";
//	std::filesystem::path schematizationPath = "data/chorematic_map/hessen-schematized.ipe";
//
//	auto regionMap = regionMapFromGPKG(mapPath, "Hessen", "GEN");
//	auto regionWeightMap = regionDataMapFromGPKG(dataPath, "Hessen", "GEN", [](const std::string& s) {
//	  return s;
//	});
//
//	std::vector<std::tuple<std::string, std::string, std::string, double>> names = {
//		{"mun_stats_water_normalized", "Water", "\\%", 100},
//		{"mun_stats_vegetation_normalized", "Vegetation", "\\%", 100},
//		{"mun_stats_vege_forest_normalized", "Forest", "\\%", 100},
//		{"mun_stats_veg_agrar_nomalized", "Agricultural land", "\\%", 100},
//		{"mun_stats_sie_wohnbau_normalized", "Residential area", "\\%", 100},
//		{"mun_stats_total_income_1k_normalized", "Total income", "", 1},
//	};
//
//	std::shared_ptr<RegionArrangement> regionArr = std::make_shared<RegionArrangement>(regionMapToArrangementParallel(*regionMap));
//
//	Rectangle<Inexact> bb = bboxInexact(*regionArr);
//	Rectangle<Inexact> into(52, 56, 211, 244);
//	auto trans = fitInto(bb, into);
//	CGAL::Aff_transformation_2<Inexact> sScale(CGAL::SCALING, 0.4);
//	CGAL::Aff_transformation_2<Inexact> sMove(CGAL::TRANSLATION, Vector<Inexact>(21.5, 141.0));
//	CGAL::Aff_transformation_2<Inexact> sTrans = sMove * sScale;
//
//	Point<Inexact> legendTL(145.0, 105.0);

	bool sansSerif = false;
    int numberOfBins = 2;
    int seed = 0;
    bool applyHeuristic = false;
    int nSamples = 1000;
    bool perRegion = true;
	Rectangle<Inexact> bgRect(34.5774, 32.1618, 216.697, 251.988);
	Color offBlack(68, 68, 68);
	Color offWhite(230, 230, 230);
	std::optional<Color> bgStroke = offWhite;
	std::optional<Color> bgFill = std::nullopt;
	Point<Inexact> scorePos(38.69, 247.132);

	RenderPath schematization = transform(sTrans * trans, loadIpePath(schematizationPath));

    auto sampler = std::make_shared<Sampler<LandmarksPl>>(regionArr, seed, perRegion);

    for (auto& [attribute, title, suffix, valueScalar] : names) {
        std::shared_ptr<RegionWeight> regionData = std::make_shared<RegionWeight>(regionWeightMap->at(attribute));
        auto choropleth = std::make_shared<Choropleth>(regionArr, regionData, 2);

        std::vector<Color> colors;
        switch(numberOfBins) {
            case 2: {
			    // I have picked some from different classes the colorbrewer Greens scheme
				// https://colorbrewer2.org/#type=sequential&scheme=Greens
                colors = std::vector({Color(160, 217, 155), Color(48, 162, 83)});
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

        auto sample = sampler->voronoiUniform(nSamples, 5);
        auto disks0 = fitDisks(*choropleth, sample, false, true, applyHeuristic);
        auto disks1 = fitDisks(*choropleth, sample, true, true, applyHeuristic);
        double score0 = 0;
        for (const auto& disk : disks0) {
            score0 += *(disk.score);
        }
        double score1 = 0;
        for (const auto& disk : disks1) {
            score1 += *(disk.score);
        }

		std::vector<std::vector<BinDisk>> diskSets({disks0, disks1});
		std::vector<double> scores({score0, score1});

		for (int i = 0; i < 2; ++i) {
			auto& disks = diskSets[i];
			auto& score = scores[i];

			IpeRenderer ipeRenderer;
			ipeRenderer.addPainting([&](renderer::GeometryRenderer& renderer) {
				if (bgFill.has_value() && bgStroke.has_value()) {
					renderer.setMode(GeometryRenderer::fill | GeometryRenderer::stroke);
					renderer.setFill(*bgFill);
					renderer.setStroke(*bgStroke, 0.4);
				} else if (bgFill.has_value()) {
					renderer.setMode(GeometryRenderer::fill);
					renderer.setFill(*bgFill);
				} else if (bgStroke.has_value()) {
					renderer.setMode(GeometryRenderer::stroke);
					renderer.setStroke(*bgStroke, 0.4);
				} else {
					return;
				}
				renderer.draw(bgRect);
			}, "Background");

			ChoroplethPainting::Options options;
			options.transformation = trans;
			options.strokeWidth = 0.2;
			options.strokeColor = offWhite;
			auto choroplethP = std::make_shared<ChoroplethPainting>(*choropleth, colors.begin(), colors.end(), options);
			ipeRenderer.addPainting(choroplethP, "Choropleth");

			ipeRenderer.addPainting(
			    [&](renderer::GeometryRenderer& renderer) {
				    renderer.setMode(GeometryRenderer::fill);
				    auto bgBin = disks[0].bin == 0 ? 1 : 0;
				    renderer.setFill(choroplethP->m_colors[bgBin]);
				    renderer.draw(schematization);
			    },
			    "Schematization_fill");

			ipeRenderer.addPainting(
			    [&](renderer::GeometryRenderer& renderer) {
				    renderer.setClipping(true);
				    renderer.setClipPath(schematization);
				    renderer.setMode(GeometryRenderer::fill | GeometryRenderer::stroke);
				    renderer.setStroke(offWhite, 0.6);
				    for (const auto& binDisk : disks) {
					    renderer.setFill(choroplethP->m_colors[binDisk.bin]);
					    auto c = binDisk.disk;
					    if (c.has_value() && c->is_circle()) {
						    renderer.draw(
						        approximate(c->get_circle()).orthogonal_transform(sTrans * trans));
					    } else {
						    throw std::runtime_error("Todo: handle halfplane");
					    }
				    }
				    renderer.setClipping(false);
			    },
			    "Disks");

			ipeRenderer.addPainting(
			    [&](renderer::GeometryRenderer& renderer) {
				    renderer.setMode(GeometryRenderer::stroke);
				    renderer.setStroke(offBlack, 0.6);
				    renderer.draw(schematization);
			    },
			    "Schematization_stroke");

			ipeRenderer.addPainting(
			    [&](GeometryRenderer& renderer) {
				    renderer.setMode(GeometryRenderer::stroke);
				    renderer.setLineCap(GeometryRenderer::RoundCap);
				    renderer.setStroke(offBlack, 0.6);

				    auto&& polys = sampler->getLandmassPolys();
				    for (const auto& poly : polys) {
					    renderer.draw(transform(trans, approximate(poly)));
				    }
			    },
			    "Outline");

			ipeRenderer.addPainting(
			    [&](GeometryRenderer& renderer) {
				    auto intervals = choropleth->getIntervals();
				    for (int bin = 0; bin < choropleth->numberOfBins(); ++bin) {
					    auto& low = intervals[bin];
					    auto& high = intervals[bin + 1];
					    std::stringstream ss;
					    ss << std::fixed << std::setprecision(1);
					    ss << (sansSerif ? "\\textsf{" : "") << (valueScalar * low) << "--"
					       << (valueScalar * high) << suffix << (sansSerif ? "}" : "") << std::endl;
					    Point<Inexact> pos = legendTL - Vector<Inexact>(0, 10) * bin;
					    Rectangle<Inexact> r(pos, pos + Vector<Inexact>(6.5, -6.5));
					    renderer.setMode(GeometryRenderer::stroke | GeometryRenderer::fill);
					    renderer.setStroke(offBlack, 0.8);
					    renderer.setFill(choroplethP->m_colors[bin]);
					    renderer.setLineCap(GeometryRenderer::ButtCap);
					    renderer.setLineJoin(GeometryRenderer::MiterJoin);
					    renderer.setHorizontalTextAlignment(GeometryRenderer::AlignLeft);
					    renderer.setVerticalTextAlignment(GeometryRenderer::AlignVCenter);
					    renderer.draw(r);
					    renderer.drawText(pos + Vector<Inexact>(12, -3.25), ss.str());
				    }
			    },
			    "Legend");

			ipeRenderer.addPainting(
			    [&](GeometryRenderer& renderer) {
				    renderer.setHorizontalTextAlignment(GeometryRenderer::AlignHCenter);
				    renderer.setVerticalTextAlignment(GeometryRenderer::AlignBaseline);
				    auto bb = approximate(sampler->getArrBoundingBox()).transform(trans);
				    auto bottomBB = midpoint(get_side(bb, Bottom));
				    auto bottomBG = midpoint(get_side(bgRect, Bottom));
				    std::stringstream ss;
				    ss << (sansSerif ? "\\textsf{" : "") << title << (sansSerif ? "}" : "");
				    renderer.setStroke(offBlack, 0.8);
				    renderer.drawText({bottomBG.x(), (bottomBG.y() * 2 + bottomBB.y()) / 3},
				                      ss.str());
			    },
			    "Title");

			ipeRenderer.addPainting(
			    [&](GeometryRenderer& renderer) {
				    renderer.setHorizontalTextAlignment(GeometryRenderer::AlignLeft);
				    renderer.setVerticalTextAlignment(GeometryRenderer::AlignTop);
				    std::stringstream ss;
				    //		  std::cout << score << " " << totalArea << " " << (score / totalArea) << std::endl;
				    ss << std::setprecision(2) << score;
				    renderer.setStroke(offBlack, 0.8);
				    renderer.drawText(scorePos, ss.str());
			    },
			    "Score");

			ipeRenderer.setPreamble("\\usepackage{times}");

			// Somehow saving to pdf does not fully work
			std::stringstream outputFileName;
			outputFileName << name << "_" << attribute << "_" << i << ".pdf";
			ipeRenderer.save(outputFileName.str());
		}
    }
}