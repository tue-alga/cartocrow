#ifndef CARTOCROW_SAMPLING_H
#define CARTOCROW_SAMPLING_H

#include "../core/arrangement_helpers.h"
#include "../core/region_arrangement.h"

#include "weighted_point.h"

#include <CGAL/Arr_landmarks_point_location.h>
#include <CGAL/Boolean_set_operations_2/oriented_side.h>
#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Delaunay_triangulation_2.h>
#include <CGAL/Random.h>
#include <CGAL/centroid.h>
#include <CGAL/mark_domain_in_triangulation.h>
#include <CGAL/point_generators_2.h>

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

template <class K>
std::pair<Point<K>, Number<K>> centroidPWH(const PolygonWithHoles<K>& pgn) {
	const Polygon<K>& outer = pgn.outer_boundary();

	CDT<K> cdt;
	cdt.insert_constraint(outer.vertices_begin(), outer.vertices_end(), true);
	for (auto hit = pgn.holes_begin(); hit != pgn.holes_end(); ++hit) {
		cdt.insert_constraint(hit->vertices_begin(), hit->vertices_end(), true);
	}
	std::unordered_map<typename CDT<K>::Face_handle, bool> in_domain_map;
	boost::associative_property_map< std::unordered_map<typename CDT<K>::Face_handle,bool>>
	    in_domain(in_domain_map);

	//Mark facets that are inside the domain bounded by the polygon
	CGAL::mark_domain_in_triangulation(cdt, in_domain);

	Vector<K> totalCentroid;
	Number<K> totalArea;
	for (auto fh = cdt.finite_faces_begin(); fh != cdt.finite_faces_end(); ++fh) {
		if (get(in_domain, fh)) {
			auto t = cdt.triangle(fh);
			auto a = t.area();
			auto c = CGAL::centroid(t);
			totalArea += a;
			totalCentroid += a * (c - CGAL::ORIGIN);
		}
	}

	// todo: this crashes in Debug mode; fix (that is, file CGAL issue).
	return {CGAL::ORIGIN + totalCentroid / totalArea, totalArea};
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
		Number<Exact> totalWeight;
		for (auto fh : faces) {
			auto poly = face_to_polygon_with_holes<Exact>(fh);
			auto [c, area] = centroidPWH(poly);
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

template <typename T> int sgn(T val) {
	return (T(0) < val) - (val < T(0));
}

template <class PL>
class Sampler {
  public:
	std::shared_ptr<RegionArrangement> m_regionArr;
	std::shared_ptr<PL> m_pl;
	std::shared_ptr<std::unordered_map<std::string, double>> m_regionWeight;
  private:
	int m_seed;

	std::unique_ptr<CDT<Exact>> m_cdt;
	std::vector<Triangle<Exact>> m_triangles;
	std::vector<std::string> m_triangleToRegion;
	std::vector<double> m_triangleWeights;

//	class TriangulationPainting : public GeometryPainting {
//	  private:
//		std::shared_ptr<CDT> m_cdt;
//		std::unordered_map<CDT::Face_handle, bool> m_in_domain;
//
//	  public:
//		TriangulationPainting(std::shared_ptr<CDT> cdt, std::unordered_map<CDT::Face_handle, bool> in_domain) : m_cdt(std::move(cdt)), m_in_domain(std::move(in_domain)) {};
//
//		void paint(GeometryRenderer &renderer) const override {
//			renderer.setMode(GeometryRenderer::fill);
//			renderer.setFill(Color{200, 200, 200});
//			for (auto fit = m_cdt->finite_faces_begin(); fit != m_cdt->finite_faces_end(); ++fit) {
//				if (!m_in_domain.at(fit)) continue;
//				renderer.draw(m_cdt->triangle(fit));
//			}
//			renderer.setMode(GeometryRenderer::stroke);
//			renderer.setStroke(Color{0, 0, 0}, 1.0);
//			for (auto eit = m_cdt->edges_begin(); eit != m_cdt->edges_end(); ++eit) {
//				auto f1 = eit->first;
//				auto f2 = eit->first->neighbor(eit->second);
//				if (m_in_domain.contains(f1) && m_in_domain.at(f1) || m_in_domain.contains(f2) && m_in_domain.at(f2)) {
//					auto p1 = eit->first->vertex((eit->second + 1) % 3)->point();
//					auto p2 = eit->first->vertex((eit->second + 2) % 3)->point();
//					renderer.draw(Segment<Exact>(p1, p2));
//				}
//			}
//		}
//	};

  public:
	void set_seed(int seed) {
		m_seed = seed;
	}

	void initialize_triangulation() {
		m_triangles.clear();
		m_triangleWeights.clear();
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

		for (auto t_fit = m_cdt->finite_faces_begin(); t_fit != m_cdt->finite_faces_end(); ++t_fit) {
			auto t = m_cdt->triangle(t_fit);
			auto c = centroid(t);
			auto obj = m_pl->locate(c);
			if (auto fhp = boost::get<RegionArrangement::Face_const_handle>(&obj)) {
				auto fh = *fhp;
				bool inDomain = !fh->is_unbounded() && !fh->data().empty();
				if (inDomain) {
					m_triangles.push_back(t);
					m_triangleToRegion.push_back(fh->data());
					double weight = m_regionWeight->contains(fh->data()) ? m_regionWeight->at(fh->data()) : 0;
					m_triangleWeights.push_back(weight);
				}
			}
		}
	}

	void reweight_triangulation() {
		m_triangleWeights.clear();
		for (int i = 0; i < m_triangles.size(); ++i) {
			std::string region = m_triangleToRegion[i];
			double weight = m_regionWeight->contains(region) ? m_regionWeight->at(region) : 0;
			m_triangleWeights.push_back(weight);
		}
	}

	Sampler(std::shared_ptr<RegionArrangement> regionArr, std::shared_ptr<PL> pl,
	        std::shared_ptr<std::unordered_map<std::string, double>> regionData, int seed)
	  : m_regionArr(regionArr), m_pl(pl), m_regionWeight(regionData), m_seed(seed) {
		initialize_triangulation();
	}

	WeightedPoint assignWeightToPoint(const Point<Exact>& pt) const {
		auto obj = m_pl->locate(pt);
		if (auto fhp = boost::get<RegionArrangement::Face_const_handle>(&obj)) {
			auto fh = *fhp;
			double w;
			auto region = fh->data();
			if (m_regionWeight->contains(region)) {
				w = m_regionWeight->at(fh->data());
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

	template <class InputIterator, class OutputIterator>
	void assignWeightsToPoints(InputIterator begin, InputIterator end, OutputIterator out) const {
		for (auto pit = begin; pit != end; ++pit) {
			auto pt = *pit;
			*out++ = assignWeightToPoint(pt);
		}
	}

	/// Generate samples uniformly at random over the arrangement.
	/// The weight of a sample point is equal to the weight of the region it lies in.
	template <class OutputIterator>
	void uniform_random_samples(int n, OutputIterator out) const {
		CGAL::Random rng(m_seed);
		CGAL::get_default_random() = CGAL::Random(m_seed);
		auto generator = CGAL::Random_points_in_triangles_2<Point<Exact>>(m_triangles, rng);
		std::vector<Point<Exact>> points;
		std::copy_n(generator, n, std::back_inserter(points));
		assignWeightsToPoints(points.begin(), points.end(), out);
	}

	/// Generate samples on the arrangement. The weight of a region determines the probability that a point is sampled there.
	/// Sample points have unit weight.
	template <class OutputIterator>
	void uniform_random_samples_weighted(int n, OutputIterator out) const {
		CGAL::Random rng(m_seed);
		CGAL::get_default_random() = CGAL::Random(m_seed);

		std::vector<double> weightedAreas;
		double totalWeightedArea = 0.0;
		for (int i = 0; i < m_triangles.size(); ++i) {
			auto& t = m_triangles[i];
			auto w = m_triangleWeights[i];
			totalWeightedArea += abs(approximate(t).area() * w);
			weightedAreas.push_back(totalWeightedArea);
		}

		std::vector<Point<Exact>> points;

		for (int i = 0; i < n; ++i) {
			auto x = rng.get_double(0.0, totalWeightedArea);
			int index = std::distance(weightedAreas.begin(), std::upper_bound(weightedAreas.begin(), weightedAreas.end(), x));
			auto generator = CGAL::Random_points_in_triangle_2<Point<Exact>>(m_triangles[index], rng);
			std::copy_n(generator, 1, std::back_inserter(points));
		}
		assignWeightsToPoints(points.begin(), points.end(), out);
	}

	/// Create a sample point at the centroid of each region.
	/// The weight of a sample point is the area of the region times the weight of the region.
	template <class OutputIterator>
	void centroids(OutputIterator out) const {
		for (auto fit = m_regionArr->faces_begin(); fit != m_regionArr->faces_end(); ++fit) {
			if (fit->is_unbounded() || fit->data().empty()) continue;
			auto polygon = face_to_polygon_with_holes<Exact>(fit);
			auto c = approximate(centroidPWH(polygon).first);
			auto obj = m_pl->locate(Point<Exact>(c.x(), c.y()));
			double area = approximate(polygon.outer_boundary()).area();
			for (const auto& hole : polygon.holes()) {
				area -= approximate(hole).area();
			}
			if (auto fhp = boost::get<RegionArrangement::Face_const_handle>(&obj)) {
				auto fh = *fhp;
				double w;
				auto region = fh->data();
				if (m_regionWeight->contains(region)) {
					w = m_regionWeight->at(fh->data()) * area;
				} else {
					std::cerr << "Region " << region << " has no weight" << std::endl;
					w = 0;
				}
				*out++ = WeightedPoint(approximate(c), w);
			}
		}
	}

	/// Generate samples uniformly at random over the arrangement.
	/// The weight of a sample point is equal to the weight of the region it lies in.
	/// Perturb via Voronoi.
	template <class OutputIterator>
	void voronoiUniform(int n,
	                    int iters,
	                    OutputIterator out,
	                    const std::vector<std::shared_ptr<RegionArrangement>>& compArrs,
	                    const std::vector<std::shared_ptr<Landmarks_pl<RegionArrangement>>>& pls,
	                    const std::vector<Rectangle<Exact>>& bbs,
	                    const std::vector<PolygonWithHoles<Exact>>& outerPolys) const {
		CGAL::Random rng(m_seed);
		CGAL::get_default_random() = CGAL::Random(m_seed);
		auto generator = CGAL::Random_points_in_triangles_2<Point<Exact>>(m_triangles, rng);
		std::vector<Point<Exact>> points;
		std::copy_n(generator, n, std::back_inserter(points));

		std::vector<Point<Exact>> finalPoints;

		for (int i = 0; i < compArrs.size(); ++i) {
			const auto& arr = *compArrs[i];
			const auto& pl = *pls[i];
			const auto& bb = bbs[i];
			const auto& outerPoly = outerPolys[i];

			std::vector<Point<Exact>> samplesInComponent;
			for (const auto& pt : points) {
				Point<Exact> exactPoint(pt.x(), pt.y());
				if (CGAL::oriented_side(exactPoint, outerPoly) != CGAL::NEGATIVE) {
					samplesInComponent.push_back(exactPoint);
				}
			}
			if (samplesInComponent.empty()) continue;

			std::vector<Point<Exact>> newPoints;
			for (int j = 0; j < iters; ++j) {
				std::vector<Point<Exact>> approximated;
				for (const auto& pt : samplesInComponent) {
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

		assignWeightsToPoints(finalPoints.begin(), finalPoints.end(), out);
	}
};

}

#endif //CARTOCROW_SAMPLING_H
