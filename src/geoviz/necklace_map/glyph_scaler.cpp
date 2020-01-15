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

#include "glyph_scaler.h"

#include <algorithm>
#include <unordered_map>

#include <glog/logging.h>

#include "geoviz/necklace_map/detail/glyph_scaler.h"


namespace geoviz
{
namespace necklace_map
{

/**@struct GlyphScaler
 * @brief A functor to compute the optimal scale factor for a collection of necklace map elements.
 *
 * The optimal scale factor is the maximum value such that if all necklace glyphs have radius scale factor * sqrt(data value), none of these glyphs are within the minimum separation distance of another glyph on the same necklace.
 *
 * Note that this scale factor is the minimum over the scale factors per necklace. These scale factors per necklace can be determined independently.
 *
 * Note that we do not restrict the glyphs of different necklaces to overlap. In case of overlap between different necklaces, the user can manually adjust the buffer thickness or the positioning forces (see @f GlyphPositioner) to prevent overlapping glyphs.
 */

/**@brief Construct a glyph scaler.
 * @param min_separation @parblock the minimum distance between necklace glyphs.
 *
 * This distance must be in the range [0, @f$T@f$], where @f$T@f$ is half the length of the necklace divided by the number of glyphs on the necklace. While the lower bound is validated immediately, the upper bound can only be validated when applying the functor to a collection of necklace map elements.
 * @endparblock
 */
GlyphScaler::GlyphScaler(const Number& min_separation /*= 0*/)
  : dilation_(min_separation / 2)
{
  CHECK_GE(dilation_, 0);
}

/**@fn virtual Number GlyphScaler::operator()(const std::vector<MapElement::Ptr>& elements, NecklaceOrderMap& ordering) const = 0
 * @brief Apply the scaler to a collection of elements.
 * @param elements the elements for which to determine the optimal scale factor.
 * @param[out] ordering the ordering of the indices of the elements (per necklace) that resulted in the optimal scale factor.
 * @return the optimal scale factor.
 */


/**@struct GlyphScalerFixedOrder
 * @brief A functor to compute the optimal scale factor for a collection of necklace map elements with fixed order.
 *
 * The necklace map elements will always be ordered by the clockwise endpoint of their interval.
 *
 * The optimal scale factor is the maximum value such that if all necklace glyphs have radius scale factor * sqrt(data value), none of these glyphs are within the minimum separation distance of another glyph on the same necklace.
 *
 * Note that this scale factor is the minimum over the scale factors per necklace. These scale factors per necklace can be determined independently.
 *
 * Note that we do not restrict the glyphs of different necklaces to overlap. In case of overlap between different necklaces, the user can manually adjust the buffer thickness or the positioning forces (see @f GlyphPositioner) to prevent overlapping glyphs.
 */

/**@brief Construct a fixed order glyph scaler.
 *
 * The order of the glyphs is based on the clockwise extreme of their feasible interval.
 * @param min_separation @parblock the minimum distance between necklace glyphs.
 *
 * This distance must be in the range [0, @f$T@f$], where @f$T@f$ is half the length of the necklace divided by the number of glyphs on the necklace. While the lower bound is validated immediately, the upper bound can only be validated when applying the functor to a collection of necklace map elements.
 * @endparblock
 */
GlyphScalerFixedOrder::GlyphScalerFixedOrder(const Number& min_separation /*= 0*/)
  : GlyphScaler(min_separation)
{}

Number GlyphScalerFixedOrder::operator()(const std::vector<MapElement::Ptr>& elements, NecklaceOrderMap& ordering) const
{
  //TODO(tvl) constructing a sorted list of beads should be separated out, probably into separate class, or into the necklace class (?)
  // Construct an ordering on the elements.
  for (size_t e = 0; e < elements.size(); ++e)
  {
    // Ignore elements without a value.
    const MapElement::Ptr& element = elements[e];
    if (element->value <= 0)
      continue;

    CHECK_NOTNULL(element);
    CHECK_NOTNULL(element->glyph);
    CHECK_NOTNULL(element->glyph->necklace);

    ordering[element->glyph->necklace].push_back(e);
  }

  // Determine the optimal scale factor per necklace;
  // the global optimum is the smallest of these.
  Number scale_factor = -1;
  for (NecklaceOrderMap::iterator order_iter = ordering.begin(); order_iter != ordering.end(); ++order_iter)
  {
    const Necklace::Ptr& necklace = order_iter->first;
    Ordering& necklace_order = order_iter->second;

    // Check whether the minimum separation can be enforced.
    const Number total_separation = necklace_order.size() * dilation_ * 2;
    CHECK_LE(total_separation, necklace->shape->ComputeLength() / 2);  // At least half the circle is available for glyphs.

    // Sort the elements by the clockwise extreme of their feasible interval.
    std::sort
    (
      necklace_order.begin(),
      necklace_order.end(),
      [elements](const size_t& a, const size_t& b)
      {
        const MapElement::Ptr& elem_a = elements[a];
        const MapElement::Ptr& elem_b = elements[b];
        return elem_a->glyph->interval->angle_cw_rad() < elem_b->glyph->interval->angle_cw_rad();
      }
    );

    // Per element add a node to the scaler.
    const Number necklace_radius = necklace->shape->ComputeLength() / M_2xPI;
    detail::FixedGlyphScaler scaler(necklace_radius, dilation_);
    for (const size_t& e : order_iter->second)
    {
      const MapElement::Ptr& element = elements[e];
      scaler.AddNode(element);
    }

    // Determine the scale factor.
    const Number necklace_scale_factor = scaler.OptimizeScaleFactor();
    //TODO(tvl) add bisection search for true (valid) scale factor.

    if (scale_factor < 0 || necklace_scale_factor < scale_factor)
      scale_factor = necklace_scale_factor;
  }

  return scale_factor;
}

} // namespace necklace_map
} // namespace geoviz"
