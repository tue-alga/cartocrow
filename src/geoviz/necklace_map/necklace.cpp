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

#include "necklace.h"


namespace geoviz
{
namespace necklace_map
{

/**@struct NecklaceType
 * @brief A necklace is a star-shaped curve that guides the placement of data
 * visualization glyphs.
 *
 * While the glyphs could have various shapes, we currently only support disks.
 */

/**@fn virtual const Point& NecklaceType::getKernel() const
 * @brief Give the kernel of the necklace.
 *
 * Any ray originating from this point will intersect the necklace in at most 1 point.
 * @return the kernel of the necklace.
 */


/**@struct CircleNecklace
 * @brief A full circle necklace.
 */

/**@brief Main constructor.
 *
 * The necklace must have the circle center as kernel.
 *
 * @param shape the necklace covers the full circle.
 */
CircleNecklace::CircleNecklace(const Circle& shape)
  : shape(shape) {}

inline const Point& CircleNecklace::getKernel() const
{
  return shape.center();
}


/**@struct CurveNecklace
 * @brief A circular arc necklace.
 *
 * Note that the circular arc may cover the full circle. In this case, the glyph
 * placement will be identical to the placement on a CircleNecklace of the supporting
 * circle.
 */

/**@brief Main constructor.
 *
 * If from_rad and to_rad are the same, the necklace covers the full circle.
 *
 * Otherwise, the necklace covers the part of the circle from from_rad to to_rad, lying
 * counterclockwise of from_rad.
 *
 * @param shape the supporting circle of the necklace.
 * @param from_rad the counterclockwise starting angle in radians of the necklace.
 * @param to_rad the counterclockwise terminating angle in radians of the necklace.
 */
CurveNecklace::CurveNecklace(const Circle& shape, const Number& from_rad, const Number& to_rad)
  : CircleNecklace(shape),
    from_rad(from_rad),
    to_rad(to_rad) {}


/**@struct GeneralNecklace
 * @brief A generic star-shaped necklace.
 *
 * Note that for this necklace, the kernel must be set explicitly.
 */

} // namespace necklace_map
} // namespace geoviz