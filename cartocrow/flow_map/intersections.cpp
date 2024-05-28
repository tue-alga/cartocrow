/*
The CartoCrow library implements algorithmic geo-visualization methods,
developed at TU Eindhoven.
Copyright (C) 2021  Netherlands eScience Center and TU Eindhoven

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

Created by tvl (t.vanlankveld@esciencecenter.nl) on 26-02-2021
*/

#include "intersections.h"

namespace cartocrow::flow_map {
namespace detail {

int orientation(const PolarLine& line, const PolarPoint& point) {
	if (!line.containsPhi(point.phi())) {
		return -1;
	}

	// Note that using the standardized method for computing the R at phi of
	// the point on the line and comparing that with the given point fails for
	// lines through the pole.
	// Instead, we project the given point onto the pedal vector and compare the
	// distance to the foot of the line.
	const Number<Inexact> R_diff =
	    point.r() * std::cos(point.phi() - line.foot().phi()) - line.foot().r();
	return R_diff < 0 ? -1 : 0 < R_diff ? 1 : 0;
}

bool searchSpiralLineIntersection(const PolarLine& line, const Spiral& spiral, Number<Inexact>& t2,
                                  Number<Inexact>& t1, const Number<Inexact> t_precision) {
	if (t1 >= t2) {
		throw std::runtime_error("t1 needs to be smaller than t2");
	}
	const int orientationFar = orientation(line, spiral.evaluate(t1));
	const int orientationNear = orientation(line, spiral.evaluate(t2));
	if (orientationFar == 0) {
		t2 = t1;
		return true;
	}
	if (orientationNear != -orientationFar) {
		return false;
	}

	while (t2 - t1 > t_precision) {
		const Number<Inexact> tMid = (t1 + t2) / 2;

		if (tMid == t1 || tMid == t2) {
			break;
		}

		const int orientationMid = orientation(line, spiral.evaluate(tMid));

		if (orientationMid == 0) {
			t1 = tMid;
			t2 = tMid;
			return true;
		} else if (orientationMid == orientationFar) {
			t1 = tMid;
		} else {
			t2 = tMid;
		}
	}

	return true;
}

bool checkIntersection(const SpiralSegment& segment, const PolarPoint& point) {
	return segment.containsR(point.r());
}

bool checkIntersection(const PolarSegment& segment, const PolarPoint& point) {
	return segment.containsPhi(point.phi());
}
} // namespace detail

void intersect(const Spiral& spiral_1, const Spiral& spiral_2,
               std::vector<PolarPoint>& intersections) {
	// Computing the intersection of two spirals (R_1(t_1), phi_1(t_1)) and (R_2(t_2), phi_2(t_2)):
	//
	// v = (R_v, phi_v)  ->
	//    R_v = R_1(0) * e^{-t_1}; phi_v = phi_1(0) + tan(alpha_1) * t_1
	//    R_v = R_2(0) * e^{-t_2}; phi_v = phi_2(0) + tan(alpha_2) * t_2
	//
	//    R_1(0) * e^{-t_1} = R_2(0) * e^{-t_2}
	//    e^{-t_1} = (R_2(0) / R_1(0)) * e^{-t_2}
	//    e^{-t_1} = e^ln(R_2(0) / R_1(0)) * e^{-t_2}
	//    e^{-t_1} = e^{ln(R_2(0) / R_1(0)) - t_2}
	//    -t_1 = ln(R_2(0) / R_1(0)) - t_2  =>  t_2 = ln(R_2(0) / R_1(0)) + t_1
	//
	//    phi_1(0) + tan(alpha_1) * t_1 = phi_2(0) + tan(alpha_2) * t_2
	//    phi_1(0) + tan(alpha_1) * t_1 = phi_2(0) + tan(alpha_2) * (ln(R_2(0) / R_1(0)) + t_1)
	//    phi_1(0) + tan(alpha_1) * t_1 = phi_2(0) + tan(alpha_2) * ln(R_2(0) / R_1(0)) + tan(alpha_2) * t_1
	//    tan(alpha_1) * t_1 - tan(alpha_2) * t_1 = phi_2(0) - phi_1(0) + tan(alpha_2) * ln(R_2(0) / R_1(0))
	//    t_1 = (phi_2(0) - phi_1(0) + tan(alpha_2) * ln(R_2(0) / R_1(0))) / (tan(alpha_1) - tan(alpha_2))
	//
	// Note that according to the Java implementation, R_v can also be based on the dot product of the Cartesian points:
	//    R_v = sqrt( R_1(0) * R_2(0) * e^{-acos(p * q / R_1(0) * R_2(0)) / tan(alpha_1)} )

	// determine the amount that d_phi changes per t
	const Number<Inexact> tan_alpha_1 = std::tan(spiral_1.angle());
	const Number<Inexact> tan_alpha_2 = std::tan(spiral_2.angle());
	const Number<Inexact> ddt_phi = tan_alpha_1 - tan_alpha_2;
	if (ddt_phi == 0) {
		return;
	}
	const Number<Inexact> t_period = std::abs(M_2xPI / ddt_phi);

	// determine the time to spend on the second spiral to reach the same
	// distance from the pole
	const Number<Inexact> d_t_2 = std::log(spiral_2.anchor().r() / spiral_1.anchor().r());

	// determine the difference in angle at this time
	const Number<Inexact> d_phi =
	    wrapAngle(spiral_2.anchor().phi() + tan_alpha_2 * d_t_2 - spiral_1.anchor().phi());
	if (d_phi == 0) {
		intersections.push_back(spiral_1.evaluate(t_period));
		intersections.push_back(spiral_1.evaluate(0));
		return;
	}

	// remember that the spirals have an infinite number of intersections;
	// we want the one farthest from the pole for which 0 < t
	const Number<Inexact> t_1_positive = 0 < ddt_phi ? d_phi / ddt_phi : (d_phi - M_2xPI) / ddt_phi;
	//CHECK_LT(0, t_1_positive);
	//CHECK_LE(t_1, std::abs(M_PI / tan_alpha_1));
	//CHECK_LT(t_1_positive, t_period);

	intersections.push_back(spiral_1.evaluate(t_1_positive));
	intersections.push_back(spiral_1.evaluate(t_1_positive - t_period));
}

void intersect(const PolarLine& line_1, const PolarLine& line_2,
               std::vector<PolarPoint>& intersections) {
	// Computing the intersection is done by projecting the foot of the first line onto the pedal vector of the second line and converting the distance to travel to the foot of the second line to the distance on the first line.
	// Given the angle between pedal vectors phi_d and the vector lengths R_1 and R_2,
	// the foot of the first line is projected onto a point at signed distance d = R_1 * cos(phi_d);
	// the total distance to the foot of the second line is R_2 - d;
	// the signed (assuming phi_d is phi_2 - phi_1) distance between the foot of the first line and the intersection is
	// t_1 = (R_2 - d) / sin(pi - phi_d) = (R_2 - d) / sin(phi_d)
	// t_1 = (R_2 - R_1 * cos(phi_d)) / sin(phi_d)
	// t_1 = R_2 / sin(phi_d) - R_1 * cos(phi_d) / sin(phi_d) = R_2 / sin(phi_d) - R_1 / tan(phi_d)

	const Number<Inexact> phi_d = wrapAngle(line_2.foot().phi() - line_1.foot().phi());
	if (std::abs(phi_d) < Number<Inexact>(1e-15) || std::abs(phi_d - M_PI) < Number<Inexact>(1e-15)) {
		return;
	}

	// projection of first line
	const Number<Inexact> t_project = line_1.foot().r() / std::tan(phi_d);

	// pedal distance of second line
	const Number<Inexact> t_pedal = line_2.foot().r() / std::sin(phi_d);

	intersections.push_back(line_1.pointAlongLine(t_pedal - t_project));
}

void intersect(const PolarLine& line, const Spiral& spiral, std::vector<PolarPoint>& intersections) {
	// we must compute the t on the line, because this is the only way we can
	// represent the pole
	const Number<Inexact>& phi_line = line.foot().phi();
	const Number<Inexact>& phi_spiral = spiral.anchor().phi();

	if (spiral.angle() == 0) {
		const Number<Inexact> phi_diff = wrapAngle(phi_spiral - phi_line, -M_PI);

		if (line.foot().r() == 0) {
			if (phi_diff == -M_PI_2) {
				// overlapping in 'clockwise' direction from the foot
				intersections.push_back(line.pointAlongLine(0));
				intersections.push_back(line.pointAlongLine(-spiral.anchor().r()));
				return;
			} else if (phi_diff == M_PI_2) {
				// overlapping in 'counter-clockwise' direction from the foot
				intersections.push_back(line.pointAlongLine(spiral.anchor().r()));
				intersections.push_back(line.pointAlongLine(0));
				return;
			} else {
				// 1 intersection at the origin
				intersections.push_back(line.pointAlongLine(0));
				return;
			}
		} else {
			if (std::abs(phi_diff) < M_PI_2) {
				// 1 intersection at the spiral's phi
				intersections.push_back(line.pointAlongLine(line.distanceAlongLineForPhi(phi_spiral)));
				return;
			} else {
				// no intersections
				return;
			}
		}
	}

	const int anchorOrientation = detail::orientation(line, spiral.anchor());
	if (anchorOrientation == 0) {
		intersections.push_back(
		    line.pointAlongLine(line.distanceAlongLineForPhi(spiral.anchor().phi())));
		return;
	}

	// For both the line and spiral, we can express phi in R (although not unambiguously for the line).
	// We're looking for points that satisfy both relations.
	// Line (foot F = (R_f, phi_f)):
	//   cos(phi - phi_f) = R_f / R
	//   phi = phi_f + acos(R_f / R)
	// Spiral (anchor A = (R_a, phi_a), angle alpha):
	//   R(t) = R_a * e^-t  &&  phi(t) = phi_a + tan(alpha)*t
	//   R(t) / R_a = e^-t  &&  phi(t) = phi_a + tan(alpha)*t
	//   -ln(R(t) / R_a) = t  &&  phi(t) = phi_a + tan(alpha)*t
	//   phi(t) = phi_a - tan(alpha)*ln(R(t) / R_a)
	// To solve for R, it is easier to re-anchor the spiral so phi_a = phi_f.
	// Let's assume we can calculate this new anchor and call its R_a as R_n (i.e. R_n = R(phi_f) = R_a * e^{-(phi_f - phi_a) / tan(alpha)}).
	// In this case:
	//   phi = phi_a - tan(alpha)*ln(R / R_n)  &&  phi = phi_f + acos(R_f / R)
	//   phi_a - tan(alpha)*ln(R / R_n) = phi_f + acos(R_f / R)
	//   phi_a - phi_f - tan(alpha)*ln(R / R_n) = acos(R_f / R)
	//   R * cos(phi_a - phi_f - tan(alpha)*ln(R / R_n)) = R_f
	// Unfortunately, there is no easy calculus to solve for R here.
	//
	// Therefore, we take an easier (but inaccurate) approach: a binary search on the spiral to find a point 'on the line' within some small margin.
	// Specifically, we search between two points on the spiral nearest to the anchor where the tangent is parallel to the line.
	// The first point should be on the opposite side of the line compared to the anchor, the second point should be on the same side.

	const Number<Inexact> period = spiral.period();
	const Number<Inexact> t_spiral_parallel =
	    spiral.parameterForPhi(phi_line + M_PI_2 + spiral.angle());

	// We determine two reference times t_0 < t_1 such that an intersection point must lie between t_i and t_i + period / 2.
	// The point at the second reference time should lie on the same side of the line as the anchor.
	Number<Inexact> t_spiral[2];
	if (detail::orientation(line, spiral.evaluate(t_spiral_parallel)) == anchorOrientation) {
		t_spiral[1] = t_spiral_parallel;
	} else if (t_spiral_parallel < 0) {
		t_spiral[1] = t_spiral_parallel + period / 2;
	} else {
		t_spiral[1] = t_spiral_parallel - period / 2;
	}

	// The point at the first reference time should not lie on the same side of the line as the anchor.
	t_spiral[0] = t_spiral[1];
	do {
		t_spiral[0] -= period / 2;
	} while (detail::orientation(line, spiral.evaluate(t_spiral[0])) == anchorOrientation);

	for (int i = 0; i < 2; ++i) {
		Number<Inexact> t_spiral_far = t_spiral[i];
		Number<Inexact> t_spiral_near = t_spiral_far + period / 2;
		if (!detail::searchSpiralLineIntersection(line, spiral, t_spiral_near, t_spiral_far)) {
			continue;
		}

		// Convert the point on the spiral to a point on the line.
		// Note that we must be careful of lines through (and near) the pole:
		// in either case, converting through phi is incorrect or inaccurate.
		const Number<Inexact> R_near = spiral.evaluate(t_spiral_near).r();
		Number<Inexact> t_line_near[2];
		const int num = line.collectT(R_near, t_line_near);
		if (num == 2) {
			const Number<Inexact> phi_spiral_near = spiral.evaluate(t_spiral_near).phi();
			const Number<Inexact> phi_line_near_0 = line.pointAlongLine(t_line_near[0]).phi();
			const Number<Inexact> phi_line_near_1 = line.pointAlongLine(t_line_near[1]).phi();

			if (std::abs(wrapAngle(phi_spiral_near - phi_line_near_0, -M_PI)) <
			    std::abs(wrapAngle(phi_spiral_near - phi_line_near_1, -M_PI))) {
				intersections.push_back(line.pointAlongLine(t_line_near[0]));
			} else {
				intersections.push_back(line.pointAlongLine(t_line_near[1]));
			}
		} else {
			intersections.push_back(line.pointAlongLine(0));
		}
	}
}

} // namespace cartocrow::flow_map
