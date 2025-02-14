#pragma once

#include "../../core/core.h"
#include "../../core/arrangement_map.h"
#include "../historic_arrangement.h"
#include "../oblivious_arrangement.h"
#include "edge_collapse.h"

namespace cartocrow::simplification {

struct KSBBVertex;
template <class FaceData>
struct KSBBEdge;

/// These traits implements the \ref cartocrow::simplification::EdgeCollapseTraits
/// "EdgeCollapseTraits" to create a topologically-safe variant of the
/// Kronenfeld et al. algorithm. That is, the cost of collapsing an edge is its
/// symmetric difference (areal displacement), and the collapse point is chosen to
/// minimize this measure.
///
/// Title: Simplification of Polylines by Segment Collapse: Minimizing Areal Displacement While Preserving Area
///
/// Authors: Barry J. Kronenfeld, Lawrence V.Stanislawski, Barbara P.Buttenfieldx, Tyler Brockmeyer
///
/// Doi: https://doi.org/10.1080/23729333.2019.1631535
template <class FaceData = std::monostate>
struct KSBBTraits {
	using Map = ArrangementMap<KSBBVertex, KSBBEdge<FaceData>, FaceData>;

	static void ecSetEdgeMark(Map::Halfedge_handle e, ECEdgeMark m);
	static ECEdgeMark ecGetEdgeMark(Map::Halfedge_handle e);

	static Collapse ecComputeCollapse(Map::Halfedge_handle e);
	static void ecSetCollapse(Map::Halfedge_handle e, Collapse collapse);
	static Collapse ecGetCollapse(Map::Halfedge_handle e);

	static void ecSetCost(Map::Halfedge_handle e);
	static Number<Exact> ecGetCost(Map::Halfedge_handle e);

	static void ecSetBlockingNumber(Map::Halfedge_handle e, int b);
	static int ecGetBlockingNumber(Map::Halfedge_handle e);

	static void histSetData(Map::Halfedge_handle e, HalfedgeOperation<KSBBTraits>* data);
	static HalfedgeOperation<KSBBTraits>* histGetData(Map::Halfedge_handle e);
};

template <class FaceData>
using KSBBSimplification = EdgeCollapseSimplification<ObliviousArrangement<KSBBTraits<FaceData>>, KSBBTraits<FaceData>>;
template <class FaceData>
using KSBBSimplificationWithHistory =
    EdgeCollapseSimplification<HistoricArrangement<KSBBTraits<FaceData>>, KSBBTraits<FaceData>>;

/// The data associated with a vertex in the arrangement used in \ref KSBBTraits.
struct KSBBVertex {
};

/// The data associated with a halfedge in the arrangement used in \ref KSBBTraits.
template <class FaceData>
struct KSBBEdge {
	int block = 0;
	Number<Exact> cost = 0;
	ECEdgeMark mark = ECEdgeMark::NONE; 
	Collapse collapse;

	HalfedgeOperation<KSBBTraits<FaceData>>* hist = nullptr;
};

} // namespace cartocrow::simplification

#include "kronenfeld_etal.hpp"
