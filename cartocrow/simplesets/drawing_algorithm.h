#ifndef CARTOCROW_DRAWING_ALGORITHM_H
#define CARTOCROW_DRAWING_ALGORITHM_H

#include "partition.h"
#include "dilated/dilated_poly.h"
#include "../renderer/geometry_painting.h"
#include <CGAL/Arrangement_with_history_2.h>

#include <utility>

namespace cartocrow::simplesets {
struct Relation {
	int left;
	int right;
	CGAL::Sign preference;
	CGAL::Sign ordering;
};

struct FaceData {
	std::vector<int> origins;
	std::vector<Relation> relations;
};

// todo: edge case where edges of dilated patterns overlap, so a half-edge may have multiple origins.
struct HalfEdgeData {
	int origin;
};

struct VertexData {

};

using DilatedPatternArrangement =
    CGAL::Arrangement_with_history_2<CSTraits,
                        CGAL::Arr_extended_dcel<CSTraits, VertexData, HalfEdgeData, FaceData>>;

//DilatedPatternArrangement
//dilateAndArrange(const Partition& partition, const GeneralSettings& gs, const ComputeDrawingSettings& cds);

using FaceH = DilatedPatternArrangement::Face_handle;
using VertexH = DilatedPatternArrangement::Vertex_handle;
using HalfEdgeH = DilatedPatternArrangement::Halfedge_handle;

class Component {
  public:
	typedef DilatedPatternArrangement Arr;
	typedef Arr::Size Size;

	class ComponentCcbCirculator {
	  private:
		using Self = ComponentCcbCirculator;
		Arr::Halfedge_handle m_halfedge;
		std::function<bool(FaceH)> m_in_component;

	  public:
		using iterator_category = std::bidirectional_iterator_tag;
		using value_type = Arr::Halfedge;
		using difference_type = std::ptrdiff_t;
		using pointer = Arr::Halfedge_handle;
		using reference = value_type&;

		ComponentCcbCirculator(std::function<bool(FaceH)> in_component) : m_in_component(std::move(in_component)) {};

		value_type operator*() const {
			return *m_halfedge;
		}

		pointer operator->() const {
			return m_halfedge;
		}

		Self& operator++() {
			m_halfedge = m_halfedge->next();
			while (!m_in_component(m_halfedge->twin()->face())) {
				m_halfedge = m_halfedge->twin()->next();
			}
		    return *this;
		};

		Self operator++(int) {
			Self tmp = *this;
			this->operator++();
			return tmp;
		}

		Self& operator--() {
			m_halfedge = m_halfedge->prev();
			while (!m_in_component(m_halfedge->twin()->face())) {
				m_halfedge = m_halfedge->twin()->prev();
			}
			return *this;
		};

		Self operator--(int) {
			Self tmp = *this;
			this->operator--();
			return tmp;
		}

		bool operator==(const Self& other) const {
			return m_halfedge == other.m_halfedge;
		}

		bool operator!=(const Self& other) const {
			return m_halfedge != other.m_halfedge;
		}
	};

//	Component(std::vector<FaceH> faces);
	Component(std::function<bool(FaceH)> in_component);

//	bool has_outer_ccb() const;
//	Hole_iterator holes_begin();
//	Hole_iterator holes_end();
//	Inner_ccb_iterator inner_ccbs_begin();
//	Inner_ccb_iterator inner_ccbs_end();
//	Size number_of_holes();
//	Size number_of_inner_ccbs();
//	Size number_of_outer_ccbs();
//	ComponentCcbCirculator outer_ccb();
//	Outer_ccb_iterator outer_ccbs_begin();
//	Outer_ccb_iterator outer_ccbs_end();
//	Face_iterator faces_begin();
//	Face_iterator faces_end();

  private:
	std::vector<FaceH> m_faces;
	std::function<bool(FaceH)> m_in_component;
};

class DilatedPatternDrawing {
  public:
	DilatedPatternDrawing(const Partition& partition, const GeneralSettings& gs, const ComputeDrawingSettings& cds);

	std::vector<Component> intersectionComponents(int i);
	std::vector<Component> intersectionComponents(int i, int j);
	Relation computePreference(int i, int j, const Component& c);

	DilatedPatternArrangement m_arr;
	std::unordered_map<int, std::vector<FaceH>> m_iToFaces;
	std::map<CSTraits::X_monotone_curve_2, int> m_curve_to_origin;
	std::vector<Dilated> m_dilated;
	const GeneralSettings& m_gs;
	const ComputeDrawingSettings& m_cds;

  private:
};

class SimpleSetsPainting : public renderer::GeometryPainting {
  public:
	SimpleSetsPainting(const DilatedPatternDrawing& dpd, const DrawSettings& ds);
	void paint(renderer::GeometryRenderer& renderer) const override;

  private:
	const DrawSettings& m_ds;
	const DilatedPatternDrawing& m_dpd;

};
}

#endif //CARTOCROW_DRAWING_ALGORITHM_H
