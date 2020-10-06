/*
The Flow Map library implements the algorithmic geo-visualization
method by the same name, developed by Kevin Verbeek, Kevin Buchin,
and Bettina Speckmann at TU Eindhoven
(DOI: 10.1007/s00453-013-9867-z & 10.1109/TVCG.2011.202).
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 02-09-2020
*/

#include "svg_writer.h"

#include <glog/logging.h>


namespace geoviz
{
namespace flow_map
{

/**@class SvgWriter
 * @brief A writer for flow map output geometry.
 */

/**@brief Construct a flow map geometry writer.
 */
SvgWriter::SvgWriter() {}

/**@brief Write a flow map to a stream.
 * @param options the options for how to write the flow map.
 * @param out the stream to which to write.
 * @return whether the flow map could be successfully written to the stream.
 */
bool SvgWriter::Write
(
  const WriteOptions::Ptr& options,
  std::ostream& out
) const
{
  //detail::SvgWriter writer(elements, necklaces, scale_factor, options, out);

  // The order of drawing the features determines their stacking order, i.e. the last one will be on top.
  //writer.DrawPolygonRegions();
  //writer.DrawPointRegions();

  return true;
}

} // namespace flow_map
} // namespace geoviz
