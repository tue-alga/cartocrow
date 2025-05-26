#ifndef CARTOCROW_SAMPLER_H
#define CARTOCROW_SAMPLER_H

#include "../core/arrangement_helpers.h"
#include "../core/region_arrangement.h"
#include "../core/centroid.h"
#include "../core/rectangle_helpers.h"

#include "weighted_point.h"
#include "weighted_region_sample.h"

#include <CGAL/Arr_landmarks_point_location.h>
#include <CGAL/Boolean_set_operations_2/oriented_side.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Random.h>
#include <CGAL/centroid.h>
#include <CGAL/mark_domain_in_triangulation.h>
#include <CGAL/point_generators_2.h>

#include <utility>
#include <future>

namespace cartocrow::chorematic_map {

typedef CGAL::Triangulation_vertex_base_2<Exact>                      Vb;
typedef CGAL::Constrained_triangulation_face_base_2<Exact>            Fb;
typedef CGAL::Triangulation_data_structure_2<Vb,Fb>                   TDS;
typedef CGAL::No_constraint_intersection_requiring_constructions_tag  Itag;
template <class K> using CDT = CGAL::Constrained_Delaunay_triangulation_2<K, TDS, Itag>;
typedef CGAL::Delaunay_triangulation_2<Exact>  						  DT;

// The following Cropped_voronoi_from_delaunay struct is adapted from:
// https://github.com/CGAL/cgal/blob/master/Triangulation_2/examples/Triangulation_2/print_cropped_voronoi.cpp
// which falls under the CC0 license.

//A class to recover Voronoi diagram from stream.
//Rays, lines and segments are cropped to a rectangle
//so that only segments are stored
struct Cropped_voronoi_from_delaunay{
	std::list<Segment<Exact>> m_cropped_vd;
	Rectangle<Exact> m_bbox;

	Cropped_voronoi_from_delaunay(const Rectangle<Exact>& bbox):m_bbox(bbox){}

	template <class RSL>
	void crop_and_extract_segment(const RSL& rsl){
		CGAL::Object obj = CGAL::intersection(rsl,m_bbox);
		const Segment<Exact>* s=CGAL::object_cast<Segment<Exact>>(&obj);
		if (s) m_cropped_vd.push_back(*s);
	}

