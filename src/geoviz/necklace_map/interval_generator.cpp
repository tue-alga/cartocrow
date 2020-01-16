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

#include "interval_generator.h"

#include <cmath>

#include <glog/logging.h>


namespace geoviz
{
namespace necklace_map
{

/**@struct IntervalGenerator
 * @brief An interface for a functor to generate feasible intervals for necklace glyph placement.
 */

/**@fn virtual void IntervalGenerator::operator()(const Polygon& extent, const std::shared_ptr<NecklaceType>& necklace, std::shared_ptr<NecklaceInterval>& interval) const = 0
 * @brief Apply the functor to a region and necklace.
 * @param extent the spatial extent of the region.
 * @param necklace the necklace.
 * @param interval the feasible interval for placing the necklace glyph of the region on the necklace.
 */

/**@brief Apply the functor to a map element.
 * @param[in,out] element the element.
 */
void IntervalGenerator::operator()(MapElement::Ptr& element) const
{
  for (MapElement::GlyphMap::value_type& map_value : element->glyphs)
  {
    const Necklace::Ptr& necklace = map_value.first;
    NecklaceGlyph::Ptr& bead = map_value.second;

    CHECK_NOTNULL(necklace);
    CHECK_NOTNULL(bead);

    Polygon extent;
    element->region.MakeSimple(extent);

    bead->interval = (*this)(extent, necklace);
  }
}

/**@brief Apply the functor to a collection of map elements.
 * @param[in,out] elements the elements.
 */
void IntervalGenerator::operator()(std::vector<MapElement::Ptr>& elements) const
{
  for (MapElement::Ptr& element : elements)
    (*this)(element);
}


/**@struct IntervalCentroidGenerator
 * @brief A functor to generate feasible centroid intervals for necklace glyph placement.
 *
 * The generated centroid interval is the intersection of the necklace and a wedge @f$W@f$, such that the apex of @f$W@f$ is the necklace kernel, the inner bisector of @f$W@f$ intersects the centroid of a map region, and the inner angle of @f$W@f$ is twice some predefined angle.
 *
 * If the centroid of the region is the necklace kernel, the wedge bisector is undefined. In this case the wedge is chosen such that the inner bisector has the same direction as the positive x axis.
 */

/**@brief Construct a centroid interval generator.
 * @param buffer_rad @parblock half the inner angle (in radians) of the wedge used when generating an interval.
 *
 * In other words, this is the angle between the inner bisector of the wedge and either boundary ray of the wedge.
 * @endparblock
 */
IntervalCentroidGenerator::IntervalCentroidGenerator(const Number& length_rad)
  : IntervalGenerator(), half_length_rad_(0.5 * length_rad) {}

NecklaceInterval::Ptr IntervalCentroidGenerator::operator()
(
  const Polygon& extent,
  const Necklace::Ptr& necklace
) const
{
  const Point centroid = ComputeCentroid()(extent);
  const Vector centroid_offset = centroid - necklace->shape->kernel();

  // Note the special case where the centroid overlaps the necklace kernel.
  const Number angle_rad =
    centroid_offset.squared_length() == 0
    ? 0
    : std::atan2(centroid_offset.y(), centroid_offset.x());

  return std::make_shared<IntervalCentroid>(angle_rad - half_length_rad_, angle_rad + half_length_rad_);
}


/**@struct IntervalWedgeGenerator
 * @brief A functor to generate feasible wedge intervals for necklace glyph placement.
 *
 * The generated wedge interval is the intersection of the necklace and a wedge @f$W@f$, such that the apex of @f$W@f$ is the necklace kernel, @f$W@f$ contains a map region, and the inner angle of @f$W@f$ is minimal.
 *
 * If the region contains the necklace kernel, the wedge interval would cover the complete plane. In this case, a centroid interval in generated instead.
 */

NecklaceInterval::Ptr IntervalWedgeGenerator::operator()
(
  const Polygon& extent,
  const Necklace::Ptr& necklace
) const
{
  //TODO(tvl) implement 'toggle' to always use centroid interval for empty-interval regions (e.g. point regions).
  LOG(FATAL) << "Not implemented yet.";
}

} // namespace necklace_map
} // namespace geoviz
