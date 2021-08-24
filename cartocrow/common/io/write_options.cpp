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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 02-09-2020
*/

#include "write_options.h"

namespace cartocrow {

/**@brief Initialize write options with default values.
 * @return Default write options.
 */
WriteOptions::Ptr WriteOptions::Default() {
	WriteOptions::Ptr options = std::make_shared<WriteOptions>();

	options->pixel_width = 500;
	options->numeric_precision = 9;

	return options;
}

/**@brief Initialize write options with debug values.
 * @return Debug write options.
 */
WriteOptions::Ptr WriteOptions::Debug() {
	WriteOptions::Ptr options = std::make_shared<WriteOptions>();

	options->pixel_width = 500;
	options->numeric_precision = 9;

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

} // namespace cartocrow
