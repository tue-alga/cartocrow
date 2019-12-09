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

#include "necklace_element.h"

#include <glog/logging.h>


namespace geoviz
{
namespace necklace_map
{

/**@struct NecklaceGlyph
 * @brief A visualization element to show the numeric value associated with a map region.
 *
 * Each region with a value larger than 0 that is also assigned a necklace will get a necklace glyph. The value is visualized using the area of the element. While this does not directly convey the absolute value, the difference between glyphs exposes their relative values.
 *
 * While glyphs could have various shapes, we currently only support disks.
 */

/**@brief Check whether the glyph is valid.
 *
 * This validity depends on three aspects: the interval must not be null, the interval must be valid, and the glyph's position must be in the interval.
 */
bool NecklaceGlyph::IsValid() const
{
  return interval != nullptr && interval->IsValid() && interval->IntersectsRay(angle_rad);
}


/**@struct NecklaceElement
 * @brief A region and its associated data for use in a necklace map.
 *
 * This element has a region, a numeric value that should be visualized in the necklace map, and a glyph used for this visualization.
 *
 * Note that the Necklace Map algorithms ignore holes in the region for all intents and purposes. Also note that polygons of a region may intersect, although polygon self-intersection will produce undefined results. Similarly, different regions may intersect.
 *
 * In some cases, a multi-polygon region is simplified to its convex hull, but when determining the centroid, the centroid of the polygon set is used.
 */


/**@brief Construct a necklace region and data element with an empty region.
 * @param id @parblock the ID of the region.
 *
 * See @f Region::Region(const std::string& id) for details on this ID.
 *
 * Note that necklace elements without an ID are ignored when constructing the necklace
 * map.
 * @endparblock
 */
NecklaceElement::NecklaceElement(const std::string& id)
  : region(id), value(0), necklace(), glyph() {}

/**@brief Construct a necklace region and data element from a region.
 * @param region @parblock the region of the element.
 *
 * Note that necklace elements of a region without an ID are ignored when constructing the necklace map.
 * @endparblock
 */
NecklaceElement::NecklaceElement(const Region& region)
  : region(region), value(0), necklace(), glyph() {}

/**@brief Check whether the necklace element is valid.
 *
 * This validity is based on two conditions: the region must be valid and the value must be at least 0.
 * @param strict whether the value must be strictly larger than 0.
 * @return whether the element is valid.
 */
bool NecklaceElement::IsValid(const bool strict /*= true*/) const
{
  LOG(FATAL) << "Not implemented yet!";
}

} // namespace necklace_map
} // namespace geoviz"
