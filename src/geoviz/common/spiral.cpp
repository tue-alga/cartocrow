/*
The GeoViz library implements algorithmic geo-visualization methods,
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 16-10-2020
*/

#include "spiral.h"

#include <cmath>

#include <glog/logging.h>


namespace geoviz
{

/**@class Spiral
 * @brief A logarithmic spiral.
 *
 * The polar coordinates of the points on the spiral p(t) = (R(t), phi(t)) are R(t) = R(0)*e^{-t}, and phi(t) = phi(0) + tan(alpha) * t, where p(0) = (R(0), phi(0)) is the position of the spiral's anchor.
 *
 * The special case where the angle is 0, this spiral is a straight line instead.
 */

/**@brief Construct a logarithmic spiral.
 * @param anchor the point on the logarithmic spiral at t = 0. This point cannot be the pole, because then the spiral could not be determined uniquely.
 * @param angle_rad the alpha used to compute the phi coordinate of the point on the logarithmic spiral.
 */
Spiral::Spiral(const PolarPoint& anchor, const Number& angle_rad) :
  angle_rad_(angle_rad),
  anchor_(anchor)
{
  CHECK_LT(0, anchor_.R());
}

/**@brief Construct a logarithmic spiral containing two points.
 *
 * A logarithmic spiral cannot connect two points that are equidistant from the pole. For any other combination of point, an infinite number of spirals exist that connect them. This constructors the shortest spiral connecting the two points.
 *
 * This will only construct a straight line spiral if the source and target are collinear with the pole.
 *
 * The input point farthest from the pole will become the point on the logarithmic spiral at t = 0.
 * @param point_1 one of the points on the logarithmic spiral.
 * @param point_2 the other point on the logarithmic spiral.
 */
Spiral::Spiral(const PolarPoint& point_1, const PolarPoint& point_2)
{
  const PolarPoint& source = point_1.R() < point_2.R() ? point_2 : point_1;
  const PolarPoint& target = point_1.R() < point_2.R() ? point_1 : point_2;
  CHECK_LT(0, source.R());
  CHECK_LT(target.R(), source.R());

  anchor_ = source;

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

  if (target.R() == 0)
  {
    // The target is the pole.
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

/**@brief Get the anchor point of the spiral.
 *
 * The anchor point is the point on the spiral evaluated at t=0.
 * @return the anchor point.
 */
const PolarPoint& Spiral::anchor() const
{
  return anchor_;
}

/**@brief Get the polar angle of the spiral's tangents (in radians).
 *
 * A feature of logarithmic spirals is that the angle between a line through the pole and the tangent of the spiral where that line intersects the spiral is constant.
 * @return the angle of the spiral.
 */
const Number& Spiral::angle_rad() const
{
  return angle_rad_;
}

/**@brief Check whether the spiral is a left spiral.
 *
 * A left spiral moves in clockwise direction as t increases.
 *
 * Note that t increases in the direction of the pole.
 * @return whether the spiral is a left spiral.
 */
bool Spiral::IsLeft() const { return angle_rad_ < 0; }

/**@brief Check whether the spiral is a right spiral.
 *
 * A right spiral moves in counter-clockwise direction as t increases.
 *
 * Note that t increases in the direction of the pole.
 * @return whether the spiral is a right spiral.
 */
bool Spiral::IsRight() const { return 0 < angle_rad_; }

/**@brief Check whether the spiral is a straight line collinear with the pole.
 * @return whether the spiral is a straight line collinear with the pole.
 */
bool Spiral::IsCollinear() const { return angle_rad_ == 0; }

/**@brief Compute the distance between the point on the spiral at a specific time and the pole.
 * @param t the time at which to evaluate the spiral.
 * @return the distance between the point on the spiral at time t and the pole.
 */
Number Spiral::EvaluateR(const Number& t) const
{
  return anchor_.R() * std::exp(-t);
}

/**@brief Compute the phi of the point on the spiral at a specific time.
 * @param t the time at which to evaluate the spiral.
 * @return the phi of the point on the spiral at time t.
 */
Number Spiral::EvaluatePhi(const Number& t) const
{
  // phi(t) = phi(0) + tan(alpha) * t
  return Modulo(anchor_.phi() + std::tan(angle_rad_) * t);
}

/**@brief Compute the point on the spiral at a specific time.
 * @param t the time at which to evaluate the spiral.
 * @return the point on the spiral at time t.
 */
PolarPoint Spiral::Evaluate(const Number& t) const
{
  return PolarPoint(EvaluateR(t), EvaluatePhi(t));
}

/**@brief Compute the time at which the spiral is a specific distance from the pole.
 * @param R the distance from the pole.
 * @return the time at which the spiral has the desired distance from the pole.
 */
Number Spiral::ComputeT(const Number& R) const
{
  CHECK_LT(0, R);
  // R(t) = R = R(0) * e^{-t}  =>  R / R(0) = e^{-t}  =>  t = -ln(R / R(0))
  return -std::log(R / anchor_.R());
}

/**@brief Compute the phi of a point on the spiral at a specific distance from the pole.
 * @param R the distance from the pole.
 * @return the phi of the point on the spiral at a specific distance from the pole.
 */
Number Spiral::ComputePhi(const Number& R) const
{
  const Number t = ComputeT(R);
  return EvaluatePhi(t);
}

/**@brief Compute the time at which the spiral is a specific phi.
 *
 * Note that a spiral with non-zero angle will pass the same phi infinitely many times.
 * @param phi the desired phi.
 * @return the time at which the spiral has the desired phi.
 */
Number Spiral::SampleT(const Number& phi) const
{
  return Modulo(phi - anchor_.phi(), -M_PI) / std::tan(angle_rad_);
}

/**@brief Compute an R of a point on the spiral at a given phi.
 *
 * Note that a spiral with non-zero angle will pass the same phi infinitely many times.
 * @param phi the desired phi.
 * @return an R of a point on the spiral at phi.
 */
Number Spiral::SampleR(const Number& phi) const
{
  const Number t = SampleT(phi);
  return EvaluateR(t);
}

/**@brief Compute the angular period of the spiral.
 *
 * The angular period is the t to move on the spiral to reach a point with the same phi as where you started.
 * @return the angular period.
 */
Number Spiral::ComputePeriod() const
{
  return M_2xPI / std::tan(angle_rad());
}

/**@brief Move the anchor on the spiral.
 *
 * @param R the distance from the pole of the new anchor.
 */
void Spiral::MoveAnchor(const Number& R)
{
  CHECK_LT(0, anchor_.R());
  const Number phi = ComputePhi(R);
  anchor_ = PolarPoint(R, phi);
}


std::ostream& operator<<(std::ostream& os, const Spiral& spiral)
{
  os << "S<@= " << spiral.anchor() << ", ang= " << spiral.angle_rad() << ">";
  return os;
}

} // namespace geoviz
