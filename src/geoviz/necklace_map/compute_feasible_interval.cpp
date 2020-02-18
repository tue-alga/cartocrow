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
  switch (parameters.interval_type)
  {
    case IntervalType::kCentroid:
      return Ptr(new ComputeFeasibleCentroidInterval(parameters.centroid_interval_length_rad));
    case IntervalType::kWedge:
      return Ptr(new ComputeFeasibleWedgeInterval());
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
  for (MapElement::BeadMap::value_type& map_value : element->beads)
  {
    const Necklace::Ptr& necklace = map_value.first;
    Bead::Ptr& bead = map_value.second;

    CHECK_NOTNULL(necklace);
    CHECK_NOTNULL(bead);

    Polygon extent;
    element->region.MakeSimple(extent);

    bead->feasible = (*this)(extent, necklace);
  }
}

/**@brief Apply the functor to a collection of map elements.
 * @param[in,out] elements the elements.
 */
void ComputeFeasibleInterval::operator()(std::vector<MapElement::Ptr>& elements) const
{
  for (MapElement::Ptr& element : elements)
    (*this)(element);
}


/**@class ComputeFeasibleCentroidInterval
 * @brief A functor to generate feasible centroid intervals for necklace bead placement.
 *
 * The generated centroid interval is the intersection of the necklace and a wedge @f$W@f$, such that the apex of @f$W@f$ is the necklace kernel, the inner bisector of @f$W@f$ intersects the centroid of a map region, and the inner angle of @f$W@f$ is twice some predefined angle.
 *
 * If the centroid of the region is the necklace kernel, the wedge bisector is undefined. In this case the wedge is chosen such that the inner bisector has the same direction as the positive x axis.
 */

/**@brief Construct a centroid interval generator.
 * @param length_rad @parblock the inner angle (in radians) of the wedge used when generating an interval.
 *
 * The centroid intervals cannot be empty or cover the whole necklace, i.e. the length is restricted to the range (0, 2*pi).
 * @endparblock
 */
ComputeFeasibleCentroidInterval::ComputeFeasibleCentroidInterval(const Number& length_rad)
  : ComputeFeasibleInterval(), half_length_rad_(0.5 * length_rad)
{
  CHECK_GT(half_length_rad_, 0);
  CHECK_LT(half_length_rad_, M_PI);
}

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
  //TODO(tvl) implement 'toggle' to always use centroid interval for empty-interval regions (e.g. point regions). Waarom niet sowieso?
  LOG(FATAL) << "Not implemented yet.";
}

} // namespace necklace_map
} // namespace geoviz
