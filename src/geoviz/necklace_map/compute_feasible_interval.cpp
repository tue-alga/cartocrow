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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 05-12-2019
*/

#include "compute_feasible_interval.h"

#include <cmath>

#include <glog/logging.h>


namespace geoviz
{
namespace necklace_map
{

/**@class ComputeFeasibleInterval
 * @brief An interface for a functor to generate feasible intervals for necklace bead placement.
 */

/**@brief Construct a new feasible interval computation functor.
 * @param parameters the parameters describing the desired type of functor.
 * @return a unique pointer containing a new functor or a nullptr if the functor could not be constructed.
 */
ComputeFeasibleInterval::Ptr ComputeFeasibleInterval::New(const Parameters& parameters)
{
  // The wedge interval functor also needs a centroid interval functor as fallback.
  Ptr compute_centroid(new ComputeFeasibleCentroidInterval(parameters));

  switch (parameters.interval_type)
  {
    case IntervalType::kCentroid:
      return compute_centroid;
    case IntervalType::kWedge:
    {
      Ptr compute_wedge(new ComputeFeasibleWedgeInterval(parameters));
      static_cast<ComputeFeasibleWedgeInterval*>(compute_wedge.get())->fallback_.swap( compute_centroid );
      return compute_wedge;
    }
    default:
      return nullptr;
  }
}

/**@fn virtual CircleRange::Ptr ComputeFeasibleInterval::operator()(const Polygon& extent, const Necklace::Ptr& necklace) const = 0;
 * @brief Apply the functor to a region and necklace.
 * @param extent the spatial extent of the region.
 * @param necklace the necklace.
 * @return the feasible interval for placing the region's bead on the necklace.
 */

/**@brief Apply the functor to a map element.
 * @param[in,out] element the element.
 */
void ComputeFeasibleInterval::operator()(MapElement::Ptr& element) const
{
  Polygon extent;
  element->region.MakeSimple(extent);

  const bool ignore_region = ignore_point_regions_ && extent.size() < 2;

  for (MapElement::BeadMap::value_type& map_value : element->beads)
  {
    const Necklace::Ptr& necklace = map_value.first;
    Bead::Ptr& bead = map_value.second;

    CHECK_NOTNULL(necklace);
    CHECK_NOTNULL(bead);

    bead->feasible = ignore_region ? nullptr : (*this)(extent, necklace);
  }

  if (ignore_region)
    element->beads.clear();
}

/**@brief Apply the functor to a collection of map elements.
 * @param[in,out] elements the elements.
 */
void ComputeFeasibleInterval::operator()(std::vector<MapElement::Ptr>& elements) const
{
  for (MapElement::Ptr& element : elements)
    (*this)(element);
}

ComputeFeasibleInterval::ComputeFeasibleInterval(const Parameters& parameters) :
  ignore_point_regions_(parameters.ignore_point_regions)
{}


/**@class ComputeFeasibleCentroidInterval
 * @brief A functor to generate feasible centroid intervals for necklace bead placement.
 *
 * The generated centroid interval is the intersection of the necklace and a wedge @f$W@f$, such that the apex of @f$W@f$ is the necklace kernel, the inner bisector of @f$W@f$ intersects the centroid of a map region, and the inner angle of @f$W@f$ is twice some predefined angle.
 *
 * If the centroid of the region is the necklace kernel, the wedge bisector is undefined. In this case the wedge is chosen such that the inner bisector has the same direction as the positive x axis.
 */

CircleRange::Ptr ComputeFeasibleCentroidInterval::operator()
(
  const Polygon& extent,
  const Necklace::Ptr& necklace
) const
{
  const Point centroid = ComputeCentroid()(extent);
  const Number angle_rad = necklace->shape->ComputeAngle(centroid);

  return std::make_shared<IntervalCentroid>(angle_rad - half_length_rad_, angle_rad + half_length_rad_);
}

/**@brief Construct a centroid interval generator.
 * @param length_rad @parblock the inner angle (in radians) of the wedge used when generating an interval.
 *
 * The centroid intervals cannot be empty or cover the whole necklace, i.e. the length is restricted to the range (0, 2*pi).
 * @endparblock
 */
ComputeFeasibleCentroidInterval::ComputeFeasibleCentroidInterval(const Parameters& parameters) :
  ComputeFeasibleInterval(parameters),
  half_length_rad_(0.5 * parameters.centroid_interval_length_rad)
{
  CHECK_GT(half_length_rad_, 0);
  CHECK_LT(half_length_rad_, M_PI);
}


/**@class ComputeFeasibleWedgeInterval
 * @brief A functor to generate feasible wedge intervals for necklace bead placement.
 *
 * The generated wedge interval is the intersection of the necklace and a wedge @f$W@f$, such that the apex of @f$W@f$ is the necklace kernel, @f$W@f$ contains a map region, and the inner angle of @f$W@f$ is minimal.
 *
 * If the region contains the necklace kernel, the wedge interval would cover the complete plane. In this case, a centroid interval in generated instead.
 */

CircleRange::Ptr ComputeFeasibleWedgeInterval::operator()
(
  const Polygon& extent,
  const Necklace::Ptr& necklace
) const
{
  CHECK_GT(extent.size(), 0);
  if (extent.size() == 1)
    return (*fallback_)(extent, necklace);

  const Number angle = necklace->shape->ComputeAngle(*extent.vertices_begin());
  CircleRange range(angle, angle);

  const Point& kernel = necklace->shape->kernel();
  for (Polygon::Edge_const_iterator edge_iter = extent.edges_begin(); edge_iter != extent.edges_end(); ++edge_iter)
  {
    const Segment& segment = *edge_iter;
    const Number angle_target = necklace->shape->ComputeAngle(segment[1]);
    if
    (
      angle_target == range.angle_ccw_rad() ||
      angle_target == range.angle_cw_rad() ||
      !range.IntersectsRay(angle_target)
    )
    {
      if (CGAL::left_turn( segment[0], segment[1], kernel ))
        range.angle_ccw_rad() = angle_target;
      else
        range.angle_cw_rad() = angle_target;
    }
  }

  if (range.IsDegenerate() || M_2xPI <= range.ComputeLength())
    return (*fallback_)(extent, necklace);

  // Force the angles into the correct range.
  return std::make_shared<CircleRange>(range.angle_cw_rad(), range.angle_ccw_rad());
}

ComputeFeasibleWedgeInterval::ComputeFeasibleWedgeInterval(const Parameters& parameters) :
  ComputeFeasibleInterval(parameters)
{}

} // namespace necklace_map
} // namespace geoviz
