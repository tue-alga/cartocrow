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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 03-03-2020
*/

#include "compute_feasible_interval_wedge.h"

#include <glog/logging.h>


namespace geoviz
{
namespace necklace_map
{

/**@class ComputeFeasibleWedgeInterval
 * @brief A functor to generate feasible wedge intervals for necklace bead placement.
 *
 * The generated wedge interval is the intersection of the necklace and a wedge @f$W@f$, such that the apex of @f$W@f$ is the necklace kernel, @f$W@f$ contains a map region, and the inner angle of @f$W@f$ is minimal.
 *
 * If the region contains the necklace kernel, the wedge interval would cover the complete plane. In this case, a centroid interval in generated instead.
 */

NecklaceInterval::Ptr
ComputeFeasibleWedgeInterval::operator()(const Polygon& extent, const Necklace::Ptr& necklace) const
{
  CHECK_GT(extent.size(), 0);
  if (extent.size() == 1)
    return (*fallback_)(extent, necklace);

  const Number angle = necklace->shape->ComputeAngleRad(*extent.vertices_begin());
  NecklaceInterval obscured(angle, angle);

  const Point& kernel = necklace->shape->kernel();
  for (Polygon::Edge_const_iterator edge_iter = extent.edges_begin(); edge_iter != extent.edges_end(); ++edge_iter)
  {
    const Segment& segment = *edge_iter;
    const Number angle_target = necklace->shape->ComputeAngleRad(segment[1]);
    if
    (
      angle_target == obscured.to_rad() ||
      angle_target == obscured.from_rad() ||
      !obscured.Contains(Modulo(angle_target, obscured.from_rad()))
    )
    {
      if (CGAL::left_turn( segment[0], segment[1], kernel ))
      {
        obscured.to_rad() = ModuloNonZero(angle_target, obscured.from_rad());
      }
      else
      {
        obscured.from_rad() = Modulo(angle_target);
        obscured.to_rad() = ModuloNonZero(obscured.to_rad(), obscured.from_rad());
      }
    }
  }

  if (obscured.IsDegenerate() || M_2xPI <= obscured.ComputeLength())
    return (*fallback_)(extent, necklace);

  // Force the angles into the correct interval.
  return std::make_shared<IntervalWedge>(obscured.from_rad(), obscured.to_rad());
}

ComputeFeasibleWedgeInterval::ComputeFeasibleWedgeInterval(const Parameters& parameters) :
  ComputeFeasibleInterval(parameters)
{}

} // namespace necklace_map
} // namespace geoviz
