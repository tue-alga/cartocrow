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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 07-11-19
*/

#include "region.h"

#include <glog/logging.h>


namespace geoviz
{
namespace necklace_map
{

/**@class Region
 * Each region can be a collection of disconnected polygons (e.g. an island group),
 * and each polygon may contain holes (e.g. lakes).
 *
 * Note that the Necklace Map algorithms ignore these holes for all intents and purposes.
 * Also note that polygons of a region may intersect, although polygon self-intersection
 * will produce undefined results.
 * Similarly, different regions may intersect.
 *
 * In some cases, a multi-polygon region is simplified to its convex hull, but when
 * determining the centroid, the centroid of the polygon set is used.
 */

/**@brief Main constructor.
 * @param id the ID of the region.
 * These IDs often follow ISO-3166-2 (ISO-3166-1 alpha-2, possibly followed by a
 * subdivision number), or ISO-3166-1 alpha-3. However, any set of unique IDs is
 * allowed.
 *
 * Note that a region must always have an ID to cross-reference the associated data.
 */
Region::Region(const std::string& id)
  : id(id), value(0) {}

/**@brief Check whether the region is valid.
 *
 * Three conditions must be met by a valid region: the ID is not empty, the shape is
 * valid, and the value is at least 0.
 *
 * The shape is valid if all its polygons have a counter-clockwise outer boundary that
 * is not self-intersecting.
 *
 * Note that different polygons may intersect and polygons may be degenerate, i.e.
 * enclosing an empty region such as a single point.
 * @param strict whether the value must be strictly larger than 0.
 * @return whether the region is valid.
 */
bool Region::isValid(const bool strict /*= true*/) const
{
  LOG(FATAL) << "Not implemented yet!";
}

/**@brief Compute the spatial extent of the region.
 *
 * For regions of exactly one polygon, the extent is the outer boundary of that
 * polygon. Otherwise, the extent is the convex hull of the polygon set.
 * @return the spatial extent of the region.
 */
Polygon Region::getExtent() const
{
  LOG(FATAL) << "Not implemented yet!";
}

} // namespace necklace_map
} // namespace geoviz
