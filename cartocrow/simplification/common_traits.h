#pragma once

#include "../core/core.h"

namespace cartocrow::simplification {

template <class T> concept BaseSimplificationTraits = requires {

	typename T::Map;
};

} // namespace cartocrow::simplification