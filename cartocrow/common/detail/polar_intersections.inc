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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 03-03-2021
*/

#ifndef CARTOCROW_COMMON_DETAIL_POLAR_INTERSECTIONS_INC
#define CARTOCROW_COMMON_DETAIL_POLAR_INTERSECTIONS_INC

namespace cartocrow
{
namespace detail
{

int Orientation(const PolarLine& line, const PolarPoint& point);

bool BinarySearch(const PolarLine& line, const Spiral& spiral, Number& t_line_near, Number& t_line_far, const Number t_precision = Number(1e-15));


bool CheckIntersection(const Spiral& spiral, const PolarPoint& point);

bool CheckIntersection(const SpiralSegment& segment, const PolarPoint& point);

bool CheckIntersection(const PolarLine& line, const PolarPoint& point);

bool CheckIntersection(const PolarSegment& segment, const PolarPoint& point);


// Note, the class has the base classes as template arguments (i.e. Spiral or PolarLine),
// while the operator method is called directly using the specialized types.
// This is necessary to enable partial method specialization (for the output iterator)
// for the special case where the objects have to be reversed,
// i.e. spiral-line intersections are implemented as line-spiral intersections.
template <class Base1_, class Base2_>
class IntersectionFunctor
{
 public:
  template <class Type1_, class Type2_, typename OutputIterator>
  int operator()(const Type1_& object_1, const Type2_& object_2, OutputIterator intersections)
  {
    Number t_1[2];
    const int num_t = ComputeIntersectionT(object_1, object_2, t_1);

    int num = 0;
    for (int i = 0; i < num_t; ++i)
    {
      *intersections = object_1.Evaluate(t_1[i]);
      if (!CheckIntersection(object_1, *intersections) || !CheckIntersection(object_2, *intersections))
        continue;

      ++intersections;
      ++num;
    }

    return num;
  }
}; // class IntersectionFunctor

template <>
class IntersectionFunctor<Spiral, PolarLine> : public IntersectionFunctor<PolarLine, Spiral>
{
  using Base = IntersectionFunctor<PolarLine, Spiral>;

 public:
  template <class Type1_, class Type2_, typename OutputIterator>
  int operator()(const Type1_& object_1, const Type2_& object_2, OutputIterator intersections)
  {
    // Note that we reverse the objects when computing the intersections.
    return Base::operator()(object_2, object_1, intersections);
  }
}; // IntersectionFunctor<Spiral, PolarLine>

} // namespace detail


/**@brief Compute two times at which two logarithmic spirals intersect.
 *
 * Note, two spirals have either 0 or an infinite number of intersections. Two spirals do not intersect if they have equal angle.
 *
 * If the spirals intersect, this method returns the two intersections closest to the first spiral's anchor and on opposite sides of that anchor.
 * The first intersection has the non-positive time closest to zero (non-positive to account for an intersection at t = 0).
 * The second intersection has the positive time closest to zero.
 * @tparam OutputIterator the type of output iterator to which to send the output.
 * @param spiral_1 the first logarithmic spiral to intersect.
 * @param spiral_2 the second logarithmic spiral to intersect.
 * @param t_1 @parblock the intersection times on the first spiral.
 * These times are the smallest positive time and the largest non-positive time at which the spirals intersect.
 * @endparblock
 * @return the number of intersections.
 */
template <typename OutputIterator>
int ComputeIntersectionT(const Spiral& spiral_1, const Spiral& spiral_2, OutputIterator t_1)
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

  // Determine the amount that d_phi changes per t.
  const Number tan_alpha_1 = std::tan(spiral_1.angle_rad());
  const Number tan_alpha_2 = std::tan(spiral_2.angle_rad());
  const Number ddt_phi = tan_alpha_1 - tan_alpha_2;
  if (ddt_phi == 0)
    return 0;
  const Number t_period = std::abs(M_2xPI / ddt_phi);

  // Determine the time to spend on the second spiral to reach the same distance from the pole.
  const Number d_t_2 = std::log(spiral_2.anchor().R() / spiral_1.anchor().R());

  // Determine the difference in angle at this time.
  const Number d_phi = Modulo(spiral_2.anchor().phi() + tan_alpha_2 * d_t_2 - spiral_1.anchor().phi());
  if (d_phi == 0)
  {
    *t_1++ = t_period;
    *t_1++ = 0;
    return 2;
  }

