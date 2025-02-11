#include "cs_curve_helpers.h"
#include "cartocrow/core/vector_helpers.h"

namespace cartocrow {
OneRootPoint closestOnCircle(const Circle<Exact>& circle, const Point<Exact>& point) {
	auto bb = circle.bbox();
	Rectangle<Exact> bbX(bb.xmin() - 1, bb.ymin() - 1, bb.xmax() + 1, bb.ymax() + 1);
	Ray<Exact> ray(circle.center(), point);
	auto inter = CGAL::intersection(bbX, ray);
	auto seg = get<Segment<Exact>>(*inter);

	using Arr = CGAL::Arrangement_2<ArrCSTraits>;
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

OneRootPoint nearest(const CSXMCurve& xm_curve, const Point<Exact>& point) {
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
			auto sdLeft = CGAL::square(point.x() - xm_curve.left().x()) +
			              CGAL::square(point.y() - xm_curve.left().y());
			auto sdRight = CGAL::square(point.x() - xm_curve.right().x()) +
			               CGAL::square(point.y() - xm_curve.right().y());
			if (sdLeft < sdRight) {
				return xm_curve.left();
			} else {
				return xm_curve.right();
			}
		}
	}
}

bool liesOn(const Point<Exact>& p, const CSXMCurve& xm_curve) {
	if (p.x() < xm_curve.left().x() || p.x() > xm_curve.right().x())
		return false;

	if (xm_curve.is_linear()) {
		return xm_curve.supporting_line().has_on(p);
	} else {
		return xm_curve.point_position({p.x(), p.y()}) == CGAL::EQUAL;
	}
}

bool liesOn(const OneRootPoint& p, const CSXMCurve& xm_curve) {
	if (p.x() < xm_curve.left().x() || p.x() > xm_curve.right().x())
		return false;
	return xm_curve.point_position(p) == CGAL::EQUAL;
}

bool liesOn(const CSXMCurve& c1, const CSXMCurve& c2) {
	auto sc = liesOn(c1.source(), c2);
	auto tc = liesOn(c1.target(), c2);
	if (!sc || !tc) {
		return false;
	}
	if (c2.is_linear()) {
		if (c1.is_circular())
			return false;
		if (c1.supporting_line() != c2.supporting_line())
			return false;
	} else {
		if (c1.is_linear())
			return false;
		if (c1.supporting_circle() != c2.supporting_circle())
			return false;
	}

	return true;
}

CSCurve toCurve(const CSXMCurve& xmc) {
	if (xmc.is_linear()) {
		return {xmc.supporting_line(), xmc.source(), xmc.target()};
	} else if (xmc.is_circular()) {
		return {xmc.supporting_circle(), xmc.source(), xmc.target()};
	} else {
		throw std::runtime_error(
		    "Impossible: circle-segment x-monotone curve is neither linear nor circular.");
	}
}

Vector<Inexact> startTangent(const CSXMCurve& c) {
	if (c.is_linear()) {
		auto l = c.supporting_line();
		auto v = approximate(l.to_vector());
		auto len = sqrt(v.squared_length());
		return v / len;
	} else {
		auto circle = c.supporting_circle();
		auto v = approximate(approximateOneRootPoint(c.source())) - approximate(circle.center());
		auto p = v.perpendicular(circle.orientation());
		auto len = sqrt(p.squared_length());
		return p / len;
	}
}

Vector<Inexact> endTangent(const CSXMCurve& c) {
	if (c.is_linear()) {
		auto l = c.supporting_line();
		auto v = approximate(l.to_vector());
		auto len = sqrt(v.squared_length());
		return v / len;
	} else {
		auto circle = c.supporting_circle();
		auto v = approximate(approximateOneRootPoint(c.target())) - approximate(circle.center());
		auto p = v.perpendicular(circle.orientation());
		auto len = sqrt(p.squared_length());
		return p / len;
	}
}

double approximateTurningAngle(const CSXMCurve& xmc) {
	if (xmc.is_linear()) {
		return 0.0;
	}

	auto c = xmc.supporting_circle();
	auto acc = approximate(c.center());
	auto v1 = approximateOneRootPoint(xmc.source()) - acc;
	auto v2 = approximateOneRootPoint(xmc.target()) - acc;

	return orientedAngleBetween(v1, v2, xmc.orientation());
}

double approximateLength(const CSXMCurve& xmc) {
    auto s = approximateOneRootPoint(xmc.source());
    auto t = approximateOneRootPoint(xmc.target());
    if (xmc.is_linear()) {
        return CGAL::sqrt(CGAL::squared_distance(s, t));
    } else {
        auto c = xmc.supporting_circle();
        auto acc = approximate(c.center());
        auto angle = orientedAngleBetween(s - acc, t - acc, xmc.orientation());
        return angle * CGAL::sqrt(CGAL::to_double(c.squared_radius()));
    }
}

double approximateLength(const CSCurve& c) {
    auto s = approximateOneRootPoint(c.source());
    auto t = approximateOneRootPoint(c.target());
    if (c.is_linear()) {
        return CGAL::sqrt(CGAL::squared_distance(s, t));
    } else {
        const auto& circ = c.supporting_circle();
        auto r = CGAL::sqrt(CGAL::to_double(circ.squared_radius()));
        if (c.is_full()) {
            return M_2xPI * r;
        }
        auto acc = approximate(circ.center());
        auto angle = orientedAngleBetween(s - acc, t - acc, c.orientation());
        return angle * r;
    }
}
}