	void operator<<(const Ray<Exact>& ray)    { crop_and_extract_segment(ray); }
	void operator<<(const Line<Exact>& line)  { crop_and_extract_segment(line); }
	void operator<<(const Segment<Exact>& seg){ crop_and_extract_segment(seg); }
};

struct SiteRegionData {
	std::optional<Point<Exact>> site;
	std::string region;
};

struct UnionSiteRegion {
	SiteRegionData operator()(const Point<Exact>& site, const std::string& region) const {
		return {site, region};
	}
};

using VoronoiArrangement = CGAL::Arrangement_2<CGAL::Arr_segment_traits_2<Exact>,
                                               CGAL::Arr_face_extended_dcel<CGAL::Arr_segment_traits_2<Exact>, Point<Exact>>>;

using VoronoiRegionArrangement = CGAL::Arrangement_2<CGAL::Arr_segment_traits_2<Exact>,
                                               		 CGAL::Arr_face_extended_dcel<CGAL::Arr_segment_traits_2<Exact>, SiteRegionData>>;

using Overlay_traits =
    CGAL::Arr_face_overlay_traits<VoronoiArrangement, RegionArrangement,
                                  VoronoiRegionArrangement, UnionSiteRegion>;

template <class Arr> using Landmarks_pl = CGAL::Arr_landmarks_point_location<Arr>;

template <class InputIterator>
VoronoiArrangement boundedVoronoiArrangement(InputIterator begin, InputIterator end, const Rectangle<Exact>& bbox) {
	DT dt;
	dt.insert(begin, end);
 	Cropped_voronoi_from_delaunay vor(bbox);
	dt.draw_dual(vor);

 	VoronoiArrangement arr;
	std::vector<Segment<Exact>> bboxSides({{bbox.vertex(0), bbox.vertex(1)}, {bbox.vertex(1), bbox.vertex(2)}, {bbox.vertex(2), bbox.vertex(3)}, {bbox.vertex(3), bbox.vertex(0)}});
	CGAL::insert_non_intersecting_curves(arr, vor.m_cropped_vd.begin(), vor.m_cropped_vd.end());
	CGAL::insert(arr, bboxSides.begin(), bboxSides.end());
	Landmarks_pl<VoronoiArrangement> pl(arr);

	for (auto vit = dt.vertices_begin(); vit != dt.vertices_end(); ++vit) {
		auto obj = pl.locate(vit->point());
		if (auto fhp = boost::get<VoronoiArrangement::Face_const_handle>(&obj)) {
			auto& fh = *fhp;
			auto fhm = arr.non_const_handle(fh);
			fhm->set_data(vit->point());
		}
	}

	return arr;
}

template <class InputIterator>
VoronoiRegionArrangement
voronoiRegionArrangement(const RegionArrangement& domain, InputIterator begin, InputIterator end, const Rectangle<Exact>& bbox) {
	auto vor = boundedVoronoiArrangement(begin, end, bbox);
	// overlay domain and vor.
	VoronoiRegionArrangement arr;
	Overlay_traits ovt;
	CGAL::overlay(vor, domain, arr, ovt);

	std::vector<VoronoiRegionArrangement::Halfedge_handle> trash;
	for (auto eit = arr.edges_begin(); eit != arr.edges_end(); ++eit) {
		auto fh1 = eit->face();
		auto fh2 = eit->twin()->face();
		if (fh1->data().region.empty() && fh2->data().region.empty()) {
			trash.push_back(eit);
		}
	}
	for (auto eh : trash) {
		arr.remove_edge(eh);
	}

	return arr;
}

template <class InputIterator, class OutputIterator>
Number<Inexact>
voronoiMoveToCentroid(const RegionArrangement& domain, const Landmarks_pl<RegionArrangement>& pl, InputIterator begin, InputIterator end, OutputIterator out, const Rectangle<Exact>& bbox) {
	auto arr = voronoiRegionArrangement(domain, begin, end, bbox);

	std::map<Point<Exact>, std::vector<VoronoiRegionArrangement::Face_handle>> siteToFaces;
	for (auto fit = arr.faces_begin(); fit != arr.faces_end(); ++fit) {
		if (!fit->data().site.has_value() || fit->data().region.empty()) continue;
		siteToFaces[*(fit->data().site)].push_back(fit);
	}

	Number<Inexact> totalDistance = 0;
	for (auto [site, faces] : siteToFaces) {
		Vector<Exact> total = Vector<Exact>(0, 0);
		Number<Exact> totalWeight = 0;
		for (auto fh : faces) {
			auto poly = face_to_polygon_with_holes<Exact>(fh);
			auto c = centroid(poly);
			auto area = poly.outer_boundary().area();
			for (const auto& hole : poly.holes()) {
				area -= hole.area();
			}
			auto weight = area;
			total += weight * (c - CGAL::ORIGIN);
			totalWeight += weight;
		}
		auto centroid = CGAL::ORIGIN + total / totalWeight;
		auto obj = pl.locate(centroid);
		if (auto fhp = boost::get<RegionArrangement::Face_const_handle>(&obj)) {
			auto& fh = *fhp;
			if (!fh->data().empty()) {
				*out++ = centroid;
				totalDistance += sqrt(squared_distance(approximate(site), approximate(centroid)));
			} else {
				*out++ = site;
			}
		} else {
			std::cerr << "Centroid does not lie in a face" << std::endl;
		}
	}
	return totalDistance / siteToFaces.size();
}

class Sampler {
  private:
	using PL = Landmarks_pl<RegionArrangement>;
	using RegionWeight = std::unordered_map<std::string, double>;

	std::shared_ptr<RegionArrangement> m_regionArr;
    bool m_samplePerRegion;
	int m_seed;

	// General ancillary data
	std::vector<std::string> m_regions;
	std::shared_ptr<PL> m_pl;

    // Ancillary data for uniform random sampling
	std::vector<Triangle<Exact>> m_triangles;
    std::vector<std::vector<Triangle<Exact>>> m_regionCCToTriangles;
    std::vector<double> m_regionCCArea;

    // Ancillary data for centroidal Voronoi diagram sampling
    std::vector<std::shared_ptr<RegionArrangement>> m_landmassArrs;
    std::vector<std::shared_ptr<PL>> m_landmassPls;
    std::vector<Rectangle<Exact>> m_landmassBbs;
    std::vector<PolygonWithHoles<Exact>> m_landmassPolys;

    std::vector<std::shared_ptr<RegionArrangement>> m_regionCCArrs;
    std::vector<std::shared_ptr<PL>> m_regionCCPls;
    std::vector<Rectangle<Exact>> m_regionCCBbs;
    std::vector<PolygonWithHoles<Exact>> m_regionCCPolys;

    // Ancillary data for grid sampling
    std::optional<Rectangle<Exact>> m_bb;