  // Remember that the spirals have an infinite number of intersections;
  // we want the one farthest from the pole for which 0 < t.
  const Number t_1_positive = 0 < ddt_phi ? d_phi / ddt_phi : (d_phi - M_2xPI) / ddt_phi;
  CHECK_LT(0, t_1_positive);
  //CHECK_LE(t_1, std::abs(M_PI / tan_alpha_1));
  CHECK_LT(t_1_positive, t_period);

  *t_1++ = t_1_positive;
  *t_1++ = t_1_positive - t_period;

  return 2;
}

/**@brief Compute the time at which two lines with polar coordinates intersect.
 *
 * Note, two lines have either 0 or 1 intersections. Two lines do not intersect if the phi values of their feet differ by a multiple of pi.
 * @tparam OutputIterator the type of output iterator to which to send the output.
 * @param line_1 the first line with polar coordinates to intersect.
 * @param line_2 the second line with polar coordinates to intersect.
 * @param t_1 the time on the first line at which the lines intersect.
 * @return the number of intersections.
 */
template <typename OutputIterator>
int ComputeIntersectionT(const PolarLine& line_1, const PolarLine& line_2, OutputIterator t_1)
{
  // Computing the intersection is done by projecting the foot of the first line onto the pedal vector of the second line and converting the distance to travel to the foot of the second line to the distance on the first line.
  // Given the angle between pedal vectors phi_d and the vector lengths R_1 and R_2,
  // the foot of the first line is projected onto a point at signed distance d = R_1 * cos(phi_d);
  // the total distance to the foot of the second line is R_2 - d;
  // the signed (assuming phi_d is phi_2 - phi_1) distance between the foot of the first line and the intersection is
  // t_1 = (R_2 - d) / sin(pi - phi_d) = (R_2 - d) / sin(phi_d)
  // t_1 = (R_2 - R_1 * cos(phi_d)) / sin(phi_d)
  // t_1 = R_2 / sin(phi_d) - R_1 * cos(phi_d) / sin(phi_d) = R_2 / sin(phi_d) - R_1 / tan(phi_d)

  const Number phi_d = Modulo(line_2.foot().phi() - line_1.foot().phi());
  if (std::abs(phi_d) < Number(1e-15) || std::abs(phi_d - M_PI) < Number(1e-15))
    return 0;

  // Projection of first line.
  const Number t_project = line_1.foot().R() / std::tan(phi_d);

  // Pedal distance of second line.
  const Number t_pedal = line_2.foot().R() / std::sin(phi_d);

  *t_1++ = t_pedal - t_project;
  return 1;
}

/**@brief Compute two times at which a line with polar coordinates and a logarithmic spiral intersect.
 *
 * Note, a line with polar coordinates and a logarithmic spiral have either 0, 1, or an infinite number of intersections.
 *
 * If the spiral has a non-zero angle, they have an infinite number of intersections, one of which is the pole if the line contains the pole. This method returns the intersections that are closest to the anchor and on opposite sides of the anchor. If the anchor lies on the line, only that intersection is returned.
 *
 * If the spiral has angle 0, the number of intersections depend on whether the line contains the pole and on the angle A between the spiral half-line and the pedal vector of the line:
 *   If the line contains the pole: infinite number of intersections if A = pi/2 (the pole and the spiral's anchor are returned), else 1 intersection at the pole.
 *   If the line does not contain the pole: 1 intersection is A < pi/2, else 0 intersections.
 * @tparam OutputIterator the type of output iterator to which to send the output.
 * @param line the line with polar coordinates to intersect.
 * @param spiral the logarithmic spiral to intersect.
 * @param t_line @parblock the intersection times on the line.
 * These times are the positive and non-positive time at which the line and spiral intersect.
 * @endparblock
 * @return the number of intersections.
 */
