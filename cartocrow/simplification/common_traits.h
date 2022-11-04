#pragma once

#include "../core/core.h"

/// Defines various basic traits for simplification algorithms

namespace cartocrow::simplification {

// Concept that requires an inner type "Map" to be defined
// This map is assumed to be a \ref RegionArrangement, though this is not checked by the concept
template <class T> concept MapType = requires {
	typename T::Map;
};

// Concept for an arrangement that allows certain modification actions
template <class T>
concept ModifiableArrangement = requires(T t, typename T::Map::Halfedge_handle e, Number<Exact> c) {

	requires MapType<T>;

	{ t.getMap() } -> std::same_as<typename T::Map&>;
	{ t.mergeWithNext(e, c) } -> std::same_as<typename T::Map::Halfedge_handle>;
};

// Extension of the \ref ModifiableArrangement concept to one that allows reverting operations.
template <class T>
concept ModifiableArrangementWithHistory = requires(T t, int c, Number<Exact> n) {
	requires ModifiableArrangement<T>;

	{t.recallComplexity(c)};
	{t.recallThreshold(n)};
};

} // namespace cartocrow::simplification