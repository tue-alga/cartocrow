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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 28-11-2019
*/

#include "map_element.h"

#include <glog/logging.h>


namespace geoviz
{
namespace necklace_map
{

/**@struct MapElement
 * @brief A region and its associated data for use in a necklace map.
 *
 * This element has a region, a numeric value that should be visualized in the necklace map, and a bead used for this visualization.
 *
 * Note that the Necklace Map algorithms ignore holes in the region for all intents and purposes. Also note that polygons of a region may intersect, although polygon self-intersection will produce undefined results. Similarly, different regions may intersect.
 *
 * In some cases, a multi-polygon region is simplified to its convex hull, but when determining the centroid, the centroid of the polygon set is used.
 */

/**@fn MapElement::Ptr
 * @brief The preferred pointer type for storing or sharing a map element.
 */

/**@fn MapElement::BeadMap
 * @brief The type for a collection of beads associated with this element, accessed by their necklace.
 *
 * Note that an element can have a single bead per necklace.
 */

/**@brief Construct a necklace region and data element with an empty region.
 * @param id @parblock the ID of the region.
 *
 * See Region::id for details on this ID.
 *
 * Note that necklace elements without an ID are ignored when constructing the necklace
 * map.
 * @endparblock
 */
MapElement::MapElement(const std::string& id)
  : region(id), value(0), beads() {}

/**@brief Construct a necklace region and data element from a region.
 * @param region @parblock the region of the element.
 *
 * Note that necklace elements of a region without an ID are ignored when constructing the necklace map.
 * @endparblock
 */
MapElement::MapElement(const Region& region)
  : region(region), value(0), beads() {}

/**@brief Check whether the necklace element is valid.
 *
 * This validity is based on three conditions: the region must be valid, the value must be at least 0, and the beads must be valid.
 * @param strict whether the value must be strictly larger than 0.
 * @return whether the element is valid.
 */
bool MapElement::IsValid(const bool strict /*= true*/) const
{
  if (!region.IsValid())
    return false;

  if (value < 0)
    return false;
  if (strict && value == 0)
    return false;

  for (const BeadMap::value_type& bead_value : beads)
    if (!bead_value.second->IsValid())
      return false;

  return true;
}

/**@brief Create a bead per necklace.
 *
 * This is skipped if this element has a positive value.
 * @param parameters the parameters describing the desired type of beads.
 */
void MapElement::InitializeBeads(const Parameters& parameters)
{
  // Elements on a necklace must have strictly positive value.
  if (value <= 0) return;
  if (parameters.ignore_point_regions)
  {
    Polygon extent;
    region.MakeSimple(extent);

    if (extent.size() < 2)
      return;
  }

  for (BeadMap::iterator bead_iter = beads.begin(); bead_iter != beads.end(); ++bead_iter)
  {
    const Necklace::Ptr& necklace = bead_iter->first;
    bead_iter->second = std::make_shared<necklace_map::Bead>(CGAL::sqrt(value), region.style, region.id);
    necklace->beads.push_back(bead_iter->second);
  }
}

/**@fn Region MapElement::region;
 * @brief The region of the map associated with this element.
 */

/**@fn Number MapElement::value;
 * @brief The data value associated with this element.
 */

/**@fn BeadMap MapElement::beads;
 * @brief The necklace beads associated with this element.
 *
 * Note that one element may have beads on multiple necklaces.
 */

} // namespace necklace_map
} // namespace geoviz"
