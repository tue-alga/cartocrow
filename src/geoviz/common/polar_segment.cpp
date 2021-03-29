/*
The GeoViz library implements algorithmic geo-visualization methods,
developed at TU Eindhoven.
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 19-02-2021
*/

#include "polar_segment.h"

#include <glog/logging.h>


namespace geoviz
{

/**@class PolarSegment
 * @brief A straight line segment with polar point coordinates.
 */

/**@brief Construct a line segment connecting two polar points.
 * @param point_1 one of the endpoints of the line.
 * @param point_2 the other endpoint of the line.
 */
PolarSegment::PolarSegment(const PolarPoint& point_1, const PolarPoint& point_2) :
  PolarLine()
{
  // The multiplier and offset are used to transform a time T within [0, 1] to evaluate to a point on the line segment by evaluating at t = multiplier * T - offset.
  multiplier_ = SetFoot(point_1, point_2);
  offset_ = (point_1.R() * point_1.R() - point_2.R() * point_2.R() + multiplier_ * multiplier_) / (2 * multiplier_);
}

// TODO(tvl) remove.
Number PolarSegment::FromT() const
{
  return ToDistance(0);
}

// TODO(tvl) remove.
Number PolarSegment::ToT() const
{
  return ToDistance(1);
}

/**@brief Check whether the line is a left line.
 *
 * A left line moves in clockwise direction when moving from point(0) to point(1).
 * @return whether the line is a left line.
 */
bool PolarSegment::IsLeft() const
{
  return 0 < foot().R() && multiplier_ < 0;
}

/**@brief Check whether the line is a right line.
 *
 * A left line moves in counter-clockwise direction when moving from point(0) to point(1).
 * @return whether the line is a right line.
 */
bool PolarSegment::IsRight() const
{
  return 0 < foot().R() && 0 < multiplier_;
}

/**@brief Check whether the line is collinear with the pole.
 * @return whether the line is collinear with the pole.
 */
bool PolarSegment::IsCollinear() const
{
  return 0 == foot().R();
}

bool PolarSegment::ContainsFoot() const
{
  if (ContainsPhi(foot().phi()))
    return true;
}

/**@brief Check whether the line segment contains a point evaluated at a given time.
 * @param t the time to evaluate the line segment.
 * @return whether the line segment contains a point at the desired time.
 */
bool PolarSegment::ContainsT(const Number& t) const
{
  return 0 <= t && t <= 1;
}

/**@brief Check whether the line segment contains a point at a given distance from the pole.
 * @param R the given distance from the pole.
 * @return whether the line segment contains any point within the desired distance from the pole.
 */
bool PolarSegment::ContainsR(const Number& R) const
{
  // Compare to the endpoints.
  if (EvaluateR(0) < R && EvaluateR(1) < R)
    return false;

  // Compare the closest point.
  return ComputeClosestToPole().R() <= R;
}

/**@brief Check whether the line segment contains any point with a given phi coordinate.
 * @param phi the desired phi coordinate.
 * @return whether the line segment contains any point with the given phi coordinate.
 */
bool PolarSegment::ContainsPhi(const Number& phi) const
{
  if (!PolarLine::ContainsPhi(phi))
    return false;

  const Number t = ComputeT(phi);
  return ContainsT(t);
}

Number PolarSegment::EvaluateR(const Number& t) const
{
  const Number distance = ToDistance(t);
  return PolarLine::EvaluateR(distance);
}

Number PolarSegment::EvaluatePhi(const Number& t) const
{
  const Number distance = ToDistance(t);
  return PolarLine::EvaluatePhi(distance);
}

PolarPoint PolarSegment::Evaluate(const Number& t) const
{
  const Number distance = ToDistance(t);
  return PolarLine::Evaluate(distance);
}

/**@brief Compute the time value of a point on the line segment at a given phi.
 *
 * Note, throws an exception if phi is not contained in the line segment.
 * @param phi the phi of the point.
 * @return the time value of the point on the line. Points on the line segment have t between 0 and 1 (inclusive).
 */
Number PolarSegment::ComputeT(const Number& phi) const
{
  const Number distance = PolarLine::ComputeT(phi);
  return ToT(distance);
}

/**@brief Compute the point on the line segment closest to the pole.
 *
 * Note that this point may be an endpoint if the closest point of the supporting line is not contained in the line segment.
 * @return the point on the line segment closest to the pole.
 */
PolarPoint PolarSegment::ComputeClosestToPole() const
{
  // Note that the closest point on the supporting line may not lie inside the segment.
  if (ContainsPhi(foot().phi()))
    return foot();

  const PolarPoint p0 = Evaluate(0);
  const PolarPoint p1 = Evaluate(1);
  return p0.R() < p1.R() ? p0 : p1;
}

/**@brief access the supporting line of this line segment.
 * @return the supporting line.
 */
const PolarLine& PolarSegment::SupportingLine() const
{
  return *this;
}

Number PolarSegment::ToDistance(const Number& t) const
{
  return multiplier_ * t - offset_;
}

Number PolarSegment::ToT(const Number& distance) const
{
  return (distance + offset_) / multiplier_;
}


std::ostream& operator<<(std::ostream& os, const PolarSegment& line)
{
  os << "s[" << line.Evaluate(0).to_cartesian() << ", " << line.Evaluate(1).to_cartesian() << "]";
  return os;
}

} // namespace geoviz
