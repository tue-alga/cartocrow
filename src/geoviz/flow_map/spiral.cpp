/*
The Flow Map library implements the algorithmic geo-visualization
method by the same name, developed by Kevin Verbeek, Kevin Buchin,
and Bettina Speckmann at TU Eindhoven
(DOI: 10.1007/s00453-013-9867-z & 10.1109/TVCG.2011.202).
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 16-10-2020
*/

#include "spiral.h"

#include <cmath>

#include <glog/logging.h>


namespace geoviz
{
namespace flow_map
{

/**@class Spiral
 * @brief A logarithmic spiral.
 *
 * The polar coordinates of the points on the spiral p(t) = (R(t), phi(t)) are R(t) = R(0)*e^{-t}, and phi(t) = phi(0) + tan(alpha) * t, where p(0) = (R(0), phi(0)) is the position of the spiral's anchor.
 *
 * The special case where the angle is 0, this spiral is a straight line instead.
 */

/**@fn Spiral::Ptr
 * @brief The preferred pointer type for storing or sharing a logarithmic spiral.
 */

/**@brief Construct a new logarithmic spiral.
 * @param angle_rad the alpha used to compute the phi coordinate of the point on the logarithmic spiral.
 * @param anchor the point on the logarithmic spiral at t = 0. This point cannot be the root, because then the spiral could not be determined uniquely.
 */
Spiral::Spiral(const Number& angle_rad, const PolarPoint& anchor) :
  angle_rad_(angle_rad),
  anchor_(anchor)
{
  CHECK_NE(anchor_.R(), 0);
}

/**@brief Construct a logarithmic spiral that connects two points.
 *
 * A logarithmic spiral cannot connect two points that are equidistant from the root.
 *
 * Note, this constructor will only construct a straight line spiral if the source and target are collinear with the root.
 * @param source the point on the logarithmic spiral at t = 0. This point cannot be the root, because then the spiral could not be determined uniquely.
 * @param target another point on the logarithmic spiral. This point must have a different distance to the root than the source.
 */
Spiral::Spiral(const PolarPoint& source, const PolarPoint& target) :
  anchor_(source)
{
  // Computing a spiral through two points (p, q), with unknown angle (alpha):
  //
  // p = (R_p, phi_p)  ->  R(0) = R_p; phi(0) = phi_p
  // q = (R_q, phi_q)  ->  R(t) = R_p * e^{-t}; phi(t) = phi_p + tan(alpha) * t
  //
  // R(0) = R_p * e^0 = R_p
  // phi(0) = phi_p + tan(alpha) * 0 = phi_p
  // Assuming R_q < R_p and 0 < t:
  //    R(t) = R_q = R_p * e^{-t}  =>  e^{-t} = R_q / R_p  =>  t = -ln(R_q / R_p)
  //    phi(t) = phi_q = phi_p + tan(alpha) * t  =>  tan(alpha) = (phi_q - phi_p) / t  =>  alpha = tan^-1((phi_q - phi_p) / t)
  //
  //    =>  alpha = tan^-1((phi_q - phi_p) / -ln(R_q / R_p))

  CHECK_LT(0, source.R());
  CHECK_LT(target.R(), source.R());

  if (target.R() == 0)
  {
    // The target is the root.
    angle_rad_ = 0;
    return;
  }

  Number diff_phi = target.phi() - source.phi();
  if (diff_phi < -M_PI)
    diff_phi += M_2xPI;
  else if (M_PI < diff_phi)
    diff_phi -= M_2xPI;

  angle_rad_ = std::atan(diff_phi / -std::log(target.R() / source.R()));
}

/**@brief Get the polar angle of the spiral's tangents (in radians).
 *
 * A feature of logarithmic spirals is that the angle between a line through the root and the tangent of the spiral where that line intersects the spiral is constant.
 * @return the angle of the spiral.
 */
const Number& Spiral::angle_rad() const
{
  return angle_rad_;
}

/**@brief Get the anchor point of the spiral.
 *
 * The anchor point is the point on the spiral evaluated at t=0.
 * @return the anchor point.
 */
const PolarPoint& Spiral::anchor() const
{
  return anchor_;
}

/**@brief Check whether the spiral is a left spiral.
 *
 * A left spiral moves in counter-clockwise direction as t increases.
 * @return whether the spiral is a left spiral.
 */
bool Spiral::IsLeft() const { return angle_rad_ < 0; }

/**@brief Check whether the spiral is a right spiral.
 *
 * A right spiral moves in clockwise direction as t increases.
 * @return whether the spiral is a right spiral.
 */
bool Spiral::IsRight() const { return 0 < angle_rad_; }

/**@brief Check whether the spiral is a straight line.
 * @return whether the spiral is a straight line.
 */
bool Spiral::IsStraight() const { return angle_rad_ == 0; }

/**@brief Compute the point on the spiral at a specific time.
 * @param t the time at which to evaluate the spiral.
 * @return the point on the spiral at time t.
 */
PolarPoint Spiral::Evaluate(const Number& t) const
{
  return PolarPoint(anchor_.R() * std::exp(-t), anchor_.phi() + std::tan(angle_rad_) * t);
}

/**@brief Computes a number that can be used to sort spirals around the root.
 *
 * While a single catalog number could be any number, comparing the catalog numbers of two spirals produces their local order.
 *
 * A spiral that has a smaller catalog number than another spiral is locally clockwise of that spiral and a larger catalog number indicates the spiral is locally counter-clockwise.
 *
 * Two spirals with equal catalog number overlap. However, these spirals may have different anchor points.
 *
 * Note that when comparing spirals globally, any two spirals are both clockwise and counter-clockwise of each other. This catalog number cuts the circle at some arbitrary point to remove this circularity. This means that the spiral with the largest catalog number will, in practice, be locally counter-clockwise of the spiral with the smallest catalog number.
 *
 * @return the sorting number.
 */
Number Spiral::ComputeOrder() const
{
  // We choose as catalog number, the angle where the spiral intersects the unit circle.
  //
  // R(t) = 1 = R(0) * e^{-t}  =>  1 / R(0) = e^{-t}  =>  t = -ln(1 / R(0))
  // phi(t) = phi(0) + tan(alpha) * t  =>  phi(t) = phi(0) - tan(alpha) * ln(1 / R(0))
  return anchor_.phi() - std::tan(angle_rad_) * std::log(1 / anchor_.R());
}

/**@brief Computes the intersection with another logarithmic spiral.
 *
 * Also note, two spirals have either 0 or an infinite number of intersections. Two spirals do not intersect if they have equal alpha.
 *
 * If the spirals intersect, this method returns the intersection farthest from the root that is closer to the root than this spiral's anchor.
 * @param s the logarithmic spiral to intersect.
 * @return the intersection of the spirals.
 */
PolarPoint Spiral::Intersect(const Spiral& s) const
{
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

  const Number tan_alpha_1 = std::tan(angle_rad_);
  const Number tan_alpha_2 = std::tan(s.angle_rad_);
  CHECK_NE(tan_alpha_1, tan_alpha_2);

  // Determine the time to spend on the other spiral to reach the same distance from the root.
  const Number d_t_2 = std::log(s.anchor_.R() / anchor_.R());

  // Determine the difference in angle at this time.
  const Number d_phi = s.anchor_.phi() - anchor_.phi() + tan_alpha_2 * d_t_2;

  // Determine the amount that d_phi changes per t.
  const Number ddt_phi = tan_alpha_1 - tan_alpha_2;

  // Remember that the spirals have an infinite number of intersections;
  // we want the one farthest from the root for which 0 < t.
  const Number t_1 = (d_phi < 0 == ddt_phi < 0) ? d_phi / ddt_phi : (d_phi - M_2xPI) / ddt_phi;
  CHECK_GE(t_1, 0);
  CHECK_LE(t_1, std::abs(M_2xPI / tan_alpha_1));

  return Evaluate(t_1);
}

/**@brief Construct a minimum bounding box of the spiral.
 *
 * Only the part of the spiral between the anchor and the root is incorporated.
 * @return the minimum bounding box.
 */
Box Spiral::ComputeBoundingBox() const
{
  // The bounding box is based on 5 points: the anchor point and the first four points where the tangent of the spiral is parallel to an axis, i.e. 0, pi/2, pi, and 3/2 pi.
  //
  // By definition, the tangent of the spiral has these properties when phi(t) = b + k * pi/2 (where k is an integer).
  // phi(t) = phi(0) + tan(b)*t = b + k*pi/2
  //    tan(b)*t = b + k*pi/2 - phi(0)
  //    t = (b + k*pi/2 - phi(0)) / tan(b)
  //    t = (b - phi(0)) / tan(b) + k*pi/(2*tan(b))

  Kernel::Construct_bbox_2 bbox = Kernel().construct_bbox_2_object();
  Box bounding_box = bbox(Point(CGAL::ORIGIN)) + bbox(anchor_.to_cartesian());

  const Number tan_b = std::tan(angle_rad_);
  const Number period = std::abs(M_PI / (2 * tan_b));

  Number t = (angle_rad_ - anchor_.phi()) / tan_b;

  // Make sure that we start at the instance farthest from the root.
  while (0 < t)
    t -= period;
  while (t < 0)
    t += period;

  for (int k = 0; k < 4; ++k)
  {
    const PolarPoint p = Evaluate(t + k * period);
    bounding_box += bbox(p.to_cartesian());
  }

  return bounding_box;
}

/**@fn Number Spiral::angle_rad_
 * @brief The alpha used to compute the phi coordinate of the point on the logarithmic spiral.
 */

/**@fn PolarPoint Spiral::anchor_
 * @brief The point on the logarithmic spiral at t = 0.
 */

} // namespace flow_map
} // namespace geoviz
