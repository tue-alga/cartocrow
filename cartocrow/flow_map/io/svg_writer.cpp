/*
The Flow Map library implements the algorithmic geo-visualization
method by the same name, developed by Kevin Verbeek, Kevin Buchin,
and Bettina Speckmann at TU Eindhoven
(DOI: 10.1007/s00453-013-9867-z & 10.1109/TVCG.2011.202).
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 02-09-2020
*/

#include "svg_writer.h"

#include <glog/logging.h>

#include "cartocrow/flow_map/io/detail/svg_writer.h"

namespace cartocrow {
namespace flow_map {

/**@class SvgWriter
 * @brief A writer for flow map output geometry.
 */

/**@brief Construct a flow map geometry writer.
 */
SvgWriter::SvgWriter() {}

/**@brief Write a flow map to a stream.
 * @param context the context regions of the flow map.
 * @param obstacles the polygonal obstacles that must be avoided by the flow tree.
 * @param tree the flow tree.
 * @param options the options for how to write the flow map.
 * @param out the stream to which to write.
 * @return whether the flow map could be successfully written to the stream.
 */
bool SvgWriter::Write(const std::vector<Region>& context, const std::vector<Region>& obstacles,
                      const FlowTree::Ptr& tree, const WriteOptions::Ptr& options,
                      std::ostream& out) const {
	detail::SvgWriter writer(context, obstacles, tree, options, out);

	// The order of drawing the features determines their stacking order, i.e. the last one will be on top.
	writer.DrawContext();
	writer.DrawObstacles();
	writer.DrawFlow();
	writer.DrawNodes();

	return true;
}

} // namespace flow_map
} // namespace cartocrow
