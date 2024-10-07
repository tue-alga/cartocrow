#include "approximate_convex_hull.h"
#include "cs_curve_helpers.h"
#include "cs_polygon_helpers.h"

namespace cartocrow::simplesets {
Segment<Inexact> tangent(const Circle<Inexact>& c1, const Circle<Inexact>& c2) {
	auto distSq = CGAL::squared_distance(c1.center(), c2.center());
	auto hyp = c2.center() - c1.center();
	auto c1r = sqrt(c1.squared_radius());
	auto c2r = sqrt(c2.squared_radius());
	auto adj = c1r - c2r;
	auto a = hyp * adj;
	auto b = hyp.perpendicular(CGAL::POSITIVE) * sqrt(distSq - adj * adj);
	auto v1 = (a - b) / distSq;
	auto v2 = (a + b) / distSq;

	Segment<Inexact> t1(c1.center() + v1 * c1r, c2.center() + v1 * c2r);
	Segment<Inexact> t2(c1.center() + v2 * c1r, c2.center() + v2 * c2r);

	if (CGAL::orientation(t1.source(), t1.target(), c2.center()) == CGAL::COUNTERCLOCKWISE) {
		return t1;
	} else {
		return t2;
	}
}

std::pair<Point<Inexact>, Point<Inexact>>
tangentPoints(const Circle<Inexact>& c1, const Circle<Inexact>& c2) {
	auto distSq = CGAL::squared_distance(c1.center(), c2.center());
	auto hyp = c2.center() - c1.center();
	auto c1r = sqrt(c1.squared_radius());
	auto c2r = sqrt(c2.squared_radius());
	auto adj = c1r - c2r;
	auto a = hyp * adj;
	auto b = hyp.perpendicular(CGAL::POSITIVE) * sqrt(distSq - adj * adj);
	auto v1 = (a - b) / distSq;
	auto v2 = (a + b) / distSq;

	Segment<Inexact> t1(c1.center() + v1 * c1r, c2.center() + v1 * c2r);
	Segment<Inexact> t2(c1.center() + v2 * c1r, c2.center() + v2 * c2r);

	if (CGAL::orientation(t1.source(), t1.target(), c2.center()) == CGAL::COUNTERCLOCKWISE) {
		return {t1.source(), t1.target()};
	} else {
		return {t2.source(), t2.target()};
	}
}

std::pair<CSTraits::Point_2, CSTraits::Point_2>
tangentPoints(const RationalRadiusCircle& c1, const RationalRadiusCircle& c2) {
	Number<Exact> distSq = CGAL::squared_distance(c1.center, c2.center);
	Vector<Exact> hyp = c2.center - c1.center;
	Number<Exact> c1r = c1.radius;
	Number<Exact> c2r = c2.radius;
	Number<Exact> adj = c1r - c2r;
	Vector<Exact> a = hyp * adj;
	Vector<Exact> bV = hyp.perpendicular(CGAL::POSITIVE);
	Number<Exact> bSSqr = distSq - adj * adj;
	CSTraits::CoordNT bx(0, bV.x(), bSSqr);
	CSTraits::CoordNT by(0, bV.y(), bSSqr);

	CSTraits::CoordNT v1x = (a.x() - bx) / distSq;
	CSTraits::CoordNT v1y = (a.y() - by) / distSq;
	CSTraits::CoordNT v2x = (a.x() + bx) / distSq;
	CSTraits::CoordNT v2y = (a.y() + by) / distSq;

	CSTraits::CoordNT t1sx = c1.center.x() + v1x * c1r;
	CSTraits::CoordNT t1sy = c1.center.y() + v1y * c1r;
	CSTraits::Point_2 t1s(t1sx, t1sy);
	CSTraits::CoordNT t1tx = c2.center.x() + v1x * c2r;
	CSTraits::CoordNT t1ty = c2.center.y() + v1y * c2r;
	CSTraits::Point_2 t1t(t1tx, t1ty);

	CSTraits::CoordNT t2sx = c1.center.x() + v2x * c1r;
	CSTraits::CoordNT t2sy = c1.center.y() + v2y * c1r;
	CSTraits::Point_2 t2s(t2sx, t2sy);
	CSTraits::CoordNT t2tx = c2.center.x() + v2x * c2r;
	CSTraits::CoordNT t2ty = c2.center.y() + v2y * c2r;
	CSTraits::Point_2 t2t(t2tx, t2ty);
	CSTraits::Point_2 c2c(c2.center.x(), c2.center.y());

	return {t1s, t1t};
}

RationalRadiusCircle approximateRadiusCircle(const Circle<Exact>& circle) {
	Number<Inexact> r = sqrt(CGAL::to_double(circle.squared_radius()));
	Number<Exact> rExact = r;
	return {circle.center(), rExact};
}

// Adapted from https://github.com/CGAL/cgal/blob/38871d9b125c5513ff8d14f9562795aa12681b38/Minkowski_sum_2/include/CGAL/Minkowski_sum_2/Approx_offset_base_2.h
// This function falls under the following license:
//// Copyright (c) 2006-2008  Tel-Aviv University (Israel).
//// All rights reserved.
////
//// This function is part of CGAL (www.cgal.org).
////
//// $URL$
//// $Id$
//// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
////
//// Author(s)     : Ron Wein       <wein_r@yahoo.com>
////                 Andreas Fabri  <Andreas.Fabri@geometryfactory.com>
////                 Laurent Rineau <Laurent.Rineau@geometryfactory.com>
////                 Efi Fogel      <efif@post.tau.ac.il>
std::variant<Segment<Exact>, std::pair<Segment<Exact>, Segment<Exact>>>
algebraicCircleTangentToRationalSegments(const CSTraits::Point_2& p1, const CSTraits::Point_2& p2,
                                         const RationalRadiusCircle& c1, const RationalRadiusCircle& c2) {
	const auto& x1 = p1.x();
	const auto& y1 = p1.y();
	const auto& x2 = p2.x();
	const auto& y2 = p2.y();

	auto delta_x = x2 - x1;
	auto delta_y = y2 - y1;
	auto sqr_d = CGAL::square(delta_x) + CGAL::square(delta_y);
	// This is hacky
	Number<Exact> app_delta_x = CGAL::to_double(x2-x1);
	Number<Exact> app_delta_y = CGAL::to_double(y2-y1);
	Number<Exact> app_d = sqrt(CGAL::to_double(sqr_d));

	auto d_app_err = sqr_d - CGAL::square(app_d);
	auto dx_app_err = app_delta_x - delta_x;
	auto dy_app_err = app_delta_y - delta_y;
	auto sign_d_app_err = CGAL::sign(d_app_err);
	auto sign_dx_app_err = CGAL::sign(dx_app_err);
	auto sign_dy_app_err = CGAL::sign(dy_app_err);

	if (sign_d_app_err == CGAL::ZERO && sign_dx_app_err == CGAL::ZERO && sign_dy_app_err == CGAL::ZERO) {
		auto tp1 = Point<Exact> (c1.center.x() + c1.radius * app_delta_y / app_d, c1.center.y() + c1.radius * (-app_delta_x) / app_d);
		auto tp2 = Point<Exact> (c2.center.x() + c2.radius * app_delta_y / app_d, c2.center.y() + c2.radius * (-app_delta_x) / app_d);

		auto seg1 = Segment<Exact> (tp1, tp2);
		return seg1;
	} else {
		// This is hacky
		if (CGAL::sign(app_delta_x) == CGAL::ZERO) {
			app_delta_x += M_EPSILON;
		}
		if (CGAL::sign(app_delta_y) == CGAL::ZERO) {
			app_delta_y += M_EPSILON;
		}

		bool rotate_pi2 = false;
		if (CGAL::compare (CGAL::abs(delta_x),
		                   CGAL::abs(delta_y)) == CGAL::SMALLER)
		{
			rotate_pi2 = true;

			// Swap the delta_x and delta_y values.
			auto tmp_app = app_delta_x;
			app_delta_x = -app_delta_y;
			app_delta_y = tmp_app;
		}

		auto lower_tan_half_phi = (app_d - app_delta_y) / (-app_delta_x);
		auto upper_tan_half_phi = (-app_delta_x) / (app_d + app_delta_y);
		if (upper_tan_half_phi < lower_tan_half_phi) {
			auto temp = lower_tan_half_phi;
			lower_tan_half_phi = upper_tan_half_phi;
			upper_tan_half_phi = temp;
		}

		// This is hacky
		lower_tan_half_phi -= M_EPSILON;
		upper_tan_half_phi += M_EPSILON;

		auto sqr_tan_half_phi = CGAL::square (lower_tan_half_phi);
		auto sin_phi = 2 * lower_tan_half_phi / (1 + sqr_tan_half_phi);
		auto cos_phi = (1 - sqr_tan_half_phi) / (1 + sqr_tan_half_phi);

		Point<Exact> tp1;
		if (! rotate_pi2)
		{
			tp1 = Point<Exact> (c1.center.x() + c1.radius*cos_phi, c1.center.y() + c1.radius*sin_phi);
		}
		else
		{
			tp1 = Point<Exact> (c1.center.x() + c1.radius*sin_phi, c1.center.y() - c1.radius*cos_phi);
		}

		sqr_tan_half_phi = CGAL::square (upper_tan_half_phi);
		sin_phi = 2 * upper_tan_half_phi / (1 + sqr_tan_half_phi);
		cos_phi = (1 - sqr_tan_half_phi) / (1 + sqr_tan_half_phi);

		Point<Exact> tp2;
		if (! rotate_pi2)
		{
			tp2 = Point<Exact> (c2.center.x() + c2.radius*cos_phi, c2.center.y() + c2.radius*sin_phi);
		}
		else
		{
			tp2 = Point<Exact> (c2.center.x() + c2.radius*sin_phi, c2.center.y() - c2.radius*cos_phi);
		}

		auto l1 = Line<Exact>(c1.center, tp1).perpendicular(tp1);
		auto l2 = Line<Exact>(c2.center, tp2).perpendicular(tp2);

		// Intersect the two lines. The intersection point serves as a common
		// end point for the two line segments we are about to introduce.
		auto obj = CGAL::intersection(l1, l2);

		Point<Exact> mid_p;
		auto assign_success = CGAL::assign (mid_p, obj);
		assert(assign_success);

		auto seg1 = Segment<Exact> (tp1, mid_p);
		auto seg2 = Segment<Exact> (mid_p, tp2);

		return std::pair(seg1, seg2);
	}
}

std::variant<Segment<Exact>, std::pair<Segment<Exact>, Segment<Exact>>>
approximateTangent(const RationalRadiusCircle& c1, const RationalRadiusCircle& c2) {
    auto [source, target] = tangentPoints(c1, c2);
	return algebraicCircleTangentToRationalSegments(source, target, c1, c2);
}

std::vector<RationalRadiusCircle> circlesOnConvexHull(const std::vector<RationalRadiusCircle>& circles) {
	if (circles.size() == 1) {
		return {circles.front()};
	}

	Apollonius apo;
	std::unordered_map<Apollonius::Vertex_handle, RationalRadiusCircle> vToCircle;
	for (const auto& c : circles) {
		auto vh = apo.insert(ASite(c.center, c.radius));
		vToCircle[vh] = c;
	}
	if (apo.number_of_vertices() == 1) {
		auto site = apo.finite_vertices_begin()->site();
		return {{site.point(), site.weight()}};
	}
	auto circ = apo.incident_vertices(apo.infinite_vertex());
	auto curr = circ;
	std::vector<RationalRadiusCircle> hullCircles;
	do {
		hullCircles.push_back(vToCircle[curr]);
	} while (++curr != circ);

	std::reverse(hullCircles.begin(), hullCircles.end());

	return hullCircles;
}

/// Precondition: the circle centers are distinct.
CSPolygon approximateConvexHull(const std::vector<Circle<Exact>>& circles) {
	// todo: approximating circle radii may cause problems when two circles overlap in a single point and one is contained in the other.
	// solution? filter out any circle that is contained in another, before approximating the radii.
	if (circles.size() == 1) {
		return circleToCSPolygon(circles.front());
	}
	std::vector<RationalRadiusCircle> rrCircles;
	for (const auto& c : circles) {
		rrCircles.push_back(approximateRadiusCircle(c));
	}
	auto hullCircles = circlesOnConvexHull(rrCircles);
	if (hullCircles.size() == 1) {
		for (const auto& c : circles) {
			if (c.center() == hullCircles[0].center) {
				return circleToCSPolygon(c);
			}
		}
	}

	std::vector<std::vector<Segment<Exact>>> tangents;

	for (int i = 0; i < hullCircles.size(); ++i) {
		auto& c1 = hullCircles[i];
		auto& c2 = hullCircles[(i + 1) % hullCircles.size()];
		auto segOrPair = approximateTangent(c1, c2);
		std::vector<Segment<Exact>> segs;
		if (segOrPair.index() == 0) {
			segs.push_back(std::get<Segment<Exact>>(segOrPair));
		} else {
			auto pair = std::get<std::pair<Segment<Exact>, Segment<Exact>>>(segOrPair);
			segs.push_back(pair.first);
			segs.push_back(pair.second);
		}
		tangents.push_back(segs);
	}

	std::vector<X_monotone_curve_2> xm_curves;
	for (int i = 0; i < hullCircles.size(); ++i) {
		auto& c1 = hullCircles[i];
		auto& c2 = hullCircles[(i + 1) % hullCircles.size()];
		auto& t1 = tangents[i];
		auto& t2 = tangents[(i + 1) % tangents.size()];
		for (const auto& piece : t1) {
			Curve_2 curve(piece);
			curveToXMonotoneCurves(curve, std::back_inserter(xm_curves));
		}
		OneRootPoint t1End(t1.back().target().x(), t1.back().target().y());
		OneRootPoint t2Start(t2.front().source().x(), t2.front().source().y());
		Curve_2 arc(Circle<Exact>(c2.center, c2.radius * c2.radius), t1End, t2Start);
		curveToXMonotoneCurves(arc, std::back_inserter(xm_curves));
	}

	return {xm_curves.begin(), xm_curves.end()};
}
}