#include "cs_polygon_helpers.h"
#include "cs_curve_helpers.h"
#include <CGAL/approximated_offset_2.h>
#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/Boolean_set_operations_2/Gps_polygon_validation.h>
#include <CGAL/Surface_sweep_2.h>
#include <CGAL/Surface_sweep_2/Default_visitor.h>

// All area functions in this file are adapted from a Stack Overflow answer by HEKTO.
// Link: https://stackoverflow.com/questions/69399922/how-does-one-obtain-the-area-of-a-general-polygon-set-in-cgal
// License info: https://stackoverflow.com/help/licensing
// The only changes made were changing the auto return type to Number<Inexact> and using the
// typedefs for circle, point, polygon etc.

namespace cartocrow {
Number<Inexact> area(const ArrCSTraits::Point_2& P1, const ArrCSTraits::Point_2& P2) {
	auto const dx = CGAL::to_double(P1.x()) - CGAL::to_double(P2.x());
	auto const sy = CGAL::to_double(P1.y()) + CGAL::to_double(P2.y());
	return dx * sy / 2;
}

Number<Inexact> area(const ArrCSTraits::Point_2& P1, const ArrCSTraits::Point_2& P2, const ArrCSTraits::Rational_circle_2& C) {
	auto const dx = CGAL::to_double(P1.x()) - CGAL::to_double(P2.x());
	auto const dy = CGAL::to_double(P1.y()) - CGAL::to_double(P2.y());
	auto const squaredChord = dx * dx + dy * dy;
	auto const chord = std::sqrt(squaredChord);
	auto const squaredRadius = CGAL::to_double(C.squared_radius());
	auto const areaSector =
	    squaredRadius * std::asin(std::min(1.0, chord / (std::sqrt(squaredRadius) * 2)));
	auto const areaTriangle = chord * std::sqrt(std::max(0.0, squaredRadius * 4 - squaredChord)) / 4;
	auto const areaCircularSegment = areaSector - areaTriangle;
	int sign;
	if (C.orientation() == CGAL::Sign::NEGATIVE) {
		sign = -1;
	} else if (C.orientation() == CGAL::Sign::POSITIVE) {
		sign = 1;
	} else {
		assert(C.orientation() == CGAL::Sign::ZERO);
		sign = 0;
	}
	return area(P1, P2) + sign * areaCircularSegment;
}

Number<Inexact> area(const CSXMCurve& XCV) {
	if (XCV.is_linear()) {
		return area(XCV.source(), XCV.target());
	} else if (XCV.is_circular()) {
		return area(XCV.source(), XCV.target(), XCV.supporting_circle());
	} else {
		return 0;
	}
}

Number<Inexact> area(const CSPolygon& P) {
	Number<Inexact> res = 0;
	for (auto it = P.curves_begin(); it != P.curves_end(); ++it) {
		res += area(*it);
	}
	return res;
}

Number<Inexact> area(const CSPolygonWithHoles& P) {
	auto res = area(P.outer_boundary());
	for (auto it = P.holes_begin(); it != P.holes_end(); ++it) {
		res += area(*it);
	}
	return res;
}

CSPolygon circleToCSPolygon(const Circle<Exact>& circle) {
	std::vector<CSXMCurve> xm_curves;
	curveToXMonotoneCurves(circle, std::back_inserter(xm_curves));
	return {xm_curves.begin(), xm_curves.end()};
}

std::optional<CSPolygon::Curve_const_iterator> liesOn(const Point<Exact>& p, const CSPolygon& polygon) {
	for (auto cit = polygon.curves_begin(); cit != polygon.curves_end(); ++cit) {
		if (liesOn(p, *cit)) {
			return cit;
		}
	}
	return std::nullopt;
}

std::optional<CSPolygon::Curve_const_iterator> liesOn(const OneRootPoint& p, const CSPolygon& polygon) {
	for (auto cit = polygon.curves_begin(); cit != polygon.curves_end(); ++cit) {
		if (liesOn(p, *cit)) {
			return cit;
		}
	}
	return std::nullopt;
}

bool onOrInside(const CSPolygon& polygon, const Point<Exact>& point) {
	Ray<Exact> ray(point, Vector<Exact>(1, 0));

	Rectangle<Exact> bbox = polygon.bbox();
	Rectangle<Exact> rect({bbox.xmin() - 1, bbox.ymin() - 1}, {bbox.xmax() + 1, bbox.ymax() + 1});

	auto inter = CGAL::intersection(ray, rect);
	if (!inter.has_value()) return false;
	if (inter->type() == typeid(Point<Exact>)) return true;
	auto seg = boost::get<Segment<Exact>>(*inter);
	CSXMCurve seg_xm_curve(seg.source(), seg.target());

	typedef std::pair<ArrCSTraits::Point_2, unsigned int> Intersection_point;
	typedef boost::variant<Intersection_point, CSXMCurve> Intersection_result;
	std::vector<Intersection_result> intersection_results;

	for (auto cit = polygon.curves_begin(); cit != polygon.curves_end(); ++cit) {
		const auto& curve = *cit;
		curve.intersect(seg_xm_curve, std::back_inserter(intersection_results));
	}

	int count = 0;
	for (const auto& ir : intersection_results) {
		if (ir.which() == 0) {
			auto ip = get<Intersection_point>(ir);
			//(ir) Intersection points are double-counted, so increase count by half.
			bool found = false;
			for (auto cit = polygon.curves_begin(); cit != polygon.curves_end(); ++cit) {
				if (cit->source() == ip.first) {
					found = true;
					break;
				}
			}
			if (found) {
				count += 1;
				continue;
			}
		}
		count += 2;
	}

	return count % 4 != 0;
}

bool liesOn(const CSXMCurve& c, const CSPolygon& polygon) {
	auto sc = liesOn(c.source(), polygon);
	auto tc = liesOn(c.target(), polygon);
	if (!sc.has_value() || !tc.has_value()) {
		return false;
	}
	do {
		auto next = *sc;
		++next;
		if (next == polygon.curves_end()) {
			next = polygon.curves_begin();
		}
		if (liesOn(c.source(), *next)) {
			sc = next;
		} else {
			break;
		}
	} while (true);
	do {
		auto next = *tc;
		++next;
		if (next == polygon.curves_end()) {
			next = polygon.curves_begin();
		}
		if (liesOn(c.source(), *next)) {
			tc = next;
		} else {
			break;
		}
	} while (true);
	auto sit = *sc;
	auto tit = *sc;
	auto curr = sit;
	do {
		if (curr->is_linear()) {
			if (c.is_circular()) return false;
			if (curr->supporting_line() != c.supporting_line()) return false;
		} else {
			if (c.is_linear()) return false;
			if (curr->supporting_circle() != c.supporting_circle()) return false;
		}
	} while (curr++ != tit);

	return true;
}

bool inside(const CSPolygon& polygon, const Point<Exact>& point) {
    return onOrInside(polygon, point) && !liesOn(point, polygon);
}

bool outside(const CSPolygon& polygon, const Point<Exact>& point) {
    return !onOrInside(polygon, point);
}

bool onOrOutside(const CSPolygon& polygon, const Point<Exact>& point) {
    return liesOn(point, polygon) || outside(polygon, point);
}

CGAL::Bounded_side bounded_side(const CSPolygon& polygon, const Point<Exact>& point) {
    if (liesOn(point, polygon)) return CGAL::ON_BOUNDARY;
    return onOrInside(polygon, point) ? CGAL::ON_BOUNDED_SIDE : CGAL::ON_UNBOUNDED_SIDE;
}

CSPolycurve arrPolycurveFromCSPolygon(const CSPolygon& polygon) {
	return arrPolycurveFromXMCurves(polygon.curves_begin(), polygon.curves_end());
}

bool is_simple(const CSPolygon& pgn) {
	typedef typename GpsCSTraits::Curve_const_iterator       Curve_const_iterator;
	typedef std::pair<Curve_const_iterator,Curve_const_iterator>
	    Cci_pair;

	// Sweep the boundary curves and look for intersections.
	typedef CGAL::Gps_polygon_validation_visitor<GpsCSTraits>      Visitor;
	typedef CGAL::Ss2::Surface_sweep_2<Visitor>                 Surface_sweep;

	GpsCSTraits traits;

	Cci_pair itr_pair = traits.construct_curves_2_object()(pgn);
	Visitor visitor;
	Surface_sweep surface_sweep(&traits, &visitor);

	visitor.sweep(itr_pair.first, itr_pair.second);
	return visitor.is_valid();
}

CSPolygon polygonToCSPolygon(const Polygon<Exact>& polygon) {
	std::vector<CSXMCurve> xmCurves;
	for (auto eit = polygon.edges_begin(); eit != polygon.edges_end(); ++eit) {
		CSXMCurve xmCurve(eit->source(), eit->target());
		xmCurves.push_back(xmCurve);
	}
	return {xmCurves.begin(), xmCurves.end()};
}

CSPolygonWithHoles polygonToCSPolygon(const PolygonWithHoles<Exact>& polygon) {
	std::vector<CSPolygon> holes;
	for (auto hit = polygon.holes_begin(); hit != polygon.holes_end(); ++hit) {
		holes.push_back(polygonToCSPolygon(*hit));
	}
	return {polygonToCSPolygon(polygon.outer_boundary()), holes.begin(), holes.end()};
}
}