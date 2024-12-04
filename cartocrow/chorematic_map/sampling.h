#ifndef CARTOCROW_SAMPLING_H
#define CARTOCROW_SAMPLING_H

#include "../core/arrangement_helpers.h"
#include "../core/region_arrangement.h"

#include "weighted_point.h"

#include <CGAL/Constrained_Delaunay_triangulation_2.h>
#include <CGAL/Random.h>
#include <CGAL/mark_domain_in_triangulation.h>
#include <CGAL/point_generators_2.h>
#include <CGAL/centroid.h>

namespace cartocrow::chorematic_map {

typedef CGAL::Triangulation_vertex_base_2<Exact>                      Vb;
typedef CGAL::Constrained_triangulation_face_base_2<Exact>            Fb;
typedef CGAL::Triangulation_data_structure_2<Vb,Fb>                   TDS;
typedef CGAL::No_constraint_intersection_requiring_constructions_tag  Itag;
typedef CGAL::Constrained_Delaunay_triangulation_2<Exact, TDS, Itag>  CDT;

template <typename T> int sgn(T val) {
	return (T(0) < val) - (val < T(0));
}

template <class PL>
class Sampler {
  private:
	const RegionArrangement& m_regionArr;
	const PL& m_pl;
	const std::unordered_map<std::string, double>& m_regionWeight;
	int m_seed;

	std::unique_ptr<CDT> m_cdt;
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
		m_cdt = std::make_unique<CDT>();
		for (auto fit = m_regionArr.faces_begin(); fit != m_regionArr.faces_end(); ++fit) {
			std::vector<RegionArrangement::Ccb_halfedge_const_circulator> ccbs;
			std::copy(fit->outer_ccbs_begin(), fit->outer_ccbs_end(), std::back_inserter(ccbs));
			std::copy(fit->inner_ccbs_begin(), fit->inner_ccbs_end(), std::back_inserter(ccbs));
			for (auto ccb : ccbs) {
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
			auto obj = m_pl.locate(c);
			if (auto fhp = boost::get<RegionArrangement::Face_const_handle>(&obj)) {
				auto fh = *fhp;
				bool inDomain = !fh->is_unbounded() && !fh->data().empty();
				if (inDomain) {
					m_triangles.push_back(t);
					m_triangleToRegion.push_back(fh->data());
					double weight = m_regionWeight.contains(fh->data()) ? m_regionWeight.at(fh->data()) : 0;
					m_triangleWeights.push_back(weight);
				}
			}
		}
	}

	void reweight_triangulation() {
		m_triangleWeights.clear();
		for (int i = 0; i < m_triangles.size(); ++i) {
			std::string region = m_triangleToRegion[i];
			double weight = m_regionWeight.contains(region) ? m_regionWeight.at(region) : 0;
			m_triangleWeights.push_back(weight);
		}
	}

	Sampler(const RegionArrangement& regionArr, const PL& pl,
	        const std::unordered_map<std::string, double>& regionData, int seed)
	  : m_regionArr(regionArr), m_pl(pl), m_regionWeight(regionData), m_seed(seed) {
		initialize_triangulation();
	}

	/// Generate samples uniformly at random over the arrangement.
	/// The weight of a sample point is equal to the weight of the region it lies in.
	template <class OutputIterator>
	void uniform_random_samples(int n, OutputIterator out) {
		CGAL::Random rng(m_seed);
		CGAL::get_default_random() = CGAL::Random(m_seed);
		auto generator = CGAL::Random_points_in_triangles_2<Point<Exact>>(m_triangles, rng);
		std::vector<Point<Exact>> points;
		std::copy_n(generator, n, std::back_inserter(points));
		for (const auto& pt : points) {
			auto obj = m_pl.locate(pt);
			if (auto fhp = boost::get<RegionArrangement::Face_const_handle>(&obj)) {
				auto fh = *fhp;
				double w;
				auto region = fh->data();
				if (m_regionWeight.contains(region)) {
					w = m_regionWeight.at(fh->data());
				} else {
					std::cerr << "Region " << region << " has no weight" << std::endl;
					w = 0;
				}
				*out++ = WeightedPoint(approximate(pt), w);
			}
		}
	}

	/// Generate samples on the arrangement. The weight of a region determines the probability that a point is sampled there.
	/// Sample points have unit weight.
	template <class OutputIterator>
	void uniform_random_samples_weighted(int n, OutputIterator out) {
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
//			std::binary_search(areas.begin(), areas.end(), x);
//			std::upper_bound()
			int index = std::distance(weightedAreas.begin(), std::upper_bound(weightedAreas.begin(), weightedAreas.end(), x));
			auto generator = CGAL::Random_points_in_triangle_2<Point<Exact>>(m_triangles[index], rng);
			std::copy_n(generator, 1, std::back_inserter(points));
		}

		for (const auto& pt : points) {
			auto obj = m_pl.locate(pt);
			if (auto fhp = boost::get<RegionArrangement::Face_const_handle>(&obj)) {
				auto fh = *fhp;
				double w;
				auto region = fh->data();
				if (m_regionWeight.contains(region)) {
					w = sgn(m_regionWeight.at(region));
				} else {
					std::cerr << "Region " << region << " has no weight" << std::endl;
					w = 0;
				}
				*out++ = WeightedPoint(approximate(pt), w);
			}
		}
	}

	/// Create a sample point at the centroid of each region.
	/// The weight of a sample point is the area of the region times the weight of the region.
	template <class OutputIterator>
	void centroids(OutputIterator out) {
		for (auto fit = m_regionArr.faces_begin(); fit != m_regionArr.faces_end(); ++fit) {
			if (fit->is_unbounded() || fit->data().empty()) continue;
			auto polygon = approximate(face_to_polygon_with_holes<Exact>(fit));
			auto c = CGAL::centroid(polygon.outer_boundary().vertices_begin(), polygon.outer_boundary().vertices_end());
			auto obj = m_pl.locate(Point<Exact>(c.x(), c.y()));
			double area = polygon.outer_boundary().area();
			for (const auto& hole : polygon.holes()) {
				area -= hole.area();
			}
			if (auto fhp = boost::get<RegionArrangement::Face_const_handle>(&obj)) {
				auto fh = *fhp;
				double w;
				auto region = fh->data();
				if (m_regionWeight.contains(region)) {
					w = m_regionWeight.at(fh->data()) * area;
				} else {
					std::cerr << "Region " << region << " has no weight" << std::endl;
					w = 0;
				}
				*out++ = WeightedPoint(approximate(c), w);
			}
		}
	}
};

}

#endif //CARTOCROW_SAMPLING_H
