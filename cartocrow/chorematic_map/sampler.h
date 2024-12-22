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
void
voronoiMoveToCentroid(const RegionArrangement& domain, const Landmarks_pl<RegionArrangement>& pl, InputIterator begin, InputIterator end, OutputIterator out, const Rectangle<Exact>& bbox) {
	auto arr = voronoiRegionArrangement(domain, begin, end, bbox);

	std::map<Point<Exact>, std::vector<VoronoiRegionArrangement::Face_handle>> siteToFaces;
	for (auto fit = arr.faces_begin(); fit != arr.faces_end(); ++fit) {
		if (!fit->data().site.has_value() || fit->data().region.empty()) continue;
		siteToFaces[*(fit->data().site)].push_back(fit);
	}

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
			Polygon<Exact> pgn;
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
			} else {
				*out++ = site;
			}
		}
	}
}

template <class PL>
class Sampler {
  private:
	using RegionWeight = std::unordered_map<std::string, double>;

	std::shared_ptr<RegionArrangement> m_regionArr;
    bool m_samplePerRegion;
	int m_seed;

	// General ancillary data
	std::vector<std::string> m_regions;
	std::shared_ptr<PL> m_pl;

    // Ancillary data for uniform random sampling
	std::unique_ptr<CDT<Exact>> m_cdt;
	std::vector<Triangle<Exact>> m_triangles;
	std::vector<std::string> m_triangleToRegion;
    std::unordered_map<std::string, std::vector<Triangle<Exact>>> m_regionToTriangles;
    std::unordered_map<std::string, double> m_regionArea;

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

  private:
    void initializeTriangulation() {
        m_triangles.clear();

        // Initialize triangulation
        m_cdt = std::make_unique<CDT<Exact>>();
        for (auto fit = m_regionArr->faces_begin(); fit != m_regionArr->faces_end(); ++fit) {
            std::vector<RegionArrangement::Ccb_halfedge_const_circulator> ccbs;
            std::copy(fit->outer_ccbs_begin(), fit->outer_ccbs_end(), std::back_inserter(ccbs));
            std::copy(fit->inner_ccbs_begin(), fit->inner_ccbs_end(), std::back_inserter(ccbs));
            for (auto& ccb : ccbs) {
                std::vector<Point<Exact>> vertices;
                auto curr = ccb;
                do {
                    auto point = curr->source()->point();
                    vertices.push_back(point);
                } while (++curr != ccb);
                m_cdt->insert_constraint(vertices.begin(), vertices.end(), true);
            }
        }

        auto pl = getPL();

        for (auto t_fit = m_cdt->finite_faces_begin(); t_fit != m_cdt->finite_faces_end(); ++t_fit) {
            auto t = m_cdt->triangle(t_fit);
            auto c = centroid(t);
            auto obj = pl->locate(c);
            if (auto fhp = boost::get<RegionArrangement::Face_const_handle>(&obj)) {
                auto fh = *fhp;
                bool inDomain = !fh->is_unbounded() && !fh->data().empty();
                if (inDomain) {
                    m_triangles.push_back(t);
                    m_triangleToRegion.push_back(fh->data());
                    m_regionToTriangles[fh->data()].push_back(t);
//                    double weight = m_regionWeight->contains(fh->data()) ? m_regionWeight->at(fh->data()) : 0;
//                    m_triangleWeights.push_back(weight);
                    if (!m_regionArea.contains(fh->data())) {
                        m_regionArea[fh->data()] = 0;
                    }
                    m_regionArea[fh->data()] += abs(approximate(t).area());
                }
            }
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

  public:
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
        m_cdt = nullptr;

		// General ancillary data
		m_pl = nullptr;
		m_regions.clear();

        // Ancillary data for uniform random sampling
        m_triangles.clear();
        m_triangleToRegion.clear();
        m_regionToTriangles.clear();
        m_regionArea.clear();

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

    void setTriangulation(std::unique_ptr<CDT<Exact>> cdt) {
        m_cdt = std::move(cdt);
    }

    CDT<Exact>& getTriangulation() {
        if (!m_cdt) {
            initializeTriangulation();
        }
        return *m_cdt;
    }

    void setTriangles(std::vector<Triangle<Exact>> triangles) {
        m_triangles = std::move(triangles);
    }

    std::vector<Triangle<Exact>>& getTriangles() {
        if (m_triangles.empty()) {
            initializeTriangulation();
        }
        return m_triangles;
    }

    void setTriangleToRegion(std::vector<std::string> triangleToRegion) {
        m_triangleToRegion = std::move(triangleToRegion);
    }

    std::vector<std::string>& getTriangleToRegion() {
        if (m_triangleToRegion.empty()) {
            initializeTriangulation();
        }
        return m_triangleToRegion;
    }

    void setRegionToTriangles(std::unordered_map<std::string, std::vector<Triangle<Exact>>> regionToTriangles) {
        m_regionToTriangles = std::move(regionToTriangles);
    }

    void setRegionArea(std::unordered_map<std::string, double> regionArea) {
        m_regionArea = std::move(regionArea);
    }

    std::unordered_map<std::string, std::vector<Triangle<Exact>>>& getRegionToTriangles() {
        if (m_regionToTriangles.empty()) {
            initializeTriangulation();
        }
        return m_regionToTriangles;
    }

    std::unordered_map<std::string, double>& getRegionArea() {
        if (m_regionArea.empty()) {
            for (auto fit = m_regionArr->faces_begin(); fit != m_regionArr->faces_end(); ++fit) {
                auto region = fit->data();
                if (fit->is_unbounded() || region.empty()) continue;
                auto pwh = approximate(face_to_polygon_with_holes<Exact>(fit));
                if (!m_regionArea.contains(region)) {
                    m_regionArea[region] = 0;
                }
                m_regionArea[region] += abs(pwh.outer_boundary().area());
                for (auto& hole : pwh.holes()) {
                    m_regionArea[region] -= abs(hole.area());
                }
            }
        }
        return m_regionArea;
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
				std::cerr << "Region " << region << " has no weight" << std::endl;
				w = 0;
			}
			return WeightedPoint(approximate(pt), w);
		} else {
			std::cerr << "Point does not lie in face but on edge or vertex!" << std::endl;
			throw std::runtime_error("Unhandled degenerate case");
		}
	}