template <typename OutputIterator>
int ComputeIntersectionT(const PolarLine& line, const Spiral& spiral, OutputIterator t_line)
{
  // Note that we must return the t on the line, because this is the only way in which we can return the pole.
  const Number& phi_line = line.foot().phi();
  const Number& phi_spiral = spiral.anchor().phi();

  if (spiral.angle_rad() == 0)
  {
    const Number phi_diff = Modulo(phi_spiral - phi_line, -M_PI);

    if (line.foot().R() == 0)
    {
      if (phi_diff == -M_PI_2)
      {
        // Overlapping in 'clockwise' direction from the foot.
        *t_line++ = 0;
        *t_line++ = -spiral.anchor().R();
        return 2;
      }
      else if (phi_diff == M_PI_2)
      {
        // Overlapping in 'counter-clockwise' direction from the foot.
        *t_line++ = spiral.anchor().R();
        *t_line++ = 0;
        return 2;
      }
      else
      {
        // 1 intersection at the pole.
        *t_line++ = 0;
        return 1;
      }
    }
    else
    {
      if (std::abs(phi_diff) < M_PI_2)
      {
        // 1 intersection at the spiral's phi.
        *t_line++ = line.ComputeT(phi_spiral);
        return 1;
      }
      else
      {
        // No intersections.
        return 0;
      }
    }
  }

  const int orientation_anchor = detail::Orientation(line, spiral.anchor());
  if (orientation_anchor == 0)
  {
    *t_line++ = line.ComputeT(spiral.anchor().phi());
    return 1;
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

  const Number period = spiral.ComputePeriod();
  const Number t_spiral_parallel = spiral.SampleT(phi_line + M_PI_2 + spiral.angle_rad());

  // We determine two reference times t_0 < t_1 such that an intersection point must lie between t_i and t_i + period / 2.
  // The point at the second reference time should lie on the same side of the line as the anchor.
  Number t_spiral[2];
  if (detail::Orientation(line, spiral.Evaluate(t_spiral_parallel)) == orientation_anchor)
    t_spiral[1] = t_spiral_parallel;
  else if (t_spiral_parallel < 0)
    t_spiral[1] = t_spiral_parallel + period / 2;
  else
    t_spiral[1] = t_spiral_parallel - period / 2;

  // The point at the first reference time should not lie on the same side of the line as the anchor.
  t_spiral[0] = t_spiral[1];
  do
    t_spiral[0] -= period / 2;
  while (detail::Orientation(line, spiral.Evaluate(t_spiral[0])) == orientation_anchor);

  int intersections = 0;
  for (int i = 0; i < 2; ++i)
  {
    Number t_spiral_far = t_spiral[i];
    Number t_spiral_near = t_spiral_far + period / 2;
    if (!detail::BinarySearch(line, spiral, t_spiral_near, t_spiral_far))
      continue;

    // Convert the point on the spiral to a point on the line.
    // Note that we must be careful of lines through (and near) the pole:
    // In either case, converting through phi is incorrect or inaccurate.
    const Number R_near = spiral.EvaluateR(t_spiral_near);
    Number t_line_near[2];
    const int num = line.CollectT(R_near, t_line_near);
    if (num == 2)
    {
      const Number phi_spiral_near = spiral.EvaluatePhi(t_spiral_near);
      const Number phi_line_near_0 = line.EvaluatePhi(t_line_near[0]);
      const Number phi_line_near_1 = line.EvaluatePhi(t_line_near[1]);

      if
      (
        std::abs(Modulo(phi_spiral_near - phi_line_near_0, -M_PI)) <
        std::abs(Modulo(phi_spiral_near - phi_line_near_1, -M_PI))
      )
        *t_line++ = t_line_near[0];
      else
        *t_line++ = t_line_near[1];
    }
    else
      *t_line++ = 0;

    ++intersections;
  }

  return intersections;
}


/**@brief Compute two times at which two objects with polar coordinates intersect.
 *
 * Each object can be either a Spiral, SpiralSegment, PolarLine, or PolarSegment. Any two such objects can intersect 0, 1, or an infinite number of times, depending on the combination of object types and their arrangement.
 *
 * If there are an infinite number of intersections, at least one of the objects must be a spiral type. In this case, the two intersections closest to the (first) spiral's anchor and on opposite sides of that anchor are returned. The other intersections may be computed by constructing similar spirals with their anchor at a different R, i.e. at a different period on the same spiral.
 * @tparam OutputIterator the type of output iterator to which to send the output.
 * @param spiral_1 the first object with polar coordinates to intersect.
 * @param spiral_2 the second object with polar coordinates to intersect.
 * @param intersections the intersection times on the first object.
 * @return the number of intersections.
 */
template <class Type1_, class Type2_, typename OutputIterator>
int ComputeIntersections(const Type1_& object_1, const Type2_& object_2, OutputIterator intersections)
{
  return detail::IntersectionFunctor<Type1_, Type2_>()(object_1, object_2, intersections);
}

} // namespace cartocrow

#endif //CARTOCROW_COMMON_DETAIL_POLAR_INTERSECTIONS_INC
