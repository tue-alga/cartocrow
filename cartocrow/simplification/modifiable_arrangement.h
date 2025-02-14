#pragma once

#include "../core/core.h"

namespace cartocrow::simplification {

/// Concept that requires an inner type "Map" to be defined
/// This map is assumed to be a \ref cartocrow::RegionArrangement, though this is
/// not checked by the concept
template <class T> concept MapType = requires {
	typename T::Map;
};

/// Concept for an arrangement that allows certain modification actions.
template <class T>
concept ModifiableArrangement = requires(T ma, typename T::Map::Vertex_handle v,
                                         typename T::Map::Halfedge_handle e, Point<Exact> p) {

	requires MapType<T>;

	/// Retrieves the map stored by this arrangement.
	{ ma.getMap() } -> std::same_as<typename T::Map&>;

	/// Merges a halfedge \f$e\f$ with the next halfedge, assuming the target
	/// vertex is of degree 2, and returns the new edge, incident to the same face 
	/// as the given halfedge.
	{ ma.mergeWithNext(e) } -> std::same_as<typename T::Map::Halfedge_handle>;

	/// Splits a halfedge \f$e\f$, creating a new degree-2 vertex at the given 
	/// location. Returns the new edge pointing towards the new vertex, incident 
	/// to the same face as the given halfedge.
	{ ma.split(e, p) } -> std::same_as<typename T::Map::Halfedge_handle>;

	/// Moves the vertex \f$v\f$ to the indicated location.
	{ma.shift(v, p)};
};

/// Extension of the \ref cartocrow::simplification::ModifiableArrangement
/// "ModifiableArrangement" concept. It aims to keep track of
/// the changes made, such that these can be undone and redone. The "present"
/// refers to last computed result for this arrangement, and changes should not be
/// made when the arrangement is not in the "present". This is may cause
/// unexpected behavior but is not checked explicitly.
template <class T> concept ModifiableArrangementWithHistory = requires(T ma, Number<Exact> c) {
	requires ModifiableArrangement<T>;

	/// Sets the arrangement to its latest computed result
	{ma.goToPresent()};

	/// Starts a batch of operations, that together incur a cost c
	{ma.startBatch(c)};
	/// Ends a batch of operations
	{ma.endBatch()};
};

} // namespace cartocrow::simplification