  public:
    void initializeTriangulation() {
        m_triangles.clear();
		const std::vector<PolygonWithHoles<Exact>>& polys = getRegionCCPolys();

		for (const auto& poly : polys) {
			std::vector<Triangle<Exact>> triangles;
			CDT<Exact> cdt;
			cdt.insert_constraint(poly.outer_boundary().vertices_begin(), poly.outer_boundary().vertices_end());
			for (const auto& hole : poly.holes()) {
				cdt.insert_constraint(hole.vertices_begin(), hole.vertices_end());
			}

			for (auto t_fit = cdt.finite_faces_begin(); t_fit != cdt.finite_faces_end(); ++t_fit) {
				auto t = cdt.triangle(t_fit);
				auto c = centroid(t);
				if (oriented_side(c, poly) == CGAL::ON_POSITIVE_SIDE) {
					m_triangles.push_back(t);
					triangles.push_back(t);
				}
			}
			m_regionCCToTriangles.push_back(std::move(triangles));
		}
    }

    void computeLandmasses() {
        std::vector<Component<RegionArrangement>> comps;
        connectedComponents(*m_regionArr, std::back_inserter(comps), [](RegionArrangement::Face_handle fh) {
	    	return !fh->data().empty();
	    });
	    m_landmassArrs.clear();
	    m_landmassPls.clear();
	    m_landmassBbs.clear();
	    m_landmassPolys.clear();
	    for (const auto& comp : comps) {
	    	auto compArr = std::make_shared<RegionArrangement>(comp.arrangement());
	    	copyBoundedFaceData(*m_regionArr, *compArr);
	    	m_landmassArrs.push_back(compArr);
	    	m_landmassPls.push_back(std::make_shared<PL>(*(m_landmassArrs.back())));
	    	std::vector<Point<Exact>> points;
	    	for (auto vit = compArr->vertices_begin(); vit != compArr->vertices_end(); ++vit) {
	    		points.push_back(vit->point());
	    	}
	    	auto bb = CGAL::bbox_2(points.begin(), points.end());
	    	m_landmassBbs.emplace_back(bb);
	    	m_landmassPolys.push_back(comp.surface_polygon());
	    }
    }

    void computeRegionCCs() {
        std::vector<Component<RegionArrangement>> comps;
		std::vector<std::string>& regions = getRegions();
        for (auto& region : regions) {
            connectedComponents(*m_regionArr, std::back_inserter(comps), [region](RegionArrangement::Face_handle fh) {
                return fh->data() == region;
            });
        }
        m_regionCCArrs.clear();
        m_regionCCPls.clear();
        m_regionCCBbs.clear();
        m_regionCCPolys.clear();
        for (const auto& comp : comps) {
            auto compArr = std::make_shared<RegionArrangement>(comp.arrangement());
            copyBoundedFaceData(*m_regionArr, *compArr);
            m_regionCCArrs.push_back(compArr);
            m_regionCCPls.push_back(std::make_shared<PL>(*(m_regionCCArrs.back())));
            std::vector<Point<Exact>> points;
            for (auto vit = compArr->vertices_begin(); vit != compArr->vertices_end(); ++vit) {
                points.push_back(vit->point());
            }
            auto bb = CGAL::bbox_2(points.begin(), points.end());
            m_regionCCBbs.emplace_back(bb);
            m_regionCCPolys.push_back(comp.surface_polygon());
        }
    }

    // -----------------------------------
    // Getters and setters for input data.
    // -----------------------------------
	void setSeed(int seed) {
		m_seed = seed;
	}

    int getSeed() const {
        return m_seed;
    }

    void setRegionArr(std::shared_ptr<RegionArrangement> regionArr) {
        m_regionArr = std::move(regionArr);

		// General ancillary data
		m_pl = nullptr;
		m_regions.clear();

        // Ancillary data for uniform random sampling
        m_triangles.clear();
        m_regionCCToTriangles.clear();
		m_regionCCArea.clear();

        // Ancillary data for centroidal Voronoi diagram sampling
        m_landmassArrs.clear();
        m_landmassPls.clear();
        m_landmassBbs.clear();
        m_landmassPolys.clear();

        m_regionCCArrs.clear();
        m_regionCCPls.clear();
        m_regionCCBbs.clear();
        m_regionCCPolys.clear();

        // Ancillary data for grid sampling
        m_bb = std::nullopt;
    }

    std::shared_ptr<RegionArrangement> getRegionArr() const {
        return m_regionArr;
    }

    void setSamplePerRegion(bool samplePerRegion) {
        m_samplePerRegion = samplePerRegion;
    }

    bool getSamplePerRegion() const {
        return m_samplePerRegion;
    }

    // -----------------------------------------------------------------------
    // Getters and setters for auxiliary data.
    //
    // The getters perform lazy initialization and consequently are not const.
    // Setters are defined in case auxiliary data has already been computed
    // for other purposes.
    // -----------------------------------------------------------------------
    void setPL(std::shared_ptr<PL> arrangementPointLocation) {
        m_pl = std::move(arrangementPointLocation);
    }

