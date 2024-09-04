#include "cs_curve_helpers.h"

namespace cartocrow::simplesets {
OneRootPoint closestOnCircle(const Circle<Exact>& circle, const Point<Exact>& point) {
	auto bb = circle.bbox();
	Rectangle<Exact> bbX(bb.xmin() - 1, bb.ymin() - 1, bb.xmax() + 1, bb.ymax() + 1);
	Ray<Exact> ray(circle.center(), point);
	auto inter = CGAL::intersection(bbX, ray);
	auto seg = get<Segment<Exact>>(*inter);

	using Arr = CGAL::Arrangement_2<CSTraits>;
	Arr arr;
	CGAL::insert(arr, circle);
	CGAL::insert(arr, seg);

	std::optional<OneRootPoint> intersectionPoint;
	for (auto vit = arr.vertices_begin(); vit != arr.vertices_end(); ++vit) {
		if (vit->degree() == 4) {
			intersectionPoint = vit->point();
		}
	}

	if (!intersectionPoint.has_value()) {
		std::stringstream ss;
		ss << "Could not project point " << point << " on circle " << circle;
		throw std::runtime_error(ss.str());
	}

	return *intersectionPoint;
}

OneRootPoint nearest(const X_monotone_curve_2& xm_curve, const Point<Exact>& point) {
	if (xm_curve.is_linear()) {
		auto l = xm_curve.supporting_line();
		auto k = l.perpendicular(point);
		auto inter = get<Point<Exact>>((*CGAL::intersection(l, k)));

		if (xm_curve.is_vertical()) {
			auto minY = std::min(xm_curve.left().y(), xm_curve.right().y());
			auto maxY = std::max(xm_curve.left().y(), xm_curve.right().y());
			auto x = xm_curve.left().x();
			if (inter.y() >= maxY) {
				return {x, maxY};
			} else if (inter.y() <= minY) {
				return {x, minY};
			} else {
				return {inter.x(), inter.y()};
			}
		}

		if (inter.x() <= xm_curve.left().x()) {
			return xm_curve.left();
		} else if (inter.x() >= xm_curve.right().x()) {
			return xm_curve.right();
		} else {
			return {inter.x(), inter.y()};
		}
	} else {
		auto c = xm_curve.supporting_circle();
		xm_curve.point_position({point.x(), point.y()});
		auto inter = closestOnCircle(c, point);
		if (inter.x() <= xm_curve.left().x()) {
			return xm_curve.left();
		} else if (inter.x() >= xm_curve.right().x()) {
			return xm_curve.right();
		} else {
			if (liesOn(inter, xm_curve)) {
				return inter;
			}
			OneRootPoint opposite(2 * c.center().x() - inter.x(), 2 * c.center().y() - inter.y());
			if (liesOn(opposite, xm_curve)) {
				return opposite;
			}
			auto sdLeft = CGAL::square(point.x() - xm_curve.left().x()) + CGAL::square(point.y() - xm_curve.left().y());
			auto sdRight = CGAL::square(point.x() - xm_curve.right().x()) + CGAL::square(point.y() - xm_curve.right().y());
			if (sdLeft < sdRight) {
				return xm_curve.left();
			} else {
				return xm_curve.right();
			}
		}
	}
}

bool liesOn(const Point<Exact>& p, const X_monotone_curve_2& xm_curve) {
	if (p.x() < xm_curve.left().x() || p.x() > xm_curve.right().x())
		return false;

	if (xm_curve.is_linear()) {
		return xm_curve.supporting_line().has_on(p);
	} else {
		return xm_curve.point_position({p.x(), p.y()}) == CGAL::EQUAL;
	}
}

bool liesOn(const OneRootPoint& p, const X_monotone_curve_2& xm_curve) {
	if (p.x() < xm_curve.left().x() || p.x() > xm_curve.right().x())
		return false;
//	CSTraits traits;
//	auto lies_on = traits.compare_y_at_x_2_object();
	return xm_curve.point_position(p) == CGAL::EQUAL;
}

renderer::RenderPath renderPathFromXMCurve(const X_monotone_curve_2& xm_curve) {
	renderer::RenderPath path;
	path.moveTo(approximateAlgebraic(xm_curve.source()));

	if (xm_curve.is_circular()) {
		auto circle = xm_curve.supporting_circle();
		path.arcTo(approximate(circle.center()), xm_curve.orientation() == CGAL::CLOCKWISE,
		           approximateAlgebraic(xm_curve.target()));
	} else {
		auto line = xm_curve.supporting_line();
		path.lineTo(approximateAlgebraic(xm_curve.target()));
	}

	return path;
}

void addCurveToRenderPath(const X_monotone_curve_2& xm_curve, renderer::RenderPath& path, bool& first) {
	auto as = approximateAlgebraic(xm_curve.source());
	auto at = approximateAlgebraic(xm_curve.target());
	if (first) {
		path.moveTo(as);
		first = false;
	}
	if (xm_curve.is_linear()) {
		path.lineTo(at);
	} else if (xm_curve.is_circular()){
		if (CGAL::squared_distance(as, at) < M_EPSILON) {
			return;
		}
		auto circle = xm_curve.supporting_circle();
		path.arcTo(approximate(circle.center()), xm_curve.orientation() == CGAL::CLOCKWISE, approximateAlgebraic(xm_curve.target()));
	}
}
}