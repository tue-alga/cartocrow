#ifndef CARTOCROW_CS_POLYGONSET_HELPERS_H
#define CARTOCROW_CS_POLYGONSET_HELPERS_H

#include "../types.h"
#include "cartocrow/renderer/render_path.h"
#include <CGAL/Arr_polycurve_traits_2.h>

namespace cartocrow::simplesets {
renderer::RenderPath renderPath(const CSPolygonSet& polygonSet);
}

#endif //CARTOCROW_CS_POLYGONSET_HELPERS_H
