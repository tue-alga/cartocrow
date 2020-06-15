/*
The Necklace Map library implements the algorithmic geo-visualization
method by the same name, developed by Bettina Speckmann and Kevin Verbeek
at TU Eindhoven (DOI: 10.1109/TVCG.2010.180 & 10.1142/S021819591550003X).
Copyright (C) 2019  Netherlands eScience Center and TU Eindhoven

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
#include <math.h>

#include <glog/logging.h>


namespace geoviz
{
namespace necklace_map
{
namespace
{

// Compare the curves by the angle of their target point: if the set of curves forms a closed curve, the curve with the smallest target angle must contain the angle 0.
class CompareBezierCurves
{
 public:
  explicit CompareBezierCurves(const NecklaceShape& shape) : shape_(shape) {}

  inline bool operator()(const BezierCurve& curve_a, const BezierCurve& curve_b) const
  {
    return shape_.ComputeAngleRad(curve_a.target()) < shape_.ComputeAngleRad(curve_b.target());
  }

  inline bool operator()(const BezierCurve& curve, const Number& angle_rad) const
  {
    const Number angle_target_rad = shape_.ComputeAngleRad(curve.target());
    return angle_target_rad < angle_rad;
  }

 private:
  const NecklaceShape& shape_;
}; // class CompareBezierCurves

} // anonymous namespace

/**@class BezierCurve
 * @brief A cubic Bezier curve.
 */

/**@brief Construct a quadratic Bezier curve based on three control points.
 * @param source the first control point and starting point of the curve.
 * @param control the second control point.
 * @param target the third control point and terminating point of the curve.
 */
BezierCurve::BezierCurve(const Point& source, const Point& control, const Point& target) :
  BezierCurve(source, control, control, target)
{}

/**@brief Construct a cubic Bezier curve based on four control points.
 * @param source the first control point and starting point of the curve.
 * @param source_control the second control point.
 * @param target_control the third control point.
 * @param target the fourth control point and terminating point of the curve.
 */
BezierCurve::BezierCurve(const Point& source, const Point& source_control, const Point& target_control, const Point& target) :
  control_points_
  ({
    source - Point(CGAL::ORIGIN),
    source_control - Point(CGAL::ORIGIN),
    target_control - Point(CGAL::ORIGIN),
    target - Point(CGAL::ORIGIN)
  })
{
  coefficients_ =
  {
    control_points_[3] - control_points_[0] + 3 * (control_points_[1] - control_points_[2]),  // t^3
    3 * (control_points_[0] + control_points_[2] - 2 * control_points_[1]),  // t^2
    3 * (control_points_[1] - control_points_[0]),  // t
    control_points_[0]  // 1
  };
}

/**@brief Check whether the curve is valid in relation to the necklace.
 *
 * For the curve to be valid it must not be degenerate, i.e. its points must not all be the same.
 *
 * The curve must also be fully visible from the kernel, i.e. no ray originating from the kernel intersects the curve in more than one point.
 *
 * Finally, the curve must describe a counterclockwise sweep around the kernel, i.e. the curve must start to the left of the vector from the kernel to the curve source.
 * @param kernel the necklace kernel.
 * @return Whether the curve is valid.
 */
bool BezierCurve::IsValid(const Point& kernel) const
{
  return
    source() != source_control() &&
    target() != target_control() &&
    !CGAL::right_turn(source(), source_control(), kernel) &&
    !CGAL::left_turn(target(), target_control(), kernel);
}

/**@brief Give the starting point of the curve.
 * @return the starting point of the curve.
 */
Point BezierCurve::source() const
{
  return Point(CGAL::ORIGIN) + control_points_[0];
}

/**@brief Give the second control point.
 *
 * The curve at the source is tangent to the line connecting the source and this control point.
 * @return the second control point.
 */
Point BezierCurve::source_control() const
{
  return Point(CGAL::ORIGIN) + control_points_[1];
}

/**@brief Give the third control point.
 *
 * The curve at the target is tangent to the line connecting the target and this control point.
 * @return the third control point.
 */
Point BezierCurve::target_control() const
{
  return Point(CGAL::ORIGIN) + control_points_[2];
}

/**@brief Give the terminating point of the curve.
 * @return the terminating point of the curve.
 */
