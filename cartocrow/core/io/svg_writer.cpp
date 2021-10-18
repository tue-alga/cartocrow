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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 09-03-2021
*/

#include "svg_writer.h"

#include "cartocrow/core/io/detail/svg_writer.h"

namespace cartocrow {

/**@class SvgWriter
 * @brief A writer for commonly used geometry.
 *
 * This writer should be given all the relevant geometry before writing it all to a stream.
 */

/**@brief Construct a commonly used geometry writer.
 */
SvgWriter::SvgWriter() {}

/**@brief Add a point with polar coordinates to the geometry to write.
 * @param point the point with polar coordinates to add.
 */
void SvgWriter::Add(const PolarPoint& point) {
	points_.push_back(point);
}

/**@brief Add a spiral to the geometry to write.
 * @param point the spiral to add.
 */
void SvgWriter::Add(const Spiral& spiral) {
	spirals_.push_back(spiral);
}

/**@brief Add a spiral segment to the geometry to write.
 * @param point the spiral segment to add.
 */
void SvgWriter::Add(const SpiralSegment& segment) {
	spiral_segments_.push_back(segment);
}

/**@brief Add a line with polar coordinates to the geometry to write.
 * @param point the line with polar coordinates to add.
 */
void SvgWriter::Add(const PolarLine& line) {
	lines_.push_back(line);
}

/**@brief Add a line segment with polar coordinates to the geometry to write.
 * @param point the line segment with polar coordinates to add.
 */
void SvgWriter::Add(const PolarSegment& segment) {
	line_segments_.push_back(segment);
}

/**@brief Write the collected geometry to a stream.
 * @param out the stream to which to write.
 * @return whether the geometry could be successfully written to the stream.
 */
bool SvgWriter::Write(const WriteOptions::Ptr& options, std::ostream& out) const {
	detail::SvgWriter writer(points_, spirals_, spiral_segments_, lines_, line_segments_, options,
	                         out);

	// The order of drawing the features determines their stacking order, i.e. the last one will be on top.
	writer.DrawSpirals();
	writer.DrawLines();
	writer.DrawPoints();

	return true;
}

} // namespace cartocrow
