#ifndef CARTOCROW_TRANSFORM_HELPERS_H
#define CARTOCROW_TRANSFORM_HELPERS_H

#include "core.h"

namespace cartocrow {
PolygonWithHoles<Inexact> transform(const CGAL::Aff_transformation_2 <Inexact> &t, const PolygonWithHoles <Inexact> &pwh);
CGAL::Aff_transformation_2<Inexact> fitInto(const Rectangle<Inexact>& toFit, const Rectangle<Inexact>& into);
}

#endif //CARTOCROW_TRANSFORM_HELPERS_H
