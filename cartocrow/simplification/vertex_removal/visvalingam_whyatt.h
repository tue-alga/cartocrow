#pragma once

#include "../../core/core.h"
#include "../../core/arrangement_map.h"
#include "../oblivious_arrangement.h"
#include "../historic_arrangement.h"
#include "vertex_removal.h"

namespace cartocrow::simplification {

template <class FaceData = std::monostate>
struct VWVertex;
template <class FaceData = std::monostate>
struct VWEdge;

/// These traits implements the \ref cartocrow::simplification::VertexRemovalTraits 
/// "VertexRemovalTraits" to create a topologically-safe variant of the 
/// Visvalingam-Whyatt algorithm. That is, the cost of removing a vertex is equal 
/// to the area spanned by its triangle.
///
/// Title: Line generalisation by repeated elimination of points
/// 
/// Authors: M. Visvalingam and J. D. Whyatt
/// 
/// Doi: https://doi.org/10.1179/000870493786962263
template <class FaceData = std::monostate>
struct VWTraits {
	using Map = ArrangementMap<VWVertex<FaceData>, VWEdge<FaceData>, FaceData>;

	static void vrSetCost(Map::Vertex_handle v, Triangle<Exact> T);
	static Number<Exact> vrGetCost(Map::Vertex_handle v);

	static void vrSetBlockingNumber(Map::Vertex_handle v, int b);
	static int vrGetBlockingNumber(Map::Vertex_handle v);

	static void vrSetHalfedge(Map::Vertex_handle v, Map::Halfedge_handle inc);
	static Map::Halfedge_handle vrGetHalfedge(Map::Vertex_handle v);

	static void histSetData(Map::Halfedge_handle e, HalfedgeOperation<VWTraits>* data);
	static HalfedgeOperation<VWTraits>* histGetData(Map::Halfedge_handle e);
};

template <class FaceData = std::monostate>
using VWSimplification = VertexRemovalSimplification<ObliviousArrangement<VWTraits<FaceData>>,VWTraits<FaceData>>;
template <class FaceData = std::monostate>
using VWSimplificationWithHistory = VertexRemovalSimplification<HistoricArrangement<VWTraits<FaceData>>,VWTraits<FaceData>>;

/// The data associated with a vertex in the arrangement used in \ref VWTraits.
template <class FaceData>
struct VWVertex {
	int block;
	Number<Exact> cost;
	VWTraits<FaceData>::Map::Halfedge_handle inc;
};

/// The data associated with a halfedge in the arrangement used in \ref VWTraits.
template <class FaceData>
struct VWEdge {
	HalfedgeOperation<VWTraits<FaceData>>* hist = nullptr;
};

} // namespace cartocrow::simplification

#include "visvalingam_whyatt.hpp"
