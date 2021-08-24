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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 05-12-2019
*/

#include "polygon.h"

#include <glog/logging.h>


namespace cartocrow
{
namespace detail
{

Vector ComputeCentroid(const Polygon& shape, Number& area)
{
  if (shape.size() == 1)
    return shape[0] - Point(CGAL::ORIGIN);

  Vector sum(0, 0);
  for (Polygon::Edge_const_iterator edge_iter = shape.edges_begin(); edge_iter != shape.edges_end(); ++edge_iter)
  {
    const Number weight =
      edge_iter->source().x() * edge_iter->target().y() -
      edge_iter->target().x() * edge_iter->source().y();
    sum += weight * (edge_iter->source() - Point(CGAL::ORIGIN));
    sum += weight * (edge_iter->target() - Point(CGAL::ORIGIN));
  }

  area += shape.area();
  return sum / (Number(6) * area);
}

Vector ComputeCentroid(const Polygon_with_holes& shape, Number& area)
{
  Vector sum = ComputeCentroid(shape.outer_boundary(), area);
  for
  (
    Polygon_with_holes::Hole_const_iterator hole_iter = shape.holes_begin();
    hole_iter != shape.holes_end();
    ++hole_iter
  )
  {
    // Note that because the hole is clockwise, its area is negative.
    CHECK(hole_iter->is_clockwise_oriented());
    sum += ComputeCentroid(*hole_iter, area);
  }
  return sum / area;
}

} // namespace detail


/**@class ComputeCentroid
 * @brief Compute the centroid of a 2D shape.
 *
 * Note that while CGAL provides functionality for computing the centroid of a point set, the centroid of a shape depends on the space it covers, not just its boundary.
 */

/**@brief Compute the centroid of a straight-line polygon.
 * @param shape the polygon.
 * @return the centroid of the polygon.
 */
Point ComputeCentroid::operator()(const Polygon& shape) const
{
  Number area = 0;
  return Point(CGAL::ORIGIN) + detail::ComputeCentroid(shape, area);
}

/**@brief Compute the centroid of a straight-line polygon with holes.
 *
 * Note that all holes must have clockwise orientation.
 * @param shape the polygon with holes.
 * @return the centroid of the polygon with holes.
 */
Point ComputeCentroid::operator()(const Polygon_with_holes& shape) const
{
  Number area = 0;
  return Point(CGAL::ORIGIN) + detail::ComputeCentroid(shape, area);
}

} // namespace cartocrow
