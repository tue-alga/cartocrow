/*
The Necklace Map library implements the algorithmic
geo-visualization method by the same name, developed by
Bettina Speckmann and Kevin Verbeek at TU Eindhoven
(DOI: 10.1109/TVCG.2010.180 & 10.1142/S021819591550003X).
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 29-01-2020
*/

#include "svg_writer.h"

#include <glog/logging.h>

#include "geoviz/necklace_map/io/detail/svg_writer.h"


namespace geoviz
{
namespace necklace_map
{

/**@class SvgWriter
 * @brief A writer for necklace map output geometry.
 */

/**@fn SvgWriter::MapElement
 * @brief A region and its associated data for use in a necklace map.
 */

/**@fn SvgWriter::Necklace
 * @brief A collection of visualization symbols that are organized on a curve.
 */

/**@brief Construct a necklace map geometry writer.
 */
SvgWriter::SvgWriter() {}

/**@brief Write a necklace map to a stream.
 * @param elements the elements of the necklace map.
 * @param necklaces the necklaces of the map.
 * @param scale_factor the factor by which to scale the necklace beads.
 * @param options the options for how to write the necklace map.
 * @param out the stream to which to write.
 * @return whether the necklace map could be successfully written to the stream.
 */
<<<<<<< HEAD:console/necklace_map_io/necklace_writer.cpp
void NecklaceWriter::Write
||||||| 5e5d058:src/console/necklace_map_io/necklace_writer.cpp
bool NecklaceWriter::Write
=======
bool SvgWriter::Write
>>>>>>> dev_final:geoviz/necklace_map/io/svg_writer.cpp
(
  const std::vector<MapElement::Ptr>& elements,
  const std::vector<Necklace::Ptr>& necklaces,
  const Number& scale_factor,
  const WriteOptions::Ptr& options,
  std::ostream& out
) const
{
  detail::SvgWriter writer(elements, necklaces, scale_factor, options, out);

  // The order of drawing the features determines their stacking order, i.e. the last one will be on top.
  writer.DrawPolygonRegions();
  writer.DrawPointRegions();
  writer.DrawNecklaces();
  writer.DrawValidIntervals();
  writer.DrawRegionAngles();
  writer.DrawBeadAngles();
  writer.DrawFeasibleIntervals();
  writer.DrawBeads();

  return true;
}

} // namespace necklace_map
} // namespace geoviz
