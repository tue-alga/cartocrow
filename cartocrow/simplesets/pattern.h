#ifndef CARTOCROW_PATTERN_H
#define CARTOCROW_PATTERN_H

#include "../core/core.h"
#include "../core/polyline.h"
#include <variant>
#include "types.h"
#include "cat_point.h"
#include "settings.h"

namespace cartocrow::simplesets {
class Pattern {
  public:
	virtual std::variant<Polyline<K>, Polygon<K>> contour() = 0;
	virtual std::vector<CatPoint> catPoints() = 0;
	virtual Number<K> coverRadius() = 0;
	virtual bool isValid(GeneralSettings gs) = 0;
};
}

#endif //CARTOCROW_PATTERN_H
