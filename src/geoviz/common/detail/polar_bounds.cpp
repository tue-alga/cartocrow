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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 26-02-2021
*/

#include "polar_bounds.h"

#include <cmath>

#include <glog/logging.h>


namespace geoviz
{

Box ConstructBoundingBox(const PolarSegment& segment)
{
  Kernel::Construct_bbox_2 bbox = Kernel().construct_bbox_2_object();
  const Box bounding_box = bbox(segment.Evaluate(0).to_cartesian()) + bbox(segment.Evaluate(1).to_cartesian());
  return bounding_box;
}

/**@brief Construct a minimum bounding box of a logarithmic spiral.
 *
 * Only the part of the spiral between the anchor and the pole is incorporated.
 * @param spiral the logarithmic spiral to bound.
 * @return the minimum bounding box.
 */
Box ConstructBoundingBox(const Spiral& spiral)
{
  // The bounding box is based on 5 points: the anchor point and the first four points where the tangent of the spiral is parallel to an axis, i.e. 0, pi/2, pi, and 3/2 pi.
  //
  // By definition, the tangent of the spiral has these properties when phi(t) = b + k * pi/2 (where k is an integer).
  // phi(t) = phi(0) + tan(b)*t = b + k*pi/2
  //    tan(b)*t = b + k*pi/2 - phi(0)
  //    t = (b + k*pi/2 - phi(0)) / tan(b)
  //    t = (b - phi(0)) / tan(b) + k*pi/(2*tan(b))

  Kernel::Construct_bbox_2 bbox = Kernel().construct_bbox_2_object();
  Box bounding_box = bbox(Point(CGAL::ORIGIN)) + bbox(spiral.anchor().to_cartesian());

  const Number tan_b = std::tan(spiral.angle_rad());
  const Number period = std::abs(M_PI / (2 * tan_b));

  Number t = (spiral.angle_rad() - spiral.anchor().phi()) / tan_b;

  // Make sure that we start at the instance farthest from the pole.
  while (0 < t)
    t -= period;
  while (t < 0)
    t += period;

  for (int k = 0; k < 4; ++k)
  {
    const PolarPoint p = spiral.Evaluate(t + k * period);
    bounding_box += bbox(p.to_cartesian());
  }

  return bounding_box;
}

/**@brief Construct a minimum bounding box of a logarithmic spiral segment.
 * @param spiral the logarithmic spiral segment to bound.
 * @return the minimum bounding box.
 */
Box ConstructBoundingBox(const SpiralSegment& segment)
{
  // The bounding box is based on up to 6 points: the anchor point, the point at R_min, and the first four points where the tangent of the spiral is parallel to an axis, i.e. 0, pi/2, pi, and 3/2 pi, but only those that have R_min < R < R_max.
  //
  // By definition, the tangent of the spiral has these properties when phi(t) = b + k * pi/2 (where k is an integer).
  // phi(t) = phi(0) + tan(b)*t = b + k*pi/2
  //    tan(b)*t = b + k*pi/2 - phi(0)
  //    t = (b + k*pi/2 - phi(0)) / tan(b)
  //    t = (b - phi(0)) / tan(b) + k*pi/(2*tan(b))

  Kernel::Construct_bbox_2 bbox = Kernel().construct_bbox_2_object();
  Box bounding_box = bbox(segment.near().to_cartesian()) + bbox(segment.far().to_cartesian());

  const Number tan_b = std::tan(segment.angle_rad());
  const Number period = std::abs(M_PI / (2 * tan_b));

  Number t = (segment.angle_rad() - segment.anchor().phi()) / tan_b;

  // Make sure that we start at the instance farthest from the pole.
  while (0 < t)
    t -= period;
  while (t < 0)
    t += period;

  for (int k = 0; k < 4; ++k)
  {
    const PolarPoint p = segment.Evaluate(t + k * period);
    if (segment.R_min() < p.R() && p.R() < segment.anchor().R())
      bounding_box += bbox(p.to_cartesian());
  }

  return bounding_box;
}

} // namespace geoviz