Point BezierCurve::target() const
{
  return Point(CGAL::ORIGIN) + control_points_[3];
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
Point BezierCurve::Evaluate(const Number& t) const
{
  CHECK_GE(t, 0);
  CHECK_LE(t, 1);
  if (t == 0) return source();
  else if (t == 1) return target();

  const Number t_ = 1 - t;
  const Number a = t_ * t_ * t_;
  const Number b = 3 * t * t_ * t_;
  const Number c = 3 * t * t * t_;
  const Number d = t * t * t;

  return Point(CGAL::ORIGIN) + a * control_points_[0] + b * control_points_[1] + c * control_points_[2] + d * control_points_[3];
}

/**@brief Compute the intersection of the curve with a ray.
 * @param source the source of the ray.
 * @param target the target of the ray.
 * @param intersections @parblock the intersections of the ray and the curve.
 *
 * There may be up to three intersections, so this variable must be a container that can hold at least three values.
 * @endparblock
 * @param intersection_t @parblock the t-values of the intersections.
 *
 * There may be up to three intersections, so this variable must be a container that can hold at least three values.
 * @endparblock
 * @return whether the ray intersects the curve in at least one point.
 */
size_t BezierCurve::IntersectRay(const Point& source, const Point& target, Point* intersections, Number* intersection_t) const
{
  CHECK_NE(source, target);

  // Computing the intersection(s) of a line with a cubic Bezier curve,
  // based on the Particle In Cell javascript implementation (https://www.particleincell.com/2013/cubic-line-intersection/),
  // which is based on Stephen Schmitt's algorithm (http://mysite.verizon.net/res148h4j/javascript/script_exact_cubic.html).

  const Vector AB
  (
    target.y() - source.y(),  // A = y2-y1
    source.x() - target.x()   // B = x1-x2
  );
  const Number C =
    source.x() * (source.y() - target.y()) +
    source.y() * (target.x() - source.x());  // C = x1*(y1-y2)+y1*(x2-x1)

  std::array<Number, 3> roots;
  {
    // Compute the roots of the cubic function based on AB and the coefficients.
    const Number f_3 = AB * coefficients_[0];      // t^3
    const Number f_2 = AB * coefficients_[1];      // t^2
    const Number f_1 = AB * coefficients_[2];      // t
    const Number f_0 = AB * coefficients_[3] + C;  // 1

    CHECK_NE(f_3, 0);
    const Number A = f_2 / f_3;
    const Number B = f_1 / f_3;
    const Number C = f_0 / f_3;

    const Number Q = (3 * B - A * A) / 9;
    const Number R = (9 * A * B - 27 * C - 2 * A * A * A) / 54;
    const Number D = Q * Q * Q + R * R;  // Polynomial discriminant.

    if (D >= 0)  // Complex or duplicate roots.
    {
      const Number sqrt_D = CGAL::sqrt(D);
      constexpr const Number third = Number(1) / 3;
      const Number S = CGAL::sign(R + sqrt_D) * std::pow(std::abs(R + sqrt_D), third);
      const Number T = CGAL::sign(R - sqrt_D) * std::pow(std::abs(R - sqrt_D), third);

      roots[0] = -A/3 + (S + T);    // Real root.
      roots[1] = -A/3 - (S + T)/2;  // Real part of complex root.
      roots[2] = -A/3 - (S + T)/2;  // Real part of complex root.

      const Number sqrt_3 = CGAL::sqrt(Number(3));
      const Number I = std::abs(sqrt_3 * (S - T) / 2);  // Complex part of root pair

      // Discard complex roots.
      if (I != 0)
      {
        roots[1] = -1;
        roots[2] = -1;
      }
    }
    else  // Distinct real roots.
    {
      const Number th = std::acos(R / CGAL::sqrt(-std::pow(Q, 3)));

      roots[0] = 2 * CGAL::sqrt(-Q) * std::cos(th / 3) - A / 3;
      roots[1] = 2 * CGAL::sqrt(-Q) * std::cos((th + 2 * M_PI) / 3) - A / 3;
      roots[2] = 2 * CGAL::sqrt(-Q) * std::cos((th + 4 * M_PI) / 3) - A / 3;
    }
  }

  size_t num = 0;
  for (const Number& t : roots)
  {
    // Ignore roots outside the range of the curve.
    if (t < 0 || 1 < t)
      continue;

    /*const Number t_3 = t * t * t;
    const Number t_2 = t * t;
    const Point intersection = Point(CGAL::ORIGIN) + t_3 * coefficients_[0] + t_2 * coefficients_[1] + t * coefficients_[2] + coefficients_[3];*/
    const Point intersection = Evaluate(t);

    // Verify the intersection is on the ray by using the inner product.
    const Number s = (intersection.x() - source.x()) * (target.x() - source.x());
    if (s < 0)
      continue;

    intersections[num] = intersection;
    intersection_t[num] = t;
    ++num;
  }

  return num;
}




/**@class BezierNecklaceVisitor
 * @brief The base class to visit Bezier spline necklace shape types.
 */

/**@fn virtual void BezierNecklaceVisitor::Visit(BezierCurve& curve);
 * @brief Visit a Bezier spline curve.
 * @param curve the curve to visit.
 */


/**@class BezierNecklace
 * @brief A cubic Bezier curve necklace.
 *
 * Note that for this necklace, the kernel must be set explicitly.
 */

BezierNecklace::BezierNecklace(const Point& kernel) :
  kernel_(kernel),
  checked_(false),
  winding_(CGAL::COLLINEAR),
  bounding_box_()
{}

const Point& BezierNecklace::kernel() const
{
  return kernel_;
}

bool BezierNecklace::IsValid() const
{
  // Criteria:
  // * no degenerate curves,
  // * unobstructed vision of the full length of each curve,
  // * traversing each curve is a counterclockwise sweep around the kernel,
  // * the set of curves makes a closed curve (this implies there are curves).
  if (IsEmpty() || winding_ != CGAL::COUNTERCLOCKWISE)
    return false;

  bool valid = true;
  Point current = curves_.back().target();
  for (const BezierCurve& curve : curves_)
  {
    valid &=
      curve.IsValid(kernel()) &
      curve.source() == current;
    current = curve.target();
  }

  return valid;
}

bool BezierNecklace::IsEmpty() const
{
  return curves_.empty();
}

bool BezierNecklace::IsClosed() const
{
  return curves_.front().source() == curves_.back().target();
}

bool BezierNecklace::IntersectRay(const Number& angle_rad, Point& intersection) const
{
  // Find the curve that contains the angle.
  CHECK(checked_);
  CurveSet::const_iterator curve_iter = FindCurveContainingAngle(angle_rad);
  if (curve_iter == curves_.end())
    return false;

  Number t;
  return IntersectRay(angle_rad, curve_iter, intersection, t);
}

void BezierNecklace::AppendCurve
(
  const Point& source,
  const Point& source_control,
  const Point& target_control,
  const Point& target
)
{
  // Check the winding of the curve.
  if (winding_ == CGAL::COLLINEAR)
    winding_ = CGAL::orientation(source, source_control, kernel());
  else
    CHECK_EQ(winding_, CGAL::orientation(source, source_control, kernel()));

  // Clockwise curves are reversed.
  // Note that the order of the curves will be reversed when finalizing the spline.
  if (winding_ == CGAL::CLOCKWISE)
    curves_.emplace_back(target, target_control, source_control, source);
  else
    curves_.emplace_back(source, source_control, target_control, target);
}

// Note, cannot be the first curve.
void BezierNecklace::AppendCurve(const Point& source_control, const Point& target_control, const Point& target)
{
  CHECK(!curves_.empty());
  const Point& source = curves_.back().target();
  AppendCurve(source, source_control, target_control, target);
}

void BezierNecklace::Finalize()
{
  // Reorder the curves to start with the curve directly to the right of the kernel.
  // Note that this automatically corrects the winding of the spline to counterclockwise.
  std::sort(curves_.begin(), curves_.end(), CompareBezierCurves(*this));
  winding_ = CGAL::COUNTERCLOCKWISE;

  checked_ = true;
}

Box BezierNecklace::ComputeBoundingBox() const
{
  if (bounding_box_.xmax() <= bounding_box_.xmin() || bounding_box_.ymax() <= bounding_box_.ymin())
  {
    // Computing the exact bounding box is more complex than required.
    // There are several obvious approaches to interpolate the bounding box (with its own disadvantages):
    // * sampling each curve (expensive for many short curves),
    // * sampling angles around the kernel (may miss small curves, expensive/complex curve selection),
    // * taking the bounding box of the set of control points (approximation may be very rough).
    // We choose the last approach, because overestimating the bounding box is more desirable than underestimating it.
    Kernel::Construct_bbox_2 bbox = Kernel().construct_bbox_2_object();
    for (const BezierCurve& curve : curves_)
      bounding_box_ += bbox(curve.source()) + bbox(curve.source_control()) + bbox(curve.target_control()) + bbox(curve.target());
  }

  return bounding_box_;
}

Number BezierNecklace::ComputeCoveringRadiusRad(const Range::Ptr& range, const Number& radius) const
{
  if (radius == 0)
    return 0;

  // Sample the range and determine the largest covering radius, i.e. the largest angle difference towards the point on the spline at a fixed distance.
  // There are several viable sampling strategies (with evaluation):
  // - fixed angle difference (sensitive to spline curvature, i.e. low curvature means oversampling)
  // - fixed distance (sensitive to spline curvature, i.e. low curvature means oversampling)
  // - fixed sample size per curve (sensitive to curve length, i.e. short curves means oversampling)
  // - fixed sample size per range (sensitive to range length, i.e. short range means oversampling)
  // - binary search on range (as previous and loses benefits of moving curve-at-distance together with sample)
  // We chose for the fixed sample size per curve because the trade-off between accuracy and sampling size seemed reasonable.
  // Taking five samples per curve (t = {0, 1/4, 1/2, 3/4, 1}) captures the extreme curvature parts of each Cubic spline.

  CHECK(checked_);
  const CurveSet::const_iterator curve_iter_from = FindCurveContainingAngle(range->from());
  const CurveSet::const_iterator curve_iter_to = FindCurveContainingAngle(range->to());
  CHECK(curve_iter_from != curves_.end());
  CHECK(curve_iter_to != curves_.end());

  Number t_from, t_to;
  Point point;
  CHECK(IntersectRay(range->from(), curve_iter_from, point, t_from));
  CHECK(IntersectRay(range->to(), curve_iter_to, point, t_to));

  const Number t_step = 0.25;
  Number t = t_from;
  Number covering_radius_rad = 0;
  for (CurveSet::const_iterator curve_iter = curve_iter_from; curve_iter != curve_iter_to || t <= t_to;)
  {
    point = curve_iter->Evaluate(t);
    const Number angle_rad = ComputeAngleRad(point);

    Number angle_ccw, angle_cw;
    CHECK(ComputeAngleAtDistanceRad(point, radius, curve_iter, t, angle_ccw));
    CHECK(ComputeAngleAtDistanceRad(point, -radius, curve_iter, t, angle_cw));

    const Number covering_radius_rad_ccw = CircularRange(angle_rad, angle_ccw).ComputeLength();
    const Number covering_radius_rad_cw = CircularRange(angle_cw, angle_rad).ComputeLength();

    covering_radius_rad = std::max(covering_radius_rad, covering_radius_rad_ccw);
    covering_radius_rad = std::max(covering_radius_rad, covering_radius_rad_cw);

    t += t_step;
    if (curve_iter == curve_iter_to && t_to < t && t < t_to + t_step)
      t = t_to;  // Final step at the exact range endpoint.
    if (curve_iter != curve_iter_to && 1 <= t)
    {
      t = 0;
      if (++curve_iter == curves_.end())
        curve_iter = curves_.begin();
    }
  }

  return covering_radius_rad;
}

Number BezierNecklace::ComputeAngleAtDistanceRad(const Number& angle_rad, const Number& distance) const
{
  if (distance == 0)
    return angle_rad;

  // Find the curve that contains the angle.
  CHECK(checked_);
  const CurveSet::const_iterator curve_iter = FindCurveContainingAngle(angle_rad);
  if (curve_iter == curves_.end())
    return false;

  // Find the angle at the specified distance.
  Point point;
  Number t;
  CHECK(IntersectRay(angle_rad, curve_iter, point, t));
  Number angle_out_rad;
  const bool correct = ComputeAngleAtDistanceRad(point, distance, curve_iter, t, angle_out_rad);
  return correct ? angle_out_rad : angle_rad;
}

void BezierNecklace::Accept(NecklaceShapeVisitor& visitor)
{
  CHECK(checked_);
  visitor.Visit(*this);
}

void BezierNecklace::Accept(BezierNecklaceVisitor& visitor)
{
  CHECK(checked_);
  visitor.Visit(*this);
  for (BezierCurve& curve : curves_)
    visitor.Visit(curve);
}

void BezierNecklace::IterateCurves(BezierNecklaceVisitor& visitor)
{
  CHECK(checked_);
  for (BezierCurve& curve : curves_)
    visitor.Visit(curve);
}

BezierNecklace::CurveSet::const_iterator BezierNecklace::FindCurveContainingAngle(const Number& angle_rad) const
{
  CurveSet::const_iterator curve_iter = std::lower_bound
  (
    curves_.begin(),
    curves_.end(),
    Modulo(angle_rad),
    CompareBezierCurves(*this)
  );
  return curve_iter == curves_.end() ? curves_.begin() : curve_iter;
}

bool BezierNecklace::IntersectRay(const Number& angle_rad, const CurveSet::const_iterator& curve_iter, Point& intersection, Number& t) const
{
  const Point target = kernel() + Vector(std::cos(angle_rad), std::sin(angle_rad));

  Point intersections[3];
  Number intersection_t[3];
  const size_t num_intersection = curve_iter->IntersectRay(kernel(), target, intersections, intersection_t);
  if (num_intersection == 0)
    return false;

  // Note that the set of Bezier curves must always be a star-shaped curve with the kernel as star point,
  // meaning that a line through the kernel has at most one intersection with the curve.
  CHECK_EQ(num_intersection, 1);
  intersection = intersections[0];
  t = intersection_t[0];
  return true;
}

bool BezierNecklace::ComputeAngleAtDistanceRad
(
  const Point& point,
  const Number& distance,
  const CurveSet::const_iterator& curve_point,
  const Number& t_point,
  Number& angle_rad
) const
{
  // Find the curve that contains the distance.
  const Number squared_distance = distance * distance;
  CurveSet::const_iterator curve_iter = curve_point;
  Number t_start = t_point;
  do
  {
    if (0 < distance)
    {
      if (squared_distance <= CGAL::squared_distance(point, curve_iter->target()))
      {
        angle_rad = SearchCurveForAngleAtDistanceRad(point, *curve_iter, squared_distance, CGAL::COUNTERCLOCKWISE, t_start);
        return true;
      }

      if (++curve_iter == curves_.end())
        curve_iter = curves_.begin();
      t_start = 0;
    }
    else
    {
      if (squared_distance <= CGAL::squared_distance(point, curve_iter->source()))
      {
        angle_rad = SearchCurveForAngleAtDistanceRad(point, *curve_iter, squared_distance, CGAL::CLOCKWISE, t_start);
        return true;
      }

      if (curve_iter == curves_.begin())
        curve_iter = curves_.end();
      --curve_iter;
      t_start = 1;
    }
  }
  while (curve_iter != curve_point);

  // No curve exists for which either endpoint is far enough away.
  // Note that while curve may contain a distant enough point in its interior,
  // this case will not occur when calculating the scale factor, because the distance is limited to be much smaller than the curve length.
  return false;
}

Number BezierNecklace::SearchCurveForAngleAtDistanceRad
(
  const Point& point,
  const BezierCurve& curve,
  const Number& squared_distance,
  const CGAL::Orientation& orientation,
  const Number& t_start
) const
{
  // Perform a binary search on the curve to estimate the point at the specified distance.
  // The assumption is that the distance between the point and a sample on the curve is monotonic in t.
  // This assumption is based on the assumption that the Bezier spline is not too far from circular.
  Number lower_bound = t_start;
  Number upper_bound = orientation == CGAL::COUNTERCLOCKWISE ? 1 : 0;
  Point point_upper = curve.Evaluate(upper_bound);
  Number squared_distance_upper = CGAL::squared_distance(point, point_upper);
  while (squared_distance * kDistanceRatioEpsilon < squared_distance_upper)
  {
    const Number t = 0.5 * (lower_bound + upper_bound);
    const Point point_t = curve.Evaluate(t);
    const Number squared_distance_t = CGAL::squared_distance(point, point_t);
    CHECK_LE(squared_distance_t, squared_distance_upper);

    if (squared_distance_t < squared_distance)
    {
      lower_bound = t;
    }
    else
    {
      upper_bound = t;
      point_upper = point_t;
      squared_distance_upper = squared_distance_t;
    }
  }

  // The upper bound is the closest t for which the distance is confirmed to be larger than the specified distance.
  return ComputeAngleRad(point_upper);
}

} // namespace necklace_map
} // namespace geoviz