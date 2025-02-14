#include "cs_render_helpers.h"
#include "cartocrow/circle_segment_helpers/cs_curve_helpers.h"

namespace cartocrow::renderer {
void addToRenderPath(const CSXMCurve& xm_curve, renderer::RenderPath& path, bool first) {
	auto as = approximateOneRootPoint(xm_curve.source());
	auto at = approximateOneRootPoint(xm_curve.target());
	if (first) {
		path.moveTo(as);
	}
	if (xm_curve.is_linear()) {
		path.lineTo(at);
	} else if (xm_curve.is_circular()) {
		if (CGAL::squared_distance(as, at) < M_EPSILON) {
			return;
		}
		const auto& circle = xm_curve.supporting_circle();
		path.arcTo(approximate(circle.center()), xm_curve.orientation() == CGAL::CLOCKWISE,
                   approximateOneRootPoint(xm_curve.target()));
	}
}

void addToRenderPath(const CSCurve& curve, renderer::RenderPath& path, bool first) {
	if (curve.is_full()) {
		const auto& circ = curve.supporting_circle();
		const auto& cent = approximate(circ.center());
		auto r = sqrt(CGAL::to_double(circ.squared_radius()));
		Point<Inexact> s(cent.x() - r, cent.y());
		auto clockwise = circ.orientation() == CGAL::CLOCKWISE;
		path.moveTo(s);
		path.arcTo(cent, clockwise, {cent.x() + r, cent.y()});
		path.arcTo(cent, clockwise, s);
		path.close();
		return;
	}
	auto as = approximateOneRootPoint(curve.source());
	auto at = approximateOneRootPoint(curve.target());
	if (first) {
		path.moveTo(as);
	}
	if (curve.is_linear()) {
		path.lineTo(at);
	} else if (curve.is_circular()) {
		if (CGAL::squared_distance(as, at) < M_EPSILON) {
			return;
		}
		const auto& circle = curve.supporting_circle();
		path.arcTo(approximate(circle.center()), curve.orientation() == CGAL::CLOCKWISE,
                   approximateOneRootPoint(curve.target()));
	}
}

renderer::RenderPath operator<<(renderer::RenderPath& path, const CSPolygon& polygon) {
	bool first = true;
	std::vector<CSCurve> mergedCurves;
	toCurves(polygon.curves_begin(), polygon.curves_end(), std::back_inserter(mergedCurves));
	for (const auto& c : mergedCurves) {
		addToRenderPath(c, path, first);
        first = false;
	}
	if (!holds_alternative<renderer::RenderPath::Close>(path.commands().back())) {
		path.close();
	}
	return path;
}

renderer::RenderPath renderPath(const CSXMCurve& xmCurve) {
    renderer::RenderPath path;
    addToRenderPath(xmCurve, path, true);
    return path;
}

renderer::RenderPath renderPath(const CSCurve& curve) {
    renderer::RenderPath path;
    addToRenderPath(curve, path, true);
    return path;
}

renderer::RenderPath renderPath(const CSPolygon& polygon) {
	renderer::RenderPath path;
	path << polygon;
	return path;
}

renderer::RenderPath renderPath(const CSPolygonWithHoles& withHoles) {
	renderer::RenderPath path;
	path << withHoles.outer_boundary();
	for (auto hit = withHoles.holes_begin(); hit != withHoles.holes_end(); ++hit) {
		path << *hit;
	}
	return path;
}

renderer::RenderPath renderPath(const CSPolygonSet& polygonSet) {
	std::vector<CSPolygonWithHoles> withHoles;
	polygonSet.polygons_with_holes(std::back_inserter(withHoles));

	renderer::RenderPath path;
	for (const auto& h : withHoles) {
		path += renderPath(h);
	}

	return path;
}

renderer::RenderPath renderPath(const CSPolyline& polyline) {
	renderer::RenderPath path;
	bool first = true;
	for (auto cit = polyline.curves_begin(); cit != polyline.curves_end(); ++cit) {
		addToRenderPath(*cit, path, first);
        first = false;
	}
	return path;
}
}
