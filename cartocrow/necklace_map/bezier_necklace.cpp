/*
The Necklace Map library implements the algorithmic geo-visualization
method by the same name, developed by Bettina Speckmann and Kevin Verbeek
at TU Eindhoven (DOI: 10.1109/TVCG.2010.180 & 10.1142/S021819591550003X).
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 25-02-2020
*/

#include "bezier_necklace.h"

#include <algorithm>
#include <cmath>

#include <glog/logging.h>

namespace cartocrow {
namespace necklace_map {
namespace {

// Compare the curves by the angle of their target point: if the set of curves forms a closed curve, the curve with the smallest target angle must contain the angle 0.
class CompareBezierCurves {
  public:
	explicit CompareBezierCurves(const BezierNecklace& shape) : shape_(shape) {}

	inline bool operator()(const BezierCurve& curve_a, const BezierCurve& curve_b) const {
		return shape_.computeAngleRad(curve_a.target()) < shape_.computeAngleRad(curve_b.target());
	}

	inline bool operator()(const BezierCurve& curve, const Number<Inexact>& angle_rad) const {
		const Number<Inexact> angle_target_rad = shape_.computeAngleRad(curve.target());
		return angle_target_rad < angle_rad;
	}

  private:
	const BezierNecklace& shape_;
};

} // anonymous namespace

/**@class BezierNecklace
 * @brief A cubic Bezier curve necklace.
 *
 * Note that for this necklace, the kernel must be set explicitly.
 */

/**@fn BezierNecklace::Ptr
 * @brief The preferred pointer type for storing or sharing a Bezier necklace.
 */

/**@fn const Number BezierNecklace::kDistanceRatioEpsilon
 * @brief The maximum ratio within distances from the kernel to classify as a circle necklace.
 */

/**@brief Construct a new Bezier spline necklace.
 *
 * The necklace must be a star-shaped curve with its kernel as star point.
 * @param spline the Bezier spline shape.
 * @param kernel the kernel (and star-point) of the necklace.
 */
BezierNecklace::BezierNecklace(const BezierSpline spline, const Point<Inexact>& kernel)
    : spline_(spline), kernel_(kernel) {
	// Clockwise curves are reversed.
	if (CGAL::orientation(spline_.curves().begin()->source(),
	                      spline_.curves().begin()->sourceControl(), kernel_) == CGAL::CLOCKWISE) {
		spline_.reverse();
	}

	// Reorder the curves to start with the curve directly to the right of the kernel.
	std::sort(spline_.curves().begin(), spline_.curves().end(), CompareBezierCurves(*this));
	CHECK(spline_.isClosed());
}

const Point<Inexact>& BezierNecklace::kernel() const {
	return kernel_;
}

/**@brief Give the Bezier spline shape of the necklace.
 * @return the shape of the necklace.
 */
const BezierSpline& BezierNecklace::spline() const {
	return spline_;
}

bool BezierNecklace::isValid() const {
	// Check whether the curve is valid in relation to the necklace.
	// For the curve to be valid it must not be degenerate, i.e. its points must not all be the same.
	// The curve must also be fully visible from the kernel, i.e. no ray originating from the kernel intersects the curve in more than one point.
	// Finally, the curve must describe a counterclockwise sweep around the kernel, i.e. the curve must start to the left of the vector from the kernel to the curve source.

	bool valid = spline_.isValid();
	for (const BezierCurve& curve : spline_.curves()) {
		valid &= curve.source() != curve.sourceControl() && curve.target() != curve.targetControl() &&
		         !CGAL::right_turn(curve.source(), curve.sourceControl(), kernel()) &&
		         !CGAL::left_turn(curve.target(), curve.targetControl(), kernel());
	}

	return valid;
}

bool BezierNecklace::intersectRay(const Number<Inexact>& angle_rad,
                                  Point<Inexact>& intersection) const {
	// Find the curve that contains the angle.
	BezierSpline::CurveSet::const_iterator curve_iter = findCurveContainingAngle(angle_rad);
	if (curve_iter == spline_.curves().end()) {
		return false;
	}

	Number<Inexact> t;
	return intersectRay(angle_rad, curve_iter, intersection, t);
}

Box BezierNecklace::computeBoundingBox() const {
	return spline_.computeBoundingBox();
}

Number<Inexact> BezierNecklace::computeCoveringRadiusRad(const Range& range,
                                                         const Number<Inexact>& radius) const {
	if (radius == 0) {
		return 0;
	}

	// Sample the range and determine the largest covering radius, i.e. the largest angle difference towards the point on the spline at a fixed distance.
	// There are several viable sampling strategies (with evaluation):
	// - fixed angle difference (sensitive to spline curvature, i.e. low curvature means oversampling)
	// - fixed distance (sensitive to spline curvature, i.e. low curvature means oversampling)
	// - fixed sample size per curve (sensitive to curve length, i.e. short curves means oversampling)
	// - fixed sample size per range (sensitive to range length, i.e. short range means oversampling)
	// - binary search on range (as previous and loses benefits of moving curve-at-distance together with sample)
	// We chose for the fixed sample size per curve because the trade-off between accuracy and sampling size seemed reasonable.
	// Taking five samples per curve (t = {0, 1/4, 1/2, 3/4, 1}) captures the extreme curvature parts of each Cubic spline.

	const BezierSpline::CurveSet::const_iterator curve_iter_from =
	    findCurveContainingAngle(range.from());
	const BezierSpline::CurveSet::const_iterator curve_iter_to = findCurveContainingAngle(range.to());
	CHECK(curve_iter_from != spline_.curves().end());
	CHECK(curve_iter_to != spline_.curves().end());

	Number<Inexact> t_from, t_to;
	Point<Inexact> point;
	CHECK(intersectRay(range.from(), curve_iter_from, point, t_from));
	CHECK(intersectRay(range.to(), curve_iter_to, point, t_to));

	const Number<Inexact> t_step = 0.25;
	Number<Inexact> t = t_from;
	Number<Inexact> covering_radius_rad = 0;
	for (BezierSpline::CurveSet::const_iterator curve_iter = curve_iter_from;
	     curve_iter != curve_iter_to || t <= t_to;) {
		point = curve_iter->evaluate(t);
		const Number<Inexact> angle_rad = computeAngleRad(point);

		Number<Inexact> angle_ccw, angle_cw;
		CHECK(computeAngleAtDistanceRad(point, radius, curve_iter, t, angle_ccw));
		CHECK(computeAngleAtDistanceRad(point, -radius, curve_iter, t, angle_cw));

		const Number<Inexact> covering_radius_rad_ccw = CircularRange(angle_rad, angle_ccw).length();
		const Number<Inexact> covering_radius_rad_cw = CircularRange(angle_cw, angle_rad).length();

		covering_radius_rad = std::max(covering_radius_rad, covering_radius_rad_ccw);
		covering_radius_rad = std::max(covering_radius_rad, covering_radius_rad_cw);

		t += t_step;
		if (curve_iter == curve_iter_to && t_to < t && t < t_to + t_step) {
			t = t_to; // Final step at the exact range endpoint.
		}
		if (curve_iter != curve_iter_to && 1 <= t) {
			t = 0;
			if (++curve_iter == spline_.curves().end()) {
				curve_iter = spline_.curves().begin();
			}
		}
	}

	return covering_radius_rad;
}

Number<Inexact> BezierNecklace::computeDistanceToKernel(const Range& range) const {
	// Sample the range and determine the shortest distance to the kernel.
	// There are several viable sampling strategies (with evaluation):
	// - fixed angle difference (sensitive to spline curvature, i.e. low curvature means oversampling)
	// - fixed distance (sensitive to spline curvature, i.e. low curvature means oversampling)
	// - fixed sample size per curve (sensitive to curve length, i.e. short curves means oversampling)
	// - fixed sample size per range (sensitive to range length, i.e. short range means oversampling)
	// - binary search on range (as previous and loses benefits of moving curve-at-distance together with sample)
	// We chose for the fixed sample size per curve because the trade-off between accuracy and sampling size seemed reasonable.
	// Taking five samples per curve (t = {0, 1/4, 1/2, 3/4, 1}) captures the extreme curvature parts of each Cubic spline.

	const BezierSpline::CurveSet::const_iterator curve_iter_from =
	    findCurveContainingAngle(range.from());
	const BezierSpline::CurveSet::const_iterator curve_iter_to = findCurveContainingAngle(range.to());
	CHECK(curve_iter_from != spline_.curves().end());
	CHECK(curve_iter_to != spline_.curves().end());

	Number<Inexact> t_from, t_to;
	Point<Inexact> point;
	CHECK(intersectRay(range.from(), curve_iter_from, point, t_from));
	CHECK(intersectRay(range.to(), curve_iter_to, point, t_to));

	const Number<Inexact> t_step = 0.25;
	Number<Inexact> t = t_from;
	Number<Inexact> squared_distance = std::numeric_limits<Number<Inexact>>::max();
	for (BezierSpline::CurveSet::const_iterator curve_iter = curve_iter_from;
	     curve_iter != curve_iter_to || t <= t_to;) {
		point = curve_iter->evaluate(t);
		squared_distance = std::min(squared_distance, CGAL::squared_distance(point, kernel()));

		t += t_step;
		if (curve_iter == curve_iter_to && t_to < t && t < t_to + t_step) {
			t = t_to; // Final step at the exact range endpoint.
		}
		if (curve_iter != curve_iter_to && 1 <= t) {
			t = 0;
			if (++curve_iter == spline_.curves().end()) {
				curve_iter = spline_.curves().begin();
			}
		}
	}

	return CGAL::sqrt(squared_distance);
}

Number<Inexact> BezierNecklace::computeAngleAtDistanceRad(const Number<Inexact>& angle_rad,
                                                          const Number<Inexact>& distance) const {
	if (distance == 0) {
		return angle_rad;
	}

	// Find the curve that contains the angle.
	/*CHECK(checked_);*/
	const BezierSpline::CurveSet::const_iterator curve_iter = findCurveContainingAngle(angle_rad);
	if (curve_iter == spline_.curves().end()) {
		return false;
	}

	// Find the angle at the specified distance.
	Point<Inexact> point;
	Number<Inexact> t;
	CHECK(intersectRay(angle_rad, curve_iter, point, t));
	Number<Inexact> angle_out_rad;
	const bool correct = computeAngleAtDistanceRad(point, distance, curve_iter, t, angle_out_rad);
	return correct ? angle_out_rad : angle_rad;
}

void BezierNecklace::accept(NecklaceShapeVisitor& visitor) {
	visitor.Visit(*this);
}

BezierSpline::CurveSet::const_iterator
BezierNecklace::findCurveContainingAngle(const Number<Inexact>& angle_rad) const {
	BezierSpline::CurveSet::const_iterator curve_iter =
	    std::lower_bound(spline_.curves().begin(), spline_.curves().end(), wrapAngle(angle_rad),
	                     CompareBezierCurves(*this));
	return curve_iter == spline_.curves().end() ? spline_.curves().begin() : curve_iter;
}

bool BezierNecklace::intersectRay(const Number<Inexact>& angle_rad,
                                  const BezierSpline::CurveSet::const_iterator& curve_iter,
                                  Point<Inexact>& intersection, Number<Inexact>& t) const {
	const Point<Inexact> target =
	    kernel() + Vector<Inexact>(std::cos(angle_rad), std::sin(angle_rad));

	Point<Inexact> intersections[3];
	Number<Inexact> intersection_t[3];
	const size_t num_intersection =
	    curve_iter->intersectRay(kernel(), target, intersections, intersection_t);
	if (num_intersection == 0) {
		return false;
	}

	// Note that the set of Bezier curves must always be a star-shaped curve with the kernel as star point,
	// meaning that a line through the kernel has at most one intersection with the curve.
	CHECK_EQ(num_intersection, 1);
	intersection = intersections[0];
	t = intersection_t[0];
	return true;
}

bool BezierNecklace::computeAngleAtDistanceRad(
    const Point<Inexact>& point, const Number<Inexact>& distance,
    const BezierSpline::CurveSet::const_iterator& curve_point, const Number<Inexact>& t_point,
    Number<Inexact>& angle_rad) const {
	// Find the curve that contains the distance.
	const Number<Inexact> squared_distance = distance * distance;
	BezierSpline::CurveSet::const_iterator curve_iter = curve_point;
	Number<Inexact> t_start = t_point;
	do {
		if (0 < distance) {
			if (squared_distance <= CGAL::squared_distance(point, curve_iter->target())) {
				angle_rad = searchCurveForAngleAtDistanceRad(point, *curve_iter, squared_distance,
				                                             CGAL::COUNTERCLOCKWISE, t_start);
				return true;
			}

			if (++curve_iter == spline_.curves().end()) {
				curve_iter = spline_.curves().begin();
			}
			t_start = 0;
		} else {
			if (squared_distance <= CGAL::squared_distance(point, curve_iter->source())) {
				angle_rad = searchCurveForAngleAtDistanceRad(point, *curve_iter, squared_distance,
				                                             CGAL::CLOCKWISE, t_start);
				return true;
			}

			if (curve_iter == spline_.curves().begin()) {
				curve_iter = spline_.curves().end();
			}
			--curve_iter;
			t_start = 1;
		}
	} while (curve_iter != curve_point);

	// No curve exists for which either endpoint is far enough away.
	// Note that while curve may contain a distant enough point in its interior,
	// this case will not occur when calculating the scale factor, because the distance is limited to be much smaller than the curve length.
	return false;
}

Number<Inexact> BezierNecklace::searchCurveForAngleAtDistanceRad(
    const Point<Inexact>& point, const BezierCurve& curve, const Number<Inexact>& squared_distance,
    const CGAL::Orientation& orientation, const Number<Inexact>& t_start) const {
	// Perform a binary search on the curve to estimate the point at the specified distance.
	// The assumption is that the distance between the point and a sample on the curve is monotonic in t.
	// This assumption is based on the assumption that the Bezier spline is not too far from circular.
	Number<Inexact> lower_bound = t_start;
	Number<Inexact> upper_bound = orientation == CGAL::COUNTERCLOCKWISE ? 1 : 0;
	Point<Inexact> point_upper = curve.evaluate(upper_bound);
	Number<Inexact> squared_distance_upper = CGAL::squared_distance(point, point_upper);
	while (squared_distance * kDistanceRatioEpsilon < squared_distance_upper) {
		const Number<Inexact> t = 0.5 * (lower_bound + upper_bound);
		const Point<Inexact> point_t = curve.evaluate(t);
		const Number<Inexact> squared_distance_t = CGAL::squared_distance(point, point_t);
		CHECK_LE(squared_distance_t, squared_distance_upper);

		if (squared_distance_t < squared_distance) {
			lower_bound = t;
		} else {
			upper_bound = t;
			point_upper = point_t;
			squared_distance_upper = squared_distance_t;
		}
	}

	// The upper bound is the closest t for which the distance is confirmed to be larger than the specified distance.
	return computeAngleRad(point_upper);
}

} // namespace necklace_map
} // namespace cartocrow
