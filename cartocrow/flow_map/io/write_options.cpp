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

#include "write_options.h"

namespace cartocrow {
namespace flow_map {

/**@brief Initialize write options with default values.
 * @return Default write options.
 */
WriteOptions::Ptr WriteOptions::Default() {
	WriteOptions::Ptr options = std::make_shared<WriteOptions>();

	options->pixel_width = 500;

	options->numeric_precision = 9;
	options->region_opacity = -1;
	options->obstacle_opacity = -1;
	options->flow_opacity = 1;
	options->node_opacity = 1;

	return options;
}

/**@brief Initialize write options with debug values.
 * @return Debug write options.
 */
WriteOptions::Ptr WriteOptions::Debug() {
	WriteOptions::Ptr options = std::make_shared<WriteOptions>();

	options->pixel_width = 500;

	options->numeric_precision = 9;
	options->region_opacity = -1;
	options->obstacle_opacity = -1;
	options->flow_opacity = 0.5;
	options->node_opacity = 1;

	return options;
}

/**@fn WriteOptions::Ptr
 * @brief The preferred pointer type for storing or sharing write options.
 */

/**@fn int WriteOptions::pixel_width
 * @brief The width in pixels of the output svg figure.
 */

/**@fn int WriteOptions::numeric_precision
 * @brief The numeric precision with which to store the point coordinates.
 */

/**@fn double WriteOptions::region_opacity
 * @brief The opacity with which to draw the regions.
 *
 * This must be at most 1.
 *
 * For negative values, the opacity of the input regions is used instead.
 */

/**@fn double WriteOptions::obstacle_opacity
 * @brief The opacity with which to draw the obstacles.
 *
 * This must be at most 1.
 *
 * For negative values, the opacity of the input obstacles is used instead.
 */

/**@fn double WriteOptions::flow_opacity
 * @brief The opacity with which to draw the flow tree.
 *
 * This must be at most 1.
 *
 * For negative values, the opacity of the input regions is used instead.
 */

/**@fn double WriteOptions::node_opacity
 * @brief The opacity with which to draw the nodes.
 *
 * This must be at most 1.
 *
 * For negative values, the opacity of the input regions is used instead.
 */

} // namespace flow_map
} // namespace cartocrow
