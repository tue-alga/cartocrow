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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 07-11-2019
*/

#include "region.h"

#include <iterator>

#include <CGAL/convex_hull_2.h>
#include <glog/logging.h>


namespace geoviz
{

/**@class Region
 * A collection of polygons.
 *
 * The polygons may be disconnected (e.g. an island group), and each polygon may contain holes (e.g. lakes).
 */

/**@typedef Region::PolygonSet
 * @brief The shape of the region.
 *
 * Note that a region is not required to be a connected shape.
 */

/**@brief Construct a region.
 * @param id @parblock the ID of the region.
 *
 * See Region::id for more details on this ID.
 * @endparblock
 */
Region::Region(const std::string& id)
  : id(id), shape(), style("") {}

/**@brief Check whether the region covers a sinlge point.
 * @return whether the region covers a sinlge point.
 */
bool Region::IsPoint() const
{
  return shape.size() == 1 && shape[0].outer_boundary().size() == 1;
}

/**@brief Check whether the region is valid.
 *
 * The region is valid if all its polygons have a counter-clockwise outer boundary that is not self-intersecting.
 *
 * Note that different polygons may intersect and polygons may be degenerate, i.e. enclosing an empty region such as a single point.
 * @return whether the region is valid.
 */
bool Region::IsValid() const
{
  bool correct = true;
  for (const Polygon_with_holes& polygon : shape)
  {
    const Polygon& outer = polygon.outer_boundary();

    // Degenerate polygons are considered correct.
    if (outer.size() == 1)
      continue;

    correct &=
      outer.orientation() == CGAL::COUNTERCLOCKWISE &&
      outer.is_simple();
  }
  return correct;
}

/**@brief Make the region as valid as possible.
 *
 * The region is valid if all its polygons have a counter-clockwise outer boundary that is not self-intersecting.
 *
 * Self-intersecting polygons are not corrected.
 * @return whether the region is valid after the changes.
 */
bool Region::MakeValid()
{
  bool correct = true;
  for (Polygon_with_holes& polygon : shape)
  {
    Polygon& outer = polygon.outer_boundary();

    // Degenerate polygons are considered correct.
    if (outer.size() == 1)
      continue;

    if (!outer.is_simple())
      correct = false;
    else if (outer.orientation() != CGAL::COUNTERCLOCKWISE)
      outer.reverse_orientation();
  }
  return correct;
}

/**@brief Generate a single polygon without holes that describes the region.
 *
 * If the region consists of a single polygon, the simple polygon is its outer boundary.
 *
 * Otherwise, the simple polygon is the convex hull of the set of polygons.
 * @param simple the simple polygon describing the region.
 */
void Region::MakeSimple(Polygon& simple)
{
  if (shape.size() == 1)
  {
    simple = shape[0].outer_boundary();
    return;
  }

  // Collect the outer boundary points.
  std::vector<Point> points;
  points.reserve(256);  // Reserve for a reasonable point set.
  for (const Polygon_with_holes& part : shape)
    points.insert(points.end(), part.outer_boundary().vertices_begin(), part.outer_boundary().vertices_end());

  // Construct the convex hull.
  simple.clear();
  CGAL::convex_hull_2(points.begin(), points.end(), std::back_inserter(simple));
}

/**@fn std::string Region::id
 * @brief the ID of the region.
 *
 * This IDs often follows ISO-3166-2 (ISO-3166-1 alpha-2, possibly followed by a subdivision number), or ISO-3166-1 alpha-3. However, any ID that is unique within the collection of regions is allowed.
 */

/**@fn PolygonSet Region::shape;
 * @brief The shape of the region.
 */

/**@fn std::string Region::style;
 * @brief The style used to draw the region.
 *
 * This must be a valid SVG element style attribute.
 */

} // namespace geoviz