    std::shared_ptr<PL> getPL() {
        if (!m_pl) {
            m_pl = std::make_shared<PL>(*m_regionArr);
        }
        return m_pl;
    }

	void setRegions(const std::vector<std::string>& regions) {
		m_regions = regions;
	}

	std::vector<std::string>& getRegions() {
		if (m_regions.empty()) {
			for (auto fit = m_regionArr->faces_begin(); fit != m_regionArr->faces_end(); ++fit) {
				if (!fit->data().empty())
					m_regions.push_back(fit->data());
			}
			std::sort(m_regions.begin(), m_regions.end());
			m_regions.erase(std::unique(m_regions.begin(), m_regions.end()), m_regions.end());
		}
		return m_regions;
	}

    // Triangulation for uniform random sampling

    void setTriangles(std::vector<Triangle<Exact>> triangles) {
        m_triangles = std::move(triangles);
    }

    std::vector<Triangle<Exact>>& getTriangles() {
        if (m_triangles.empty()) {
            initializeTriangulation();
        }
        return m_triangles;
    }

    void setRegionCCToTriangles(std::vector<std::vector<Triangle<Exact>>> regionCCToTriangles) {
        m_regionCCToTriangles = std::move(regionCCToTriangles);
    }

    void setRegionCCArea(std::vector<double> regionCCArea) {
		m_regionCCArea = std::move(regionCCArea);
    }

    std::vector<std::vector<Triangle<Exact>>>& getRegionCCToTriangles() {
        if (m_regionCCToTriangles.empty()) {
            initializeTriangulation();
        }
        return m_regionCCToTriangles;
    }

    std::vector<double>& getRegionCCArea() {
        if (m_regionCCArea.empty()) {
			const std::vector<PolygonWithHoles<Exact>>& polys = getRegionCCPolys();
			for (const auto& poly : polys) {
				double area = 0;
				area += abs(approximate(poly.outer_boundary()).area());
				for (const auto& h : poly.holes()) {
					area -= abs(approximate(h).area());
				}
				m_regionCCArea.push_back(area);
			}
        }
        return m_regionCCArea;
    }

    // Ancillary data for centroidal Voronoi diagram sampling
    std::vector<std::shared_ptr<RegionArrangement>>& getLandmassArrs() {
        if (m_landmassArrs.empty()) {
            computeLandmasses();
        }
        return m_landmassArrs;
    }

    std::vector<std::shared_ptr<PL>>& getLandmassPls() {
        if (m_landmassPls.empty()) {
            computeLandmasses();
        }
        return m_landmassPls;
    }

    std::vector<Rectangle<Exact>>& getLandmassBbs() {
        if (m_landmassBbs.empty()) {
            computeLandmasses();
        }
        return m_landmassBbs;
    }

    std::vector<PolygonWithHoles<Exact>>& getLandmassPolys() {
        if (m_landmassPolys.empty()) {
            computeLandmasses();
        }
        return m_landmassPolys;
    }

    std::vector<std::shared_ptr<RegionArrangement>>& getRegionCCArrs() {
        if (m_regionCCArrs.empty()) {
            computeRegionCCs();
        }
        return m_regionCCArrs;
    }

    std::vector<std::shared_ptr<PL>>& getRegionCCPls() {
        if (m_regionCCPls.empty()) {
            computeRegionCCs();
        }
        return m_regionCCPls;
    }

    std::vector<Rectangle<Exact>>& getRegionCCBbs() {
        if (m_regionCCBbs.empty()) {
            computeRegionCCs();
        }
        return m_regionCCBbs;
    }

    std::vector<PolygonWithHoles<Exact>>& getRegionCCPolys() {
        if (m_regionCCPolys.empty()) {
            computeRegionCCs();
        }
        return m_regionCCPolys;
    }

    // Ancillary data for grid sampling
    Rectangle<Exact> getArrBoundingBox() {
        if (!m_bb.has_value()) {
            if (!m_landmassBbs.empty()) {
                m_bb = CGAL::bbox_2(m_landmassBbs.begin(), m_landmassBbs.end());
            } else {
                std::vector<Point<Exact>> points;
                for (auto vit = m_regionArr->vertices_begin(); vit != m_regionArr->vertices_end(); ++vit) {
                    points.push_back(vit->point());
                }
                m_bb = CGAL::bbox_2(points.begin(), points.end());
            }
        }
        return *m_bb;
    }

	Sampler(std::shared_ptr<RegionArrangement> regionArr,
            int seed,
            bool samplePerRegion = false)
	  : m_regionArr(std::move(regionArr)), m_seed(seed), m_samplePerRegion(samplePerRegion) {}

