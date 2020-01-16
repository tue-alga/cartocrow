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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 15-01-2020
*/

#include "necklace_glyph.h"


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

/**@brief Construct a necklace glyph.
 */
NecklaceGlyph::NecklaceGlyph(const Number& radius_base)
  : interval(), angle_rad(0), angle_min_rad(0), angle_max_rad(0) {}

/**@brief Check whether the glyph is valid.
 *
 * This validity depends on three aspects: the interval must not be null, the interval must be valid, and the glyph's position must be in the interval.
 */
bool NecklaceGlyph::IsValid() const
{
  return interval != nullptr && interval->IsValid() && interval->IntersectsRay(angle_rad);
}

} // namespace necklace_map
} // namespace geoviz
