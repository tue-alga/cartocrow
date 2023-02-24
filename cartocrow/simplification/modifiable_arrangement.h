#pragma once

#include "../core/core.h"

namespace cartocrow::simplification {

/// Concept that requires an inner type "Map" to be defined
/// This map is assumed to be a \ref RegionArrangement, though this is not checked by the concept
template <class T> concept MapType = requires {
	typename T::Map;
};

/// Concept for an arrangement that allows certain modification actions.
template <class T>
concept ModifiableArrangement = requires(T ma, typename T::Map::Halfedge_handle e, Number<Exact> c) {

	requires MapType<T>;

	/// Retrieves the map stored by this arrangement.
	{ ma.getMap() } -> std::same_as<typename T::Map&>;

	/// Merges a halfedge \f$e\f$ with the next halfedge, assuming the target
	/// vertex is of degree 2, and returns the new edge. The cost \f$c\f$ of this
	/// operation is provided for optional, additional bookkeeping.
	{ ma.mergeWithNext(e, c) } -> std::same_as<typename T::Map::Halfedge_handle>;
};

/// Extension of the \ref ModifiableArrangement concept. It aims to keep track of
/// the changes made, such that these can be undone and redone. The "present"
/// refers to last computed result for this arrangement, and changes should not be
/// made when the arrangement is not in the "present". This is may cause
/// unexpected behavior but is not checked explicitly.
template <class T>
concept ModifiableArrangementWithHistory = requires(T ma, int c, Number<Exact> t) {
	requires ModifiableArrangement<T>;

	/// Sets the arrangement to its latest computed result
	{ma.goToPresent()};
};

} // namespace cartocrow::simplification