#ifndef CARTOCROW_DRAWING_ALGORITHM_H
#define CARTOCROW_DRAWING_ALGORITHM_H

#include "partition.h"
#include "dilated/dilated_poly.h"
#include "../renderer/geometry_painting.h"
#include <CGAL/Arrangement_with_history_2.h>

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
	Component(std::vector<FaceH> faces);
	std::vector<FaceH> m_faces;

	HalfEdgeH boundaryEdge();
	HalfEdgeH nextBoundaryEdge(HalfEdgeH);
	HalfEdgeH prevBoundaryEdge(HalfEdgeH);
	std::vector<HalfEdgeH> boundaryEdges();

  private:
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
