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

struct Hyperedge;

struct Relation {
	int left;
	int right;
	Order preference;
	Order ordering;
	std::vector<std::shared_ptr<Hyperedge>> hyperedges;
	Relation(int left, int right, Order preference, Order ordering) :
	 left(left), right(right), preference(preference), ordering(ordering) {}
};

bool operator==(const Relation& lhs, const Relation& rhs);

std::ostream& operator<<(std::ostream& out, const Relation& r);

struct Hyperedge {
	std::vector<int> origins;
	std::vector<std::shared_ptr<Relation>> relations;
	Hyperedge(std::vector<int> origins, std::vector<std::shared_ptr<Relation>> relations) :
		origins(origins), relations(relations) {};
};

std::optional<std::vector<int>> getRelationOrder(const Hyperedge& e);
void setRelationOrder(Hyperedge& e, const std::vector<int>& ordering);
std::optional<std::vector<int>> computeTotalOrder(const std::vector<int>& origins, const std::vector<std::shared_ptr<Relation>>& relations);

bool operator==(const Hyperedge& lhs, const Hyperedge& rhs);

struct FaceData {
	std::vector<int> origins;
	std::vector<std::shared_ptr<Relation>> relations;
	std::vector<int> ordering;
	std::unordered_map<int, std::vector<CSPolyline>> morphedEdges;
	std::unordered_map<int, std::vector<CSPolygon>> morphedFace;
};

// todo: edge case where edges of dilated patterns overlap, so a half-edge may have multiple origins.
struct HalfEdgeData {
	std::vector<int> origins;
};

struct VertexData {

};

using DilatedPatternArrangement =
    CGAL::Arrangement_with_history_2<CSTraits,
                        CGAL::Arr_extended_dcel<CSTraits, VertexData, HalfEdgeData, FaceData>>;

//DilatedPatternArrangement
//dilateAndArrange(const Partition& partition, const GeneralSettings& gs, const ComputeDrawingSettings& cds);

using Face = DilatedPatternArrangement::Face;
using FaceH = DilatedPatternArrangement::Face_handle;
using FaceCH = DilatedPatternArrangement::Face_const_handle;
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

		pointer ptr() const {
			return *m_faceHandleIterator;
		}

		pointer handle() const {
			return *m_faceHandleIterator;
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

template <class Traits, class OutputIterator, class Ccb>
void boundaryParts(Ccb ccb, int i, OutputIterator out) {
	// First find a half-edge at the start of a 'part' of this CCB (connected component of the boundary).
	auto circ = ccb;

	auto originates_from = [](Ccb circ, int i) {
		auto& os = circ->data().origins;
		return std::find(os.begin(), os.end(), i) != os.end();
	};

	bool found = true;
	while (!originates_from(circ, i)) {
		++circ;
		if (circ == ccb) {
			found = false;
			break;
		}
	}

	if (!found) {
		return;
	}

	auto start = circ;
	do {
		auto prev = circ;
		--prev;
		if (originates_from(prev, i)) {
			circ = prev;
		} else {
			break;
		}
	} while (circ != start);
	assert(originates_from(circ, i));

	Traits traits;
	auto opposite = traits.construct_opposite_2_object();

	// Next, make a polyline for every connected part of the boundary that originates from i.
	std::vector<X_monotone_curve_2> xm_curves;
	auto curr = circ;
	do {
		if (originates_from(curr, i)) {
			auto& curve = curr->curve();
			if (curr->source()->point() == curve.source()) {
				xm_curves.push_back(curve);
			} else {
				xm_curves.push_back(opposite(curve));
			}
		} else {
			if (!xm_curves.empty()) {
				++out = CSPolyline(xm_curves.begin(), xm_curves.end());
				xm_curves.clear();
			}
		}
	} while (++curr != circ);
	if (!xm_curves.empty()) {
		++out = CSPolyline(xm_curves.begin(), xm_curves.end());
		xm_curves.clear();
	}
}

std::vector<CSPolyline> boundaryParts(const Component& c, int i);
std::vector<CSPolyline> boundaryParts(FaceH f, int i);
std::vector<std::vector<std::pair<CSPolyline, int>>> originCcbs(const Component& c);

// exposed for debugging purposes
std::vector<std::vector<std::pair<int, Circle<Inexact>>>> connectedDisks(const std::vector<Circle<Inexact>>& disks);
CSPolygon thinRectangle(const Point<Exact>& p, const OneRootPoint& n, const Number<Exact>& w);

CSPolygon morph(const std::vector<CSPolyline>& boundaryParts, const CSPolygon& componentShape, const std::vector<Circle<Exact>>& inclDisks,
				const std::vector<Circle<Exact>>& exclDisks, const GeneralSettings& gs, const ComputeDrawingSettings& cds);

CSPolyline associatedBoundary(const CSPolygon& component, const CSPolygon& morphedComponent, const CSPolyline& boundaryPart);

struct IncludeExcludeDisks {
	std::vector<Circle<Exact>> include;
	std::vector<Circle<Exact>> exclude;
};

bool isStraight(const CSPolyline& polyline);

class DilatedPatternDrawing {
  public:
	DilatedPatternDrawing(const Partition& partition, const GeneralSettings& gs, const ComputeDrawingSettings& cds);

	std::vector<Component> intersectionComponents(int i) const;
	std::vector<Component> intersectionComponents(int i, int j) const;
	std::shared_ptr<Relation> computePreference(int i, int j, const Component& c);

	IncludeExcludeDisks includeExcludeDisks(int i, int j, const Component& c) const;
	IncludeExcludeDisks includeExcludeDisks(int i, const std::unordered_set<int>& js, const Component& c) const;

	std::vector<std::shared_ptr<Hyperedge>> hyperedges() const;

	void drawFaceFill(FaceH fh, renderer::GeometryRenderer& renderer,
				      const GeneralSettings& gs, const DrawSettings& ds) const;
	void drawFaceStroke(FaceH fh, renderer::GeometryRenderer& renderer,
				        const GeneralSettings& gs, const DrawSettings& ds) const;
	std::optional<std::vector<int>> totalStackingOrder() const;

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