	WeightedPoint assignWeightToPoint(const Point<Exact>& pt, const RegionWeight& regionWeight, bool unitWeight = false) {
        auto pl = getPL();
		auto obj = pl->locate(pt);
		if (auto fhp = boost::get<RegionArrangement::Face_const_handle>(&obj)) {
			auto fh = *fhp;
			double w;
			auto region = fh->data();
			if (regionWeight.contains(region)) {
				w = regionWeight.at(fh->data());
				if (unitWeight) {
					if (w > 0) {
						w = 1;
					} else {
						w = -1;
					}
				}
			} else {
				if (!region.empty()) {
					std::cerr << "Region " << region << " has no weight" << std::endl;
					w = 0;
				}
			}
			return WeightedPoint(approximate(pt), w);
		} else {
			std::cerr << "Point does not lie in face but on edge or vertex!" << std::endl;
			throw std::runtime_error("Unhandled degenerate case");
		}
	}

  private:
	std::vector<int> pointsPerRegion(int n) {
		auto& regionCCArea = getRegionCCArea();
		std::vector<int> regionN(regionCCArea.size());

		double totalArea = 0;
		std::vector<int> regionCCIndices(regionCCArea.size());
		for (int i = 0; i < regionCCArea.size(); ++i) {
			totalArea += regionCCArea[i];
			regionCCIndices[i] = i;
		}
		int left = n;
		for (int i = 0; i < regionCCArea.size(); ++i) {
			auto& area = regionCCArea[i];
			auto guaranteed = static_cast<int>(floor(area / totalArea * n));
			left -= guaranteed;
			regionN[i] = guaranteed;
		}
		if (left > 0) {
			std::sort(regionCCIndices.begin(), regionCCIndices.end(), [n, &regionCCArea, totalArea](int regionCC1, int regionCC2) {
				auto prop1 = regionCCArea.at(regionCC1) / totalArea * n;
				auto prop2 = regionCCArea.at(regionCC2) / totalArea * n;
				auto frac1 = prop1 - floor(prop1);
				auto frac2 = prop2 - floor(prop2);
				return frac1 > frac2;
			});
			for (int i = 0; i < left; ++i) {
				regionN[regionCCIndices[i]] += 1;
			}
		}

		return regionN;
	}

    template <class OutputIterator>
    void uniformRandomPoints(int n, OutputIterator out) {
        CGAL::Random rng(m_seed);
        CGAL::get_default_random() = CGAL::Random(m_seed);
        if (!m_samplePerRegion) {
            auto& triangles = getTriangles();
            auto generator = CGAL::Random_points_in_triangles_2<Point<Exact>>(triangles, rng);
			std::vector<Point<Exact>> points;
            std::copy_n(generator, n, std::back_inserter(points));

			int bad = 0;
			int i = 1;
			while (true) {
				for (auto& pt : points) {
					bool found = false;
					for (auto& pl : getLandmassPls()) {
						auto obj = pl->locate(pt);
						if (auto fhp = boost::get<RegionArrangement::Face_const_handle>(&obj)) {
							auto fh = *fhp;
							if (!fh->data().empty()) {
								found = true;
							}
						} else {
							std::cout << "Sample point does not lie on face " << pt << std::endl;
						}
					}
					if (!found) {
						++bad;
//						std::cerr << "Sample point in empty face " << pt << std::endl;
					} else {
						*out++ = pt;
					}
				}
				if (bad == 0) break;
				points.clear();
				CGAL::Random rngNew(m_seed + i);
				CGAL::get_default_random() = CGAL::Random(m_seed + i);
				auto generatorNew = CGAL::Random_points_in_triangles_2<Point<Exact>>(triangles, rngNew);
				std::copy_n(generatorNew, bad, std::back_inserter(points));
				bad = 0;
				++i;
			}

        } else {
            auto& regionCCToTriangles = getRegionCCToTriangles();
			auto regionCCns = pointsPerRegion(n);

			for (int i = 0; i < regionCCns.size(); ++i) {
				const std::vector<Triangle<Exact>>& regionCCTriangles = regionCCToTriangles.at(i);
				auto generator = CGAL::Random_points_in_triangles_2<Point<Exact>>(regionCCTriangles, rng);
				std::vector<Point<Exact>> points;
				std::copy_n(generator, regionCCns[i], std::back_inserter(points));
				auto& pl = getRegionCCPls()[i];

				int bad = 0;
				int iters = 1;
				while (true) {
					for (auto& pt : points) {
						bool found = false;
						auto obj = pl->locate(pt);
						if (auto fhp = boost::get<RegionArrangement::Face_const_handle>(&obj)) {
							auto fh = *fhp;
							if (!fh->data().empty()) {
								found = true;
							}
						} else {
							std::cout << "Sample point does not lie on face " << pt << std::endl;
						}
						if (!found) {
							++bad;
//							std::cerr << "Sample point in empty face " << pt << std::endl;
						} else {
							*out++ = pt;
						}
					}
					if (bad == 0) break;
					points.clear();
					CGAL::Random rngNew(m_seed + iters);
					CGAL::get_default_random() = CGAL::Random(m_seed + iters);
					auto generatorNew = CGAL::Random_points_in_triangles_2<Point<Exact>>(regionCCTriangles, rngNew);
					std::copy_n(generatorNew, bad, std::back_inserter(points));
					bad = 0;
					++iters;
				}
			}
        }
    }

