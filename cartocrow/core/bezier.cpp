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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 08-09-2020
*/

#include "bezier.h"

#include <cmath>

#include <glog/logging.h>

namespace cartocrow {

BezierCurve::BezierCurve(const Point<Inexact>& source, const Point<Inexact>& source_control,
                         const Point<Inexact>& target_control, const Point<Inexact>& target)
    : m_controlPoints({source - CGAL::ORIGIN, source_control - CGAL::ORIGIN,
                       target_control - CGAL::ORIGIN, target - CGAL::ORIGIN}) {
	m_coefficients = {
	    m_controlPoints[3] - m_controlPoints[0] + 3 * (m_controlPoints[1] - m_controlPoints[2]), // t^3
	    3 * (m_controlPoints[0] + m_controlPoints[2] - 2 * m_controlPoints[1]), // t^2
	    3 * (m_controlPoints[1] - m_controlPoints[0]), // t
	    m_controlPoints[0] // 1
	};
}

/**@brief Give the starting point of the curve.
 * @return the starting point of the curve.
 */
Point<Inexact> BezierCurve::source() const {
	return Point<Inexact>(CGAL::ORIGIN) + m_controlPoints[0];
}

/**@brief Give the second control point.
 *
 * The curve at the source is tangent to the line connecting the source and this control point.
 * @return the second control point.
 */
Point<Inexact> BezierCurve::sourceControl() const {
	return Point<Inexact>(CGAL::ORIGIN) + m_controlPoints[1];
}

/**@brief Give the third control point.
 *
 * The curve at the target is tangent to the line connecting the target and this control point.
 * @return the third control point.
 */
Point<Inexact> BezierCurve::targetControl() const {
	return Point<Inexact>(CGAL::ORIGIN) + m_controlPoints[2];
}

/**@brief Give the terminating point of the curve.
 * @return the terminating point of the curve.
 */
Point<Inexact> BezierCurve::target() const {
	return Point<Inexact>(CGAL::ORIGIN) + m_controlPoints[3];
}

/**@brief Evaluate the Bezier curve's function after traversing some ratio of the curve.
 *
 * @param t @parblock the ratio of the curve traversed.
 *
 * This ratio must be in the range [0, 1]. Evaluating at t=0 gives the source of the curve and evaluating at t=1 gives the target of the curve.
 *
 * Note that this variable does not directly correlate with the traversed length of the curve. For example, evaluating the curve at t=0.5 does not necessarily give the point that divides the curve into two equal length parts.
 * @endparblock
 * @return the point on the curve at t.
 */
Point<Inexact> BezierCurve::evaluate(const Number<Inexact>& t) const {
	CHECK_GE(t, 0);
	CHECK_LE(t, 1);
	if (t == 0) {
		return source();
	} else if (t == 1) {
		return target();
	}

	const Number<Inexact> t_ = 1 - t;
	const Number<Inexact> a = t_ * t_ * t_;
	const Number<Inexact> b = 3 * t * t_ * t_;
	const Number<Inexact> c = 3 * t * t * t_;
	const Number<Inexact> d = t * t * t;

	return CGAL::ORIGIN + a * m_controlPoints[0] + b * m_controlPoints[1] + c * m_controlPoints[2] +
	       d * m_controlPoints[3];
}

size_t BezierCurve::intersectRay(const Point<Inexact>& source, const Point<Inexact>& target,
                                 Point<Inexact>* intersections,
                                 Number<Inexact>* intersection_t) const {
	CHECK_NE(source, target);

	// Computing the intersection(s) of a line with a cubic Bezier curve,
	// based on the Particle In Cell javascript implementation (https://www.particleincell.com/2013/cubic-line-intersection/),
	// which is based on Stephen Schmitt's algorithm (http://mysite.verizon.net/res148h4j/javascript/script_exact_cubic.html).

	const Vector<Inexact> AB(target.y() - source.y(), // A = y2-y1
	                         source.x() - target.x() // B = x1-x2
	);
	const Number<Inexact> C = source.x() * (source.y() - target.y()) +
	                          source.y() * (target.x() - source.x()); // C = x1*(y1-y2)+y1*(x2-x1)

	std::array<Number<Inexact>, 3> roots;
	{
		// Compute the roots of the cubic function based on AB and the coefficients.
		const Number<Inexact> f_3 = AB * m_coefficients[0]; // t^3
		const Number<Inexact> f_2 = AB * m_coefficients[1]; // t^2
		const Number<Inexact> f_1 = AB * m_coefficients[2]; // t
		const Number<Inexact> f_0 = AB * m_coefficients[3] + C; // 1

		CHECK_NE(f_3, 0);
		const Number<Inexact> A = f_2 / f_3;
		const Number<Inexact> B = f_1 / f_3;
		const Number<Inexact> C = f_0 / f_3;

		const Number<Inexact> Q = (3 * B - A * A) / 9;
		const Number<Inexact> R = (9 * A * B - 27 * C - 2 * A * A * A) / 54;
		const Number<Inexact> D = Q * Q * Q + R * R; // Polynomial discriminant.

		if (D >= 0) // Complex or duplicate roots.
		{
			const Number<Inexact> sqrt_D = CGAL::sqrt(D);
			constexpr const Number<Inexact> third = Number<Inexact>(1) / 3;
			const Number<Inexact> S = CGAL::sign(R + sqrt_D) * std::pow(std::abs(R + sqrt_D), third);
			const Number<Inexact> T = CGAL::sign(R - sqrt_D) * std::pow(std::abs(R - sqrt_D), third);

			roots[0] = -A / 3 + (S + T); // Real root.
			roots[1] = -A / 3 - (S + T) / 2; // Real part of complex root.
			roots[2] = -A / 3 - (S + T) / 2; // Real part of complex root.

			const Number<Inexact> sqrt_3 = CGAL::sqrt(Number<Inexact>(3));
			const Number<Inexact> I = std::abs(sqrt_3 * (S - T) / 2); // Complex part of root pair

			// Discard complex roots.
			if (I != 0) {
				roots[1] = -1;
				roots[2] = -1;
			}
		} else // Distinct real roots.
		{
			const Number<Inexact> th = std::acos(R / CGAL::sqrt(-std::pow(Q, 3)));

			roots[0] = 2 * CGAL::sqrt(-Q) * std::cos(th / 3) - A / 3;
			roots[1] = 2 * CGAL::sqrt(-Q) * std::cos((th + 2 * M_PI) / 3) - A / 3;
			roots[2] = 2 * CGAL::sqrt(-Q) * std::cos((th + 4 * M_PI) / 3) - A / 3;
		}
	}

	size_t num = 0;
	for (const Number<Inexact>& t : roots) {
		// Ignore roots outside the range of the curve.
		if (t < 0 || 1 < t) {
			continue;
		}

		/*const Number<Inexact> t_3 = t * t * t;
    const Number<Inexact> t_2 = t * t;
    const Point<Inexact> intersection = Point(CGAL::ORIGIN) + t_3 * coefficients_[0] + t_2 * coefficients_[1] + t * coefficients_[2] + coefficients_[3];*/
		const Point<Inexact> intersection = evaluate(t);

		// Verify the intersection is on the ray by using the inner product.
		const Number<Inexact> s = (intersection.x() - source.x()) * (target.x() - source.x());
		if (s < 0) {
			continue;
		}

		intersections[num] = intersection;
		intersection_t[num] = t;
		++num;
	}

	return num;
}

BezierCurve BezierCurve::transform(const CGAL::Aff_transformation_2<Inexact> &t) const {
	return { source().transform(t), sourceControl().transform(t),
	        targetControl().transform(t), target().transform(t) };
}
BezierCurve::BezierCurve(const Point<Inexact>& source, const Point<Inexact>& control,
                         const Point<Inexact>& target)
    : BezierCurve(source, CGAL::ORIGIN + (source - CGAL::ORIGIN) / 3 + 2 * (control - CGAL::ORIGIN) / 3,
                  CGAL::ORIGIN + (target - CGAL::ORIGIN) / 3 + 2 * (control - CGAL::ORIGIN) / 3, target) {}

/**@class BezierSpline
 * @brief A cubic Bezier spline.
 */

/**@fn BezierSpline::CurveSet
 * @brief The type for storing the Bezier curves.
 */

/**@brief Construct an empty Bezier spline.
 */
BezierSpline::BezierSpline() : bounding_box_() {}

/**@brief Check whether the spline is valid.
 *
 * For the spline to be valid it must not be degenerate, i.e. its points must not all be the same.
 * @return whether the spline is valid.
 */
bool BezierSpline::isValid() const {
	if (isEmpty()) {
		return false;
	}

	bool valid = true;
	Point<Inexact> current = curves_.front().source();
	for (const BezierCurve& curve : curves_) {
		valid &= curve.source() == current;
		current = curve.target();
	}

	return valid;
}

/**@brief Check whether the spline is empty, i.e. it has no curves.
 * @return whether the spline is empty.
 */
bool BezierSpline::isEmpty() const {
	return curves_.empty();
}

/**@brief Check whether the spline is continuous.
 *
 * The spline is continuous if each next curve starts where the previous one ends.
 * @return whether the spline is continuous.
 */
bool BezierSpline::isContinuous() const {
	Point<Inexact> prev = curves_.front().source();
	for (const BezierCurve& curve : curves_) {
		const Point<Inexact> next = curve.source();
		if (prev != next) {
			return false;
		}
		prev = curve.target();
	}
	return true;
}

/**@brief Check whether the spline is closed.
 *
 * The spline is closed if each next curve starts where the previous one ends and the first curve starts where the last curve ends.
 * @return whether the spline is closed.
 */
bool BezierSpline::isClosed() const {
	return isContinuous() && curves_.front().source() == curves_.back().target();
}

/**@brief Convert the spline to a circle, and check whether this conversion is appropriate.
 *
 * The conversion is appropriate if the ratio between the smallest and largest distances to the circumcenter is small enough.
 * @param circle the circle approximating the spline.
 * @param epsilon the maximum allowed ratio between distances to the circumcenter.
 * @return whether the circle is appropriate.
 */
bool BezierSpline::toCircle(Circle<Inexact>& circle, const Number<Inexact>& epsilon /*= 0.01*/) const {
	Vector<Inexact> sum(0, 0);
	for (const BezierCurve& curve : curves_) {
		const Point<Inexact> center =
		    CGAL::circumcenter(curve.source(), curve.evaluate(0.5), curve.target());
		sum += center - Point<Inexact>(CGAL::ORIGIN);
	}
	const Point<Inexact> kernel = Point<Inexact>(CGAL::ORIGIN) + (sum / curves_.size());

	struct SquaredDistance {
		SquaredDistance(const Point<Inexact>& kernel)
		    : kernel(kernel), squared_distance_min(-1), squared_distance_max(0) {}

		void operator()(const Point<Inexact>& point) {
			const Number<Inexact> squared_distance = CGAL::squared_distance(kernel, point);
			squared_distance_min = squared_distance_min < 0
			                           ? squared_distance
			                           : std::min(squared_distance_min, squared_distance);
			squared_distance_max = std::max(squared_distance_max, squared_distance);
		}

		inline Number<Inexact> squaredRadius() const {
			return (squared_distance_min + squared_distance_max) / 2.0;
		}
		inline Number<Inexact> distanceRatio() const {
			return squared_distance_max / squared_distance_min;
		}

		const Point<Inexact>& kernel;
		Number<Inexact> squared_distance_min, squared_distance_max;
	};

	SquaredDistance squared_distance(kernel);
	squared_distance(curves_.front().source());

	for (const BezierCurve& curve : curves_) {
		// Note that we do not check the source: this will be checked as the target of the previous curve.
		const Number<Inexact> denom = 4;
		for (int e = 1; e < denom; ++e) {
			squared_distance(curve.evaluate(e / denom));
		}

		squared_distance(curve.target());
	}
	circle = Circle<Inexact>(kernel, squared_distance.squaredRadius());

	return squared_distance.distanceRatio() <= (1 + epsilon);
}

/**@brief Access the curves of the spline.
 * @return the set of curves.
 */
const BezierSpline::CurveSet& BezierSpline::curves() const {
	return curves_;
}

/**@brief Access the curves of the spline.
 * @return the set of curves.
 */
BezierSpline::CurveSet& BezierSpline::curves() {
	return curves_;
}

/**@brief Add a Bezier curve to the end of the spline.
 * @param source the source point of the curve.
 * @param source_control the first control point of the curve.
 * @param target_control the second control point of the curve.
 * @param target the target point of the curve.
 */
void BezierSpline::appendCurve(const Point<Inexact>& source, const Point<Inexact>& source_control,
                               const Point<Inexact>& target_control, const Point<Inexact>& target) {
	curves_.emplace_back(source, source_control, target_control, target);
}

void BezierSpline::appendCurve(BezierCurve curve) {
	curves_.push_back(curve);
}

/**@brief Add a Bezier curve to the end of the spline.
 *
 * The source of this curve is the target of the previous curve. For this reason, it cannot be the first curve of the spline.
 *
 * Note that the transition from one Bezier curve to the next is not required to be smooth, but a smooth necklace usually looks better than a jagged one.
 * @param source_control the first control point of the curve.
 * @param target_control the second control point of the curve.
 * @param target the target point of the curve.
 */
void BezierSpline::appendCurve(const Point<Inexact>& source_control,
                               const Point<Inexact>& target_control, const Point<Inexact>& target) {
	CHECK(!curves_.empty());
	const Point<Inexact>& source = curves_.back().target();
	appendCurve(source, source_control, target_control, target);
}

/**@brief reverse the spline.
 *
 * This involves reversing the order of the curves, as well as reversing the direction of each curve.
 */
void BezierSpline::reverse() {
	CurveSet reverse;
	for (CurveSet::reverse_iterator curve_iter = curves_.rbegin(); curve_iter != curves_.rend();
	     ++curve_iter) {
		reverse.emplace_back(curve_iter->target(), curve_iter->targetControl(),
		                     curve_iter->sourceControl(), curve_iter->source());
	}
	curves_.swap(reverse);
}

/**@brief Compute the bounding box of the spline.
 * Because computing the exact bounding box can be very costly in certain case, a decent estimate of the bounding box is returned instead.
 *
 * The complete spline is guaranteed to be inside this estimated box.
 * @return the bounding box of the spline.
 */
Box BezierSpline::computeBoundingBox() const {
	if (bounding_box_.xmax() <= bounding_box_.xmin() || bounding_box_.ymax() <= bounding_box_.ymin()) {
		// Computing the exact bounding box is more complex than required.
		// There are several obvious approaches to interpolate the bounding box (with its own disadvantages):
		// * sampling each curve (expensive for many short curves),
		// * sampling angles around the kernel (may miss small curves, expensive/complex curve selection),
		// * taking the bounding box of the set of control points (approximation may be very rough).
		// We choose the last approach, because overestimating the bounding box is more desirable than underestimating it.
		Inexact::Construct_bbox_2 bbox = Inexact().construct_bbox_2_object();
		for (const BezierCurve& curve : curves_) {
			bounding_box_ += bbox(curve.source()) + bbox(curve.sourceControl()) +
			                 bbox(curve.targetControl()) + bbox(curve.target());
		}
	}

	return bounding_box_;
}

} // namespace cartocrow::necklace_map
