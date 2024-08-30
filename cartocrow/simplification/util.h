#pragma once

#include "../core/core.h"

/// Various utility functions for working with and modifying arrangements

namespace cartocrow::simplification::util {

/// Computes the area of a face in an arrangement. For the unbounded face, this 
/// number is negative: the total size of all its holes
template <class TArr> Number<Exact> faceArea(typename TArr::Face_handle face);

/// Merges an edge with its previous edge, assuming the common point is of degree
/// 2, returning the new edge in the same direction
template <class TArr>
inline TArr::Halfedge_handle mergeWithPrev(TArr& dcel, typename TArr::Halfedge_handle edge);

/// Merges an edge with its next edge, assuming the common point is of degree 2, 
/// returning the new edge in the same direction
template <class TArr>
inline TArr::Halfedge_handle mergeWithNext(TArr& dcel, typename TArr::Halfedge_handle edge);

/// Shifts a vertex to a new location without structurally changing the arrangement
template <class TArr>
inline void shift(TArr& dcel, typename TArr::Vertex_handle vertex, Point<Exact> pt);

/// Shifts both endpoints of an edge to new locations without structurally 
/// changing the arrangement
template <class TArr>
inline void shift(TArr& dcel, typename TArr::Halfedge_handle edge, Point<Exact> pt_source,
                  Point<Exact> pt_target);

/// Splits an edge by introducing a new degree-2 vertex and the given location, 
/// returning the incoming edge of the new point in the same direction as the given edge
template <class TArr>
inline TArr::Halfedge_handle split(TArr& dcel, typename TArr::Halfedge_handle edge, Point<Exact> pt);

} // namespace cartocrow::simplification::util

#include "util.hpp"