#ifndef CARTOCROW_CS_RENDER_HELPERS_H
#define CARTOCROW_CS_RENDER_HELPERS_H

#include "../core/cs_types.h"
#include "render_path.h"

namespace cartocrow::renderer {
renderer::RenderPath renderPath(const X_monotone_curve_2& xm_curve);
renderer::RenderPath renderPath(const CSPolygon& polygon);
renderer::RenderPath renderPath(const CSPolygonWithHoles& withHoles);
renderer::RenderPath renderPath(const CSPolygonSet& polygonSet);
renderer::RenderPath renderPath(const CSPolyline& polyline);
void addToRenderPath(const X_monotone_curve_2& xm_curve, renderer::RenderPath& path, bool& first);
void addToRenderPath(const Curve_2& curve, renderer::RenderPath& path, bool& first);
}

#endif //CARTOCROW_CS_RENDER_HELPERS_H