  private:
    template <class OutputIterator>
    void uniformRandomPoints(int n, OutputIterator out) {
        CGAL::Random rng(m_seed);
        CGAL::get_default_random() = CGAL::Random(m_seed);
        if (!m_samplePerRegion) {
            auto& triangles = getTriangles();
            auto generator = CGAL::Random_points_in_triangles_2<Point<Exact>>(triangles, rng);
            std::copy_n(generator, n, out);
        } else {
            auto& regionArea = getRegionArea();
            auto& regionToTriangles = getRegionToTriangles();

            double totalArea = 0;
            std::vector<std::pair<std::string, double>> cumulativeArea;
            for (auto& [region, area] : regionArea) {
                totalArea += area;
                cumulativeArea.emplace_back(region, totalArea);
            }
            double step = totalArea / (n + 1);

            int regionIndex = 0;
            double currArea = 0.5 * step;
            for (int i = 0; i < n; ++i) {
                while (currArea > cumulativeArea[regionIndex].second) {
                    ++regionIndex;
                }
                std::string& region = cumulativeArea[regionIndex].first;
                const std::vector<Triangle<Exact>>& regionTriangles = regionToTriangles.at(region);
                auto generator = CGAL::Random_points_in_triangles_2<Point<Exact>>(regionTriangles, rng);
                std::copy_n(generator, 1, out);
                currArea += step;
            }
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

        for (int i = 0; i < arrs.size(); ++i) {
            const auto& arr = *arrs[i];
            const auto& pl = *pls[i];
            const auto& bb = bbs[i];
            const auto& outerPoly = polys[i];

            std::vector<Point<Exact>> samplesInComponent;
            for (const auto &pt: points) {
                Point<Exact> exactPoint(pt.x(), pt.y());
                if (CGAL::oriented_side(exactPoint, outerPoly) != CGAL::NEGATIVE) {
                    samplesInComponent.push_back(exactPoint);
                }
            }
            if (samplesInComponent.empty()) continue;

            std::vector<Point<Exact>> newPoints;
            for (int j = 0; j < iters; ++j) {
                if (progress.has_value()) {
                    auto f = *progress;
                    f(j);
                }
                if (cancelled.has_value()) {
                    auto f = *cancelled;
                    if (f()) break;
                }
                std::vector<Point<Exact>> approximated;
                for (const auto &pt: samplesInComponent) {
                    auto apt = approximate(pt);
                    approximated.emplace_back(apt.x(), apt.y());
                }
                voronoiMoveToCentroid(arr, pl, approximated.begin(), approximated.end(),
                                      std::back_inserter(newPoints), bb);
                samplesInComponent.clear();
                samplesInComponent.resize(0);
                std::copy(newPoints.begin(), newPoints.end(), std::back_inserter(samplesInComponent));
                newPoints.clear();
                newPoints.resize(0);
            }
            std::copy(samplesInComponent.begin(), samplesInComponent.end(), std::back_inserter(finalPoints));
        }
		return {finalPoints.begin(), finalPoints.end(), m_assignWeight};
	}

	WeightedRegionSample<Exact> squareGrid(double cellSize) {
        std::vector<Rectangle<Exact>>&& bbs = m_samplePerRegion ? getRegionCCBbs() : std::vector({getArrBoundingBox()});
        std::vector<std::shared_ptr<PL>>&& pls = m_samplePerRegion ? getRegionCCPls() : std::vector({getPL()});

        std::vector<Point<Exact>> points;
        for (int k = 0; k < bbs.size(); ++k) {
            auto& bb = bbs[k];
            auto& pl = pls[k];

            auto bbA = approximate(bb);
            auto w = width(bbA);
            auto h = height(bbA);
            int stepsX = static_cast<int>(w / cellSize) + 1;
            int stepsY = static_cast<int>(h / cellSize) + 1;
            auto bl = get_corner(bb, Corner::BL);

            for (int i = 0; i < stepsX; ++i) {
                for (int j = 0; j < stepsY; ++j) {
                    Point<Exact> pt = bl + Vector<Exact>(cellSize / 2 + i * cellSize, cellSize / 2 + j * cellSize);
                    auto obj = pl->locate(pt);
                    if (auto fhp = boost::get<RegionArrangement::Face_const_handle>(&obj)) {
                        auto fh = *fhp;
                        if (!fh->data().empty()) {
                            points.push_back(pt);
                        }
                    }
                }
            }
        }
		return {points.begin(), points.end(), m_assignWeight};
	}

	WeightedRegionSample<Exact> hexGrid(double cellSize) {
        std::vector<Rectangle<Exact>>&& bbs = m_samplePerRegion ? getRegionCCBbs() : std::vector({getArrBoundingBox()});
        std::vector<std::shared_ptr<PL>>&& pls = m_samplePerRegion ? getRegionCCPls() : std::vector({getPL()});

        std::vector<Point<Exact>> points;
        for (int k = 0; k < bbs.size(); ++k) {
            auto& bb = bbs[k];
            auto& pl = pls[k];

            auto bbA = approximate(bb);
            auto w = width(bbA);
            auto h = height(bbA);
            int stepsX = static_cast<int>(w / cellSize) + 1;
            double cellSizeY = 0.8660254 * cellSize;
            int stepsY = static_cast<int>(h / cellSizeY) + 1;
            auto bl = get_corner(bb, Corner::BL);

            for (int i = 0; i < stepsX; ++i) {
                for (int j = 0; j < stepsY; ++j) {
                    double shift = (j % 2 == 1) ? 0 : 0.5;
                    Point<Exact> pt = bl + Vector<Exact>(cellSize / 2 + (i + shift) * cellSize, cellSizeY / 2 + j * cellSizeY);
                    auto obj = pl->locate(pt);
                    if (auto fhp = boost::get<RegionArrangement::Face_const_handle>(&obj)) {
                        auto fh = *fhp;
                        if (!fh->data().empty()) {
                            points.push_back(pt);
                        }
                    }
                }
            }
        }
		return {points.begin(), points.end(), m_assignWeight};
	}
};
}

#endif //CARTOCROW_SAMPLER_H
