#pragma once

#include "../core/core.h"

namespace cartocrow::simplification {

template <class T> concept MapType = requires {
	typename T::Map;
};

template <class T>
concept ModifiableArrangement = requires(T t, typename T::Map::Halfedge_handle e, Number<Exact> c) {

	requires MapType<T>;

	{ t.getMap() } -> std::same_as<typename T::Map&>;
	{ t.mergeWithNext(e, c) } -> std::same_as<typename T::Map::Halfedge_handle>;
};

template <class T>
concept ModifiableArrangementWithHistory = requires(T t, int c) {
	requires ModifiableArrangement<T>;

	{t.recallComplexity(c)};
};

} // namespace cartocrow::simplification