/*
The GeoViz library implements algorithmic geo-visualization methods,
developed at TU Eindhoven.
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 09-03-2021
*/

#include "svg_writer.h"

#include "geoviz/common/io/detail/svg_writer.h"


namespace geoviz
{
namespace common
{

/**@class SvgWriter
 * @brief A writer for common geometry.
 */

/**@brief Construct a common geometry writer.
 */
SvgWriter::SvgWriter() {}

void SvgWriter::Add(const PolarPoint& point)
{
  points_.push_back(point);
}

void SvgWriter::Add(const Spiral& spiral)
{
  spirals_.push_back(spiral);
}

void SvgWriter::Add(const SpiralSegment& segment)
{
  spiral_segments_.push_back(segment);
}

void SvgWriter::Add(const PolarLine& line)
{
  lines_.push_back(line);
}

void SvgWriter::Add(const PolarSegment& segment)
{
  line_segments_.push_back(segment);
}

/**@brief Write the collected geometry to a stream.
 * @param out the stream to which to write.
 * @return whether the geometry could be successfully written to the stream.
 */
bool SvgWriter::Write(const WriteOptions::Ptr& options, std::ostream& out) const
{
  detail::SvgWriter writer(points_, spirals_, spiral_segments_, lines_, line_segments_, options, out);

  // The order of drawing the features determines their stacking order, i.e. the last one will be on top.
  writer.DrawSpirals();
  writer.DrawLines();
  writer.DrawPoints();

  return true;
}

} // namespace common
} // namespace geoviz
