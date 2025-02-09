#include "circle_tangents.h"

namespace cartocrow {
std::optional<std::pair<Segment<Inexact>, Segment<Inexact>>>
tangents(const Point<Inexact>& p, const Circle<Inexact>& c) {
	auto distSq = CGAL::squared_distance(p, c.center());
	if (distSq <= c.squared_radius()) return std::nullopt;
	auto hyp = c.center() - p;
	auto cr = sqrt(c.squared_radius());
	auto a = -cr * hyp;
	auto b = hyp.perpendicular(CGAL::POSITIVE) * sqrt(distSq - cr * cr);
	auto v1 = (a - b) / distSq;
	auto v2 = (a + b) / distSq;

	Segment<Inexact> t1(p, c.center() + v1 * cr);
	Segment<Inexact> t2(p, c.center() + v2 * cr);

	return std::pair(t1, t2);
}

std::optional<std::pair<OneRootPoint, OneRootPoint>>
tangentPoints(const Point<Exact>& p, const RationalRadiusCircle& c) {
	auto distSq = CGAL::squared_distance(p, c.center);
	if (distSq <= CGAL::square(c.radius)) return std::nullopt;
	auto hyp = c.center - p;
	auto cr = c.radius;
	auto a = -cr * hyp;
	auto bV = hyp.perpendicular(CGAL::POSITIVE);
	auto bSqrt = distSq - cr * cr;
	ArrCSTraits::CoordNT bx(0, bV.x(), bSqrt);
	ArrCSTraits::CoordNT by(0, bV.y(), bSqrt);
	ArrCSTraits::CoordNT v1x = (a.x() - bx) / distSq;
	ArrCSTraits::CoordNT v1y = (a.y() - by) / distSq;
	ArrCSTraits::CoordNT v2x = (a.x() + bx) / distSq;
	ArrCSTraits::CoordNT v2y = (a.y() + by) / distSq;

	OneRootPoint t1(c.center.x() + v1x * cr, c.center.y() + v1y * cr);
	OneRootPoint t2(c.center.x() + v2x * cr, c.center.y() + v2y * cr);

	return std::pair(t1, t2);
}

std::optional<std::pair<Segment<Inexact>, Segment<Inexact>>>
tangents(const Circle<Inexact>& c, const Point<Inexact>& p) {
	auto ts = tangents(p, c);
	if (!ts.has_value()) {
		return std::nullopt;
	} else {
		auto [t1, t2] = *ts;
		return std::pair(t2.opposite(), t1.opposite());
	}
}

std::optional<std::pair<OneRootPoint, OneRootPoint>>
tangentPoints(const RationalRadiusCircle& c, const Point<Exact>& p) {
	auto ts = tangentPoints(p, c);
	if (!ts.has_value()) {
		return std::nullopt;
	} else {
		auto [t1, t2] = *ts;
		return std::pair(t2, t1);
	}
}

std::optional<std::pair<Segment<Inexact>, Segment<Inexact>>>
bitangents(const Circle<Inexact>& c1, const Circle<Inexact>& c2, bool inner) {
    auto distSq = CGAL::squared_distance(c1.center(), c2.center());

    auto c1r = sqrt(c1.squared_radius());
    auto c2r = sqrt(c2.squared_radius());

    if (inner) {
        if (distSq <= std::pow((c1r + c2r), 2)) {
            return std::nullopt;
        }
    } else {
        auto rDiff = c1r - c2r;
        if (distSq <= rDiff * rDiff) {
            return std::nullopt;
        }
    }

    auto c2rSigned = inner ? -c2r : c2r;
    auto hyp = c2.center() - c1.center();
    auto adj = c1r - c2rSigned;
    auto a = hyp * adj;
    auto b = hyp.perpendicular(CGAL::POSITIVE) * sqrt(distSq - adj * adj);
    auto v1 = (a - b) / distSq;
    auto v2 = (a + b) / distSq;

    Segment<Inexact> t1(c1.center() + v1 * c1r, c2.center() + v1 * c2rSigned);
    Segment<Inexact> t2(c1.center() + v2 * c1r, c2.center() + v2 * c2rSigned);

    return std::pair(t1, t2);
}

std::optional<std::pair<std::pair<OneRootPoint, OneRootPoint>,std::pair<OneRootPoint, OneRootPoint>>>
bitangentPoints(const RationalRadiusCircle& c1, const RationalRadiusCircle& c2, bool inner) {
    Number<Exact> distSq = CGAL::squared_distance(c1.center, c2.center);
    Number<Exact> c1r = c1.radius;
    Number<Exact> c2r = c2.radius;

    if (inner) {
        if (distSq <= CGAL::square(c1r + c2r)) {
            return std::nullopt;
        }
    } else {
        auto rDiff = c1r - c2r;
        if (distSq <= rDiff * rDiff) {
            return std::nullopt;
        }
    }

    auto c2rSigned = inner ? -c2r : c2r;

    Vector<Exact> hyp = c2.center - c1.center;
    Number<Exact> adj = c1r - c2rSigned;
    Vector<Exact> a = hyp * adj;
    Vector<Exact> bV = hyp.perpendicular(CGAL::POSITIVE);
    Number<Exact> bSSqr = distSq - adj * adj;
    ArrCSTraits::CoordNT bx(0, bV.x(), bSSqr);
    ArrCSTraits::CoordNT by(0, bV.y(), bSSqr);

    ArrCSTraits::CoordNT v1x = (a.x() - bx) / distSq;
    ArrCSTraits::CoordNT v1y = (a.y() - by) / distSq;
    ArrCSTraits::CoordNT v2x = (a.x() + bx) / distSq;
    ArrCSTraits::CoordNT v2y = (a.y() + by) / distSq;

    ArrCSTraits::CoordNT t1sx = c1.center.x() + v1x * c1r;
    ArrCSTraits::CoordNT t1sy = c1.center.y() + v1y * c1r;
    ArrCSTraits::Point_2 t1s(t1sx, t1sy);
    ArrCSTraits::CoordNT t1tx = c2.center.x() + v1x * c2rSigned;
    ArrCSTraits::CoordNT t1ty = c2.center.y() + v1y * c2rSigned;
    ArrCSTraits::Point_2 t1t(t1tx, t1ty);

    ArrCSTraits::CoordNT t2sx = c1.center.x() + v2x * c1r;
    ArrCSTraits::CoordNT t2sy = c1.center.y() + v2y * c1r;
    ArrCSTraits::Point_2 t2s(t2sx, t2sy);
    ArrCSTraits::CoordNT t2tx = c2.center.x() + v2x * c2rSigned;
    ArrCSTraits::CoordNT t2ty = c2.center.y() + v2y * c2rSigned;
    ArrCSTraits::Point_2 t2t(t2tx, t2ty);

    return std::pair(std::pair(t1s, t1t), std::pair(t2s, t2t));
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
RationalTangent algebraicCircleBitangentToRationalSegments(const OneRootPoint& p1, const OneRootPoint& p2,
                                         const RationalRadiusCircle& c1, const RationalRadiusCircle& c2,
                                         bool flipTp1 = false, bool flipTp2 = false) {
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
        return {seg1};
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
        if (flipTp1) {
            tp1 = c1.center + (c1.center - tp1);
        }

        sqr_tan_half_phi = CGAL::square (upper_tan_half_phi);
        sin_phi = 2 * upper_tan_half_phi / (1 + sqr_tan_half_phi);
        cos_phi = (1 - sqr_tan_half_phi) / (1 + sqr_tan_half_phi);

        Point<Exact> tp2;
        if (! rotate_pi2 )
        {
            tp2 = Point<Exact> (c2.center.x() + c2.radius*cos_phi, c2.center.y() + c2.radius*sin_phi);
        }
        else
        {
            tp2 = Point<Exact> (c2.center.x() + c2.radius*sin_phi, c2.center.y() - c2.radius*cos_phi);
        }
        if (flipTp2) {
            tp2 = c2.center + (c2.center - tp2);
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

        return {seg1, seg2};
    }
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
RationalTangent algebraicCircleTangentToRationalSegments(const OneRootPoint& p1, const Point<Exact>& p2,
														 const RationalRadiusCircle& c1,
														 bool flipTp1 = false) {
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
		auto seg1 = Segment<Exact>(tp1, p2);
		return {seg1};
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
		if (flipTp1) {
			tp1 = c1.center + (c1.center - tp1);
		}
		auto l1 = Line<Exact>(c1.center, tp1).perpendicular(tp1);

		// Intersect the two lines. The intersection point serves as a common
		// end point for the two line segments we are about to introduce.
		auto mid_p = l1.projection(CGAL::midpoint(p2, p2 - Vector<Exact>(!rotate_pi2 ? app_delta_x : app_delta_y, !rotate_pi2 ? app_delta_y : -app_delta_x)));

		auto seg1 = Segment<Exact> (tp1, mid_p);
		auto seg2 = Segment<Exact> (mid_p, p2);

		return {seg1, seg2};
	}
}

RationalTangent RationalTangent::opposite() const {
    if (auto* uvs = std::get_if<Segment<Exact>>(&variant)) {
        return {uvs->opposite()};
    } else if (auto* uvsp = std::get_if<std::pair<Segment<Exact>, Segment<Exact>>>(&variant)) {
        auto [uvs1, uvs2] = *uvsp;
        return {uvs2.opposite(), uvs1.opposite()};
    } else {
        throw std::runtime_error("Impossible: unexpected type in variant");
    }
}

Point<Exact> RationalTangent::source() const {
	if (auto* uvs = std::get_if<Segment<Exact>>(&variant)) {
		return uvs->source();
	} else if (auto* uvsp = std::get_if<std::pair<Segment<Exact>, Segment<Exact>>>(&variant)) {
		return uvsp->first.source();
	} else {
		throw std::runtime_error("Impossible: unexpected type in variant.");
	}
}

Point<Exact> RationalTangent::target() const {
	if (auto* uvs = std::get_if<Segment<Exact>>(&variant)) {
		return uvs->target();
	} else if (auto* uvsp = std::get_if<std::pair<Segment<Exact>, Segment<Exact>>>(&variant)) {
		return uvsp->second.target();
	} else {
		throw std::runtime_error("Impossible: unexpected type in variant.");
	}
}

Polyline<Exact> RationalTangent::polyline() const {
	Polyline<Exact> polyline;
	if (auto* uvs = std::get_if<Segment<Exact>>(&variant)) {
		polyline.push_back(uvs->source());
		polyline.push_back(uvs->target());
	} else if (auto* uvsp = std::get_if<std::pair<Segment<Exact>, Segment<Exact>>>(&variant)) {
		auto [t1, t2] = *uvsp;
		polyline.push_back(t1.source());
		polyline.push_back(t1.target());
		polyline.push_back(t2.target());
	} else {
		throw std::runtime_error("Impossible: unexpected type in variant.");
	}
	return polyline;
}

Circle<Exact> RationalRadiusCircle::circle() const {
	return {center, radius * radius};
}

std::optional<std::pair<RationalTangent, RationalTangent>>
rationalBitangents(const RationalRadiusCircle& c1, const RationalRadiusCircle& c2, bool inner) {
    auto tps = bitangentPoints(c1, c2, inner);
    if (!tps.has_value()) return std::nullopt;
    if (inner) {
        return std::pair(algebraicCircleBitangentToRationalSegments(tps->first.first, tps->first.second, c1, c2, false, true),
                         algebraicCircleBitangentToRationalSegments(tps->second.second, tps->second.first, c2, c1, true, false).opposite());
    } else {
        return std::pair(algebraicCircleBitangentToRationalSegments(tps->first.first, tps->first.second, c1, c2),
                         algebraicCircleBitangentToRationalSegments(tps->second.second, tps->second.first, c2, c1).opposite());
    }
}

std::optional<std::pair<RationalTangent, RationalTangent>>
rationalTangents(const Point<Exact>& p, const RationalRadiusCircle& c) {
	auto tps = tangentPoints(p, c);
	if (!tps.has_value()) return std::nullopt;
	return std::pair(algebraicCircleTangentToRationalSegments(tps->first, p, c, true).opposite(),
					 algebraicCircleTangentToRationalSegments(tps->second, p, c, false).opposite());
}

std::optional<std::pair<RationalTangent, RationalTangent>>
rationalTangents(const RationalRadiusCircle& c, const Point<Exact>& p) {
	auto tps = tangentPoints(c, p);
	if (!tps.has_value()) return std::nullopt;
	return std::pair(algebraicCircleTangentToRationalSegments(tps->first, p, c, false),
					 algebraicCircleTangentToRationalSegments(tps->second, p, c, true));
}
}