	template <class OutputIterator>
	void squareGrid(OutputIterator out, double cellSize, const Rectangle<Exact>& bb, const std::shared_ptr<PL>& pl) {
		auto bbA = approximate(bb);
		auto w = width(bbA);
		auto h = height(bbA);
		int stepsX = static_cast<int>(std::ceil(w / cellSize));
		int stepsY = static_cast<int>(std::ceil(h / cellSize));
		double xRem = cellSize * stepsX - w;
		double yRem = cellSize * stepsY - h;
//		double xOff = -xRem / 2;
//		double yOff = -yRem / 2;
		double xOff = 0;
		double yOff = 0;
		auto bl = get_corner(bb, Corner::BL);

		for (int i = 0; i < stepsX; ++i) {
			for (int j = 0; j < stepsY; ++j) {
				Point<Exact> pt = bl + Vector<Exact>(xOff + cellSize / 2 + i * cellSize, yOff + cellSize / 2 + j * cellSize);
				auto obj = pl->locate(pt);
				if (auto fhp = boost::get<RegionArrangement::Face_const_handle>(&obj)) {
					auto fh = *fhp;
					if (!fh->data().empty()) {
						*out++ = pt;
					}
				} else {
					std::cerr << "Square grid sample point does not lie in a face" << std::endl;
				}
			}
		}
	}

	template <class OutputIterator>
	double squareGrid(OutputIterator out, int n, const Rectangle<Exact>& bb, const std::shared_ptr<PL>& pl, int maxIters = 50) {
		auto bbA = approximate(bb);
		auto w = width(bbA);
		auto h = height(bbA);
		// n <= stepsX * stepsY <= (w / cellSize + 1) * (h / cellSize + 1) =~ wh / cellSize²
		// cellSize <= ~= sqrt(wh / n)
		double lower = sqrt(w*h / n) / 4;
		double upper = sqrt(w*h / n) * 4;

		int iters = 0;
		std::vector<Point<Exact>> pts;
		while (iters < maxIters && lower < upper) {
			double mid = (lower + upper) / 2;
			pts.clear();
			pts.resize(0);
			squareGrid(std::back_inserter(pts), mid, bb, pl);
			auto size = pts.size();
			if (size < n) {
				upper = mid;
			} else if (size > n) {
				lower = mid;
			} else {
				std::copy(pts.begin(), pts.end(), out);
				return mid;
			}
			++iters;
		}
		std::vector<Point<Exact>> one;
		squareGrid(std::back_inserter(one), lower, bb, pl);
		std::vector<Point<Exact>> other;
		squareGrid(std::back_inserter(other), upper, bb, pl);
		if (one.size() != n && other.size() != n) {
			std::cerr << "Did not find square grid with " << n << " sample points." << std::endl;
		}
		if (abs(n - static_cast<int>(one.size())) < abs(n - static_cast<int>(other.size()))) {
			std::copy(one.begin(), one.end(), out);
			return lower;
		} else {
			std::copy(other.begin(), other.end(), out);
			return upper;
		}
	}

	template <class OutputIterator>
	void hexGrid(OutputIterator out, double cellSize, const Rectangle<Exact>& bb, const std::shared_ptr<PL>& pl) {
		auto bbA = approximate(bb);
		auto w = width(bbA);
		auto h = height(bbA);
		int stepsX = static_cast<int>(std::ceil(w / cellSize));
		double cellSizeY = 0.8660254 * cellSize;
		int stepsY = static_cast<int>(std::ceil(h / cellSizeY));
		auto bl = get_corner(bb, Corner::BL);

		double xRem = cellSize * stepsX - w;
		double yRem = cellSizeY * stepsY - h;
//		double xOff = -xRem / 2;
//		double yOff = -yRem / 2;
		double xOff = 0;
		double yOff = 0;

		for (int j = 0; j < stepsY; ++j) {
			bool odd = j % 2 == 1;
			for (int i = 0; i < (odd ? stepsX + 1 : stepsX); ++i) {
				double shift = odd ? 0 : -0.5;
				Point<Exact> pt = bl + Vector<Exact>(xOff + cellSize / 2 + (i + shift) * cellSize, yOff + cellSizeY / 2 + j * cellSizeY);
				auto obj = pl->locate(pt);
				if (auto fhp = boost::get<RegionArrangement::Face_const_handle>(&obj)) {
					auto fh = *fhp;
					if (!fh->data().empty()) {
						*out++ = pt;
					}
				} else {
					std::cerr << "Hex. grid sample point does not lie in a face" << std::endl;
				}
			}
		}
	}

