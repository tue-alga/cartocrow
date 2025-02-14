#ifndef CARTOCROW_DRAWING_ALGORITHM_H
#define CARTOCROW_DRAWING_ALGORITHM_H

#include "../renderer/geometry_painting.h"
#include "cartocrow/core/arrangement_helpers.h"
#include "cartocrow/simplesets/dilated/dilated_poly.h"
#include "partition.h"
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

struct HalfEdgeData {
	std::vector<int> origins;
};

struct VertexData {

};

using DilatedPatternArrangement =
    CGAL::Arrangement_with_history_2<ArrCSTraits,
                        CGAL::Arr_extended_dcel<ArrCSTraits, VertexData, HalfEdgeData, FaceData>>;

//DilatedPatternArrangement
//dilateAndArrange(const Partition& partition, const GeneralSettings& gs, const ComputeDrawingSettings& cds);

using Face = DilatedPatternArrangement::Face;
using FaceH = DilatedPatternArrangement::Face_handle;
using FaceCH = DilatedPatternArrangement::Face_const_handle;
using VertexH = DilatedPatternArrangement::Vertex_handle;
using HalfEdgeH = DilatedPatternArrangement::Halfedge_handle;

using CComponent = Component<DilatedPatternArrangement>;

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
	std::vector<CSXMCurve> xm_curves;
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

std::vector<CSPolyline> boundaryParts(const CComponent& c, int i);
std::vector<CSPolyline> boundaryParts(FaceH f, int i);
std::vector<std::vector<std::pair<CSPolyline, int>>> originCcbs(const CComponent& c);

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

	std::vector<CComponent> intersectionComponents(int i) const;
	std::vector<CComponent> intersectionComponents(int i, int j) const;
	std::shared_ptr<Relation> computePreference(int i, int j, const CComponent& c);

	IncludeExcludeDisks includeExcludeDisks(int i, int j, const CComponent& c) const;
	IncludeExcludeDisks includeExcludeDisks(int i, const std::unordered_set<int>& js, const CComponent& c) const;

	std::vector<std::shared_ptr<Hyperedge>> hyperedges() const;

	void drawFaceFill(FaceH fh, renderer::GeometryRenderer& renderer,
				      const GeneralSettings& gs, const DrawSettings& ds) const;
	void drawFaceStroke(FaceH fh, renderer::GeometryRenderer& renderer,
				        const GeneralSettings& gs, const DrawSettings& ds) const;
	std::optional<std::vector<int>> totalStackingOrder() const;

	DilatedPatternArrangement m_arr;
	std::unordered_map<int, std::vector<FaceH>> m_iToFaces;
	std::map<CSXMCurve, int> m_curve_to_origin;
	std::vector<DilatedPoly> m_dilated;
	const GeneralSettings& m_gs;
	const ComputeDrawingSettings& m_cds;

  private:
};

/// A \ref renderer::GeometryPainting "Painting" of a SimpleSets visualization.
///
/// A SimpleSets visualization is created in three steps:
/// 1. \ref cartocrow::simplesets::partition "Partition" the categorical points (\ref CatPoint) into \ref cartocrow::simplesets::Pattern "patterns".
/// 2. \ref Dilate the patterns. The patterns from the first step are polygonal (\ref PolyPattern); therefore, the dilated patterns consists of circular arcs and line segments (\ref DilatedPoly).
/// 3. Render the dilated patterns to a "physically-realizable" \cite proportional_symbol_maps_algorithmica drawing. This involves determining local stacking orders, and cutting shapes such that points beneath it are exposed.
///
/// \image html "SimpleSets-pipeline.svg"
///
/// The \ref partition function performs step 1. In fact, it creates a sequence of partitions that range from small patterns covering little of the map, to very large patterns.
/// The \ref DilatedPatternDrawing takes one such \ref Partition of polygonal patterns and performs steps 2 and 3; that is, it dilates the patterns and resolves overlap.
///
/// One can draw a \ref Partition using the \ref PartitionPainting.
/// To draw the final SimpleSets visualization use this \ref SimpleSetsPainting.
///
/// Functions of SimpleSets depend on various settings:
/// - \ref GeneralSettings : parameters such as point size.
/// - \ref PartitionSettings : parameters for the \ref partition function.
/// - \ref ComputeDrawingSettings : parameters for the \ref DilatedPatternDrawing class.
/// - \ref DrawSettings : parameters for drawing functions.
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
