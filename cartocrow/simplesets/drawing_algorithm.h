#ifndef CARTOCROW_DRAWING_ALGORITHM_H
#define CARTOCROW_DRAWING_ALGORITHM_H

#include "partition.h"
#include "dilated/dilated_poly.h"
#include "../renderer/geometry_painting.h"
#include <CGAL/Arrangement_with_history_2.h>

#include <utility>

namespace cartocrow::simplesets {
enum Order {
	SMALLER = -1,
	EQUAL = 0,
	GREATER = 1,
};

std::string to_string(Order ord);

std::ostream& operator<<(std::ostream& out, const Order& o);

struct Relation {
	int left;
	int right;
	Order preference;
	Order ordering;
};

bool operator==(const Relation& lhs, const Relation& rhs);

std::ostream& operator<<(std::ostream& out, const Relation& r);

class Hyperedge {
  public:
	std::vector<int> origins;
	std::vector<Relation> relations;
};

std::optional<std::vector<int>> getRelationOrder(const Hyperedge& e);
void setRelationOrder(Hyperedge& e, const std::vector<int>& ordering);
std::optional<std::vector<int>> computeTotalOrder(const std::vector<int>& origins, const std::vector<Relation>& relations);

bool operator==(const Hyperedge& lhs, const Hyperedge& rhs);

struct FaceData {
	std::vector<int> origins;
	std::vector<Relation> relations;
	std::vector<int> ordering;
	std::unordered_map<int, std::vector<CSPolyline>> morphedEdges;
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
//	typedef std::vector<FaceH>::iterator Face_iterator;
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

		ComponentCcbCirculator(Arr::Halfedge_handle halfedge, std::function<bool(FaceH)> in_component)
		    : m_halfedge(halfedge), m_in_component(std::move(in_component)) {};

		value_type operator*() const {
			return *m_halfedge;
		}

		pointer operator->() const {
			return m_halfedge;
		}

		pointer ptr() const {
			return m_halfedge;
		}

		pointer handle() const {
			return m_halfedge;
		}

		Self& operator++() {
			m_halfedge = m_halfedge->next();
			while (m_in_component(m_halfedge->twin()->face())) {
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
			while (m_in_component(m_halfedge->twin()->face())) {
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

	class Face_const_iterator {
	  private:
		using Self = Face_const_iterator;
		std::vector<FaceH>::const_iterator m_faceHandleIterator;

	  public:
		using iterator_category = std::input_iterator_tag;
		using value_type = Arr::Face;
		using difference_type = std::ptrdiff_t;
		using pointer = Arr::Face_handle;
		using reference = value_type&;

		Face_const_iterator(std::vector<FaceH>::const_iterator faceHandleIterator) :
		      m_faceHandleIterator(faceHandleIterator) {};

		value_type operator*() const {
			return *(*m_faceHandleIterator);
		}

		pointer operator->() const {
			return *m_faceHandleIterator;
		}

		Self& operator++() {
			++m_faceHandleIterator;
			return *this;
		};

		Self operator++(int) {
			Self tmp = *this;
			this->operator++();
			return tmp;
		}

		bool operator==(const Self& other) const {
			return m_faceHandleIterator == other.m_faceHandleIterator;
		}

		bool operator!=(const Self& other) const {
			return m_faceHandleIterator != other.m_faceHandleIterator;
		}
	};

	Component(std::vector<FaceH> faces, std::vector<HalfEdgeH> boundaryEdges, std::function<bool(FaceH)> inComponent);

	bool has_outer_ccb() const {
		return !m_outer_ccbs.empty();
	}
	Size number_of_outer_ccbs() const {
		return m_outer_ccbs.size();
	}
	std::vector<ComponentCcbCirculator>::const_iterator outer_ccbs_begin() const {
		return m_outer_ccbs.cbegin();
	}
	std::vector<ComponentCcbCirculator>::const_iterator outer_ccbs_end() const {
		return m_outer_ccbs.cend();
	}

	ComponentCcbCirculator outer_ccb() const {
		return m_outer_ccbs[0];
	}
	std::vector<ComponentCcbCirculator>::const_iterator inner_ccbs_begin() const {
		return m_inner_ccbs.cbegin();
	}
	std::vector<ComponentCcbCirculator>::const_iterator inner_ccbs_end() const {
		return m_inner_ccbs.cend();
	}
	Size number_of_inner_ccbs() const {
		return m_inner_ccbs.size();
	}

	// Hole is an alias for inner_ccb
	std::vector<ComponentCcbCirculator>::const_iterator holes_begin() const {
		return inner_ccbs_begin();
	}
	std::vector<ComponentCcbCirculator>::const_iterator holes_end() const {
		return inner_ccbs_end();
	}
	Size number_of_holes() {
		return number_of_inner_ccbs();
	}

	Face_const_iterator faces_begin() const {
		return {m_faces.cbegin()};
	}
	Face_const_iterator faces_end() const {
		return {m_faces.cend()};
	}

  private:
	std::vector<FaceH> m_faces;
	std::function<bool(FaceH)> m_in_component;
	std::vector<ComponentCcbCirculator> m_outer_ccbs;
	std::vector<ComponentCcbCirculator> m_inner_ccbs;
};

std::vector<CSPolyline> boundaryParts(const Component& c, int i);

CSPolyline morph(const CSPolyline& boundaryPart, const CSPolygon& componentShape, const std::vector<Circle<Exact>>& inclDisks,
                 const std::vector<Circle<Exact>>& exclDisks, const GeneralSettings& gs, const ComputeDrawingSettings& cds);

struct IncludeExcludeDisks {
	std::vector<Circle<Exact>> include;
	std::vector<Circle<Exact>> exclude;
};

class DilatedPatternDrawing {
  public:
	DilatedPatternDrawing(const Partition& partition, const GeneralSettings& gs, const ComputeDrawingSettings& cds);

	std::vector<Component> intersectionComponents(int i) const;
	std::vector<Component> intersectionComponents(int i, int j) const;
	Relation computePreference(int i, int j, const Component& c);

	IncludeExcludeDisks includeExcludeDisks(int i, int j, const Component& c);
	IncludeExcludeDisks includeExcludeDisks(int i, std::vector<int> js, const Component& c);

	std::vector<Hyperedge> hyperedges();
	
	DilatedPatternArrangement m_arr;
	std::unordered_map<int, std::vector<FaceH>> m_iToFaces;
	std::map<X_monotone_curve_2, int> m_curve_to_origin;
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
