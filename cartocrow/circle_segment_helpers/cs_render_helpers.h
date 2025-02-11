#ifndef CARTOCROW_CS_RENDER_HELPERS_H
#define CARTOCROW_CS_RENDER_HELPERS_H

#include "cartocrow/circle_segment_helpers/cs_types.h"
#include "cartocrow/renderer/render_path.h"

namespace cartocrow::renderer {
renderer::RenderPath renderPath(const CSXMCurve& xmCurve);
renderer::RenderPath renderPath(const CSCurve& curve);
renderer::RenderPath renderPath(const CSPolygon& polygon);
renderer::RenderPath renderPath(const CSPolygonWithHoles& withHoles);
renderer::RenderPath renderPath(const CSPolygonSet& polygonSet);
renderer::RenderPath renderPath(const CSPolyline& polyline);
void addToRenderPath(const CSXMCurve& xm_curve, renderer::RenderPath& path, bool first);
void addToRenderPath(const CSCurve& curve, renderer::RenderPath& path, bool first);
}

#endif //CARTOCROW_CS_RENDER_HELPERS_H