	template <class OutputIterator>
	double hexGrid(OutputIterator out, int n, const Rectangle<Exact>& bb, const std::shared_ptr<PL>& pl, int maxIters = 50) {
		auto bbA = approximate(bb);
		auto w = width(bbA);
		auto h = height(bbA);
		// n <= stepsX * stepsY <= (w / cellSize + 1) * (h / (cellSize * 0.8660254 + 1) =~ wh / (0.866 * cellSize²)
		// cellSize <= ~= sqrt(wh / (0.866n))
		double lower = sqrt(w * h / (0.866 * n)) / 4;
		double upper = sqrt(w * h / (0.866 * n)) * 4;

		int iters = 0;
		std::vector<Point<Exact>> pts;
		while (iters < maxIters && lower < upper) {
			double mid = (lower + upper) / 2;
			pts.clear();
			pts.resize(0);
			hexGrid(std::back_inserter(pts), mid, bb, pl);
			auto size = pts.size();
			if (size < n) {
				upper = mid;
			} else if (size > n) {
				lower = mid;
			} else {
				std::copy(pts.begin(), pts.end(), out);
				return mid;
			}
			++iters;
		}
		std::vector<Point<Exact>> one;
		hexGrid(std::back_inserter(one), lower, bb, pl);
		std::vector<Point<Exact>> other;
		hexGrid(std::back_inserter(other), upper, bb, pl);
		if (one.size() != n && other.size() != n) {
			std::cerr << "Did not find hexagonal grid with " << n << " sample points." << std::endl;
			std::cerr << "Bounding box: " << bb.xmin() << " " << bb.ymin() << " " << bb.xmax() << " " << bb.ymax() << std::endl;
		}
		if (abs(n - static_cast<int>(one.size())) < abs(n - static_cast<int>(other.size()))) {
			std::copy(one.begin(), one.end(), out);
			return lower;
		} else {
			std::copy(other.begin(), other.end(), out);
			return upper;
		}
	}

  public:
	std::function<WeightedPoint(const Point<Exact>&, const RegionWeight&)> m_assignWeight = [this](const Point<Exact>& point, const RegionWeight& regionWeights) {
		return assignWeightToPoint(point, regionWeights);
	};

	/// Generate samples uniformly at random over the arrangement.
	/// The weight of a sample point is equal to the weight of the region it lies in.
	WeightedRegionSample<Exact> uniformRandomSamples(int n) {
		WeightedRegionSample<Exact> sample;
        uniformRandomPoints(n, std::back_inserter(sample.m_points));
		sample.setAssignWeightFunction(m_assignWeight);
		return sample;
	}

