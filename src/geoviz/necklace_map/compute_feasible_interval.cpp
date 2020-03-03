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

#include "geoviz/necklace_map/bead.h"
#include "geoviz/necklace_map/compute_feasible_centroid_interval.h"
#include "geoviz/necklace_map/compute_feasible_wedge_interval.h"


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


} // namespace necklace_map
} // namespace geoviz