	/// Generate samples uniformly at random over the arrangement.
	/// The weight of a sample point is equal to the weight of the region it lies in.
	/// Perturb via Voronoi.
	WeightedRegionSample<Exact>
	voronoiUniform(int n,
	               int iters,
	               std::optional<std::function<void(int)>> progress = std::nullopt,
	               std::optional<std::function<bool()>> cancelled = std::nullopt) {
		std::vector<Point<Exact>> points;
        uniformRandomPoints(n, std::back_inserter(points));

		std::vector<Point<Exact>> finalPoints;

        auto& arrs = m_samplePerRegion ? getRegionCCArrs() : getLandmassArrs();
        auto& pls = m_samplePerRegion ? getRegionCCPls() : getLandmassPls();
        auto& bbs = m_samplePerRegion ? getRegionCCBbs() : getLandmassBbs();
        auto& polys = m_samplePerRegion ? getRegionCCPolys() : getLandmassPolys();

		auto task = [&points, &iters, &arrs, &pls, &bbs, &polys](int iStart, int iEnd) {
			std::vector<Point<Exact>> outputPoints;
			for (int i = iStart; i < iEnd; ++i) {
				const RegionArrangement& arr = *arrs[i];
				const auto& pl = *pls[i];
				const auto& bb = bbs[i];
				const PolygonWithHoles<Exact>& poly = polys[i];
				std::vector<Point<Exact>> samplesInComponent;
				for (const auto& pt : points) {
					if (oriented_side(pt, poly) == CGAL::ON_POSITIVE_SIDE) {
						samplesInComponent.push_back(pt);
					}
//					auto obj = pl.locate(pt);
//					if (auto fhp = boost::get<RegionArrangement::Face_const_handle>(&obj)) {
//						auto fh = *fhp;
//						if (!fh->data().empty()) {
//							samplesInComponent.push_back(pt);
//						}
//					} else {
//						std::cerr << "Voronoi sample point does not lie in a face" << std::endl;
//					}
				}
				if (samplesInComponent.empty())
					continue;
//				if (samplesInComponent.size() == 1 &&
//					arr.number_of_faces() - arr.number_of_unbounded_faces() == 1) {
//					auto centr = centroid(outerPoly);
//					if (oriented_side(centr, outerPoly) == CGAL::ON_POSITIVE_SIDE) {
//						outputPoints.push_back(centr);
//						continue;
//					}
//				}

				int originalNumber = samplesInComponent.size();
				std::vector<Point<Exact>> newPoints;
				for (int j = 0; j < iters; ++j) {
					//				if (progress.has_value()) {
					//					auto f = *progress;
					//					f(j);
					//				}
					//				if (cancelled.has_value()) {
					//					auto f = *cancelled;
					//					if (f()) break;
					//				}
					std::vector<Point<Exact>> approximated;
					for (const auto& pt : samplesInComponent) {
						auto apt = approximate(pt);
						approximated.emplace_back(apt.x(), apt.y());
					}
					voronoiMoveToCentroid(arr, pl, approximated.begin(), approximated.end(),
										  std::back_inserter(newPoints), bb);
					samplesInComponent.clear();
					samplesInComponent.resize(0);
					std::copy(newPoints.begin(), newPoints.end(),
							  std::back_inserter(samplesInComponent));
					newPoints.clear();
					newPoints.resize(0);
				}
				std::copy(samplesInComponent.begin(), samplesInComponent.end(), std::back_inserter(outputPoints));
			}
			return outputPoints;
		};

		int nThreads = 32;
		int nArrs = arrs.size();
		std::vector<std::future<std::vector<Point<Exact>>>> results;
		double step = nArrs / static_cast<double>(nThreads);
		for (int i = 0; i < nArrs / step; ++i) {
			int iStart = std::ceil(i * step);
			int iEnd = std::ceil((i + 1) * step);
			results.push_back(std::async(std::launch::async, task, iStart, iEnd));
		}
		for (auto& futureResult : results) {
			auto pts = futureResult.get();
			std::copy(pts.begin(), pts.end(), std::back_inserter(finalPoints));
		}
		return {finalPoints.begin(), finalPoints.end(), m_assignWeight};
	}

	WeightedRegionSample<Exact> squareGrid(int n, int maxIters = 50) {
		std::vector<Point<Exact>> points;
		if (m_samplePerRegion) {
			const std::vector<Rectangle<Exact>>& bbs = getRegionCCBbs();
			const std::vector<std::shared_ptr<PL>>& pls = getRegionCCPls();

			auto regionNMap = pointsPerRegion(n);
			for (int regionCCIndex = 0; regionCCIndex < bbs.size(); ++regionCCIndex) {
				const auto& bb = bbs.at(regionCCIndex);
				const auto& pl = pls.at(regionCCIndex);
				auto regionN = regionNMap.at(regionCCIndex);
				if (regionN > 0) {
					squareGrid(std::back_inserter(points), regionN, bb, pl, maxIters);
				}
			}
		} else {
			squareGrid(std::back_inserter(points), n, getArrBoundingBox(), getPL(), maxIters);
		}
		return {points.begin(), points.end(), m_assignWeight};
	}

	WeightedRegionSample<Exact> hexGrid(int n, int maxIters = 50) {
		std::vector<Point<Exact>> points;
		if (m_samplePerRegion) {
			const std::vector<Rectangle<Exact>>& bbs = getRegionCCBbs();
			const std::vector<std::shared_ptr<PL>>& pls = getRegionCCPls();

			auto regionNMap = pointsPerRegion(n);
			const auto& regions = getRegions();
			for (int regionCCIndex = 0; regionCCIndex < bbs.size(); ++regionCCIndex) {
				const auto& bb = bbs.at(regionCCIndex);
				const auto& pl = pls.at(regionCCIndex);
				auto regionN = regionNMap.at(regionCCIndex);
				if (regionN > 0) {
					hexGrid(std::back_inserter(points), regionN, bb, pl, maxIters);
				}
			}
		} else {
			hexGrid(std::back_inserter(points), n, getArrBoundingBox(), getPL(), maxIters);
		}
		return {points.begin(), points.end(), m_assignWeight};
	}
};
}

#endif //CARTOCROW_SAMPLER_H
