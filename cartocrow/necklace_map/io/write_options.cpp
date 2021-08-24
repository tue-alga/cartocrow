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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 02-09-2020
*/

#include "write_options.h"


namespace cartocrow
{
namespace necklace_map
{

/**@brief Initialize write options with default values.
 * @return Default write options.
 */
WriteOptions::Ptr WriteOptions::Default()
{
  WriteOptions::Ptr options = std::make_shared<WriteOptions>();

  options->pixel_width = 500;

  options->region_precision = 9;
  options->region_opacity = -1;
  options->bead_opacity = 1;
  options->bead_id_font_size_px = 16;

  options->draw_necklace_curve = true;
  options->draw_necklace_kernel = false;
  options->draw_bead_ids = true;

  options->draw_feasible_intervals = false;
  options->draw_valid_intervals = false;
  options->draw_region_angles = false;
  options->draw_bead_angles = false;

  return options;
}

/**@brief Initialize write options with debug values.
 * @return Debug write options.
 */
WriteOptions::Ptr WriteOptions::Debug()
{
  WriteOptions::Ptr options = std::make_shared<WriteOptions>();

  options->pixel_width = 500;

  options->region_precision = 9;
  options->region_opacity = -1;
  options->bead_opacity = 0.5;
  options->bead_id_font_size_px = 16;

  options->draw_necklace_curve = true;
  options->draw_necklace_kernel = true;
  options->draw_bead_ids = true;

  options->draw_feasible_intervals = true;
  options->draw_valid_intervals = true;
  options->draw_region_angles = false;
  options->draw_bead_angles = true;

  return options;
}

/**@fn WriteOptions::Ptr
 * @brief The preferred pointer type for storing or sharing write options.
 */

/**@fn int WriteOptions::pixel_width
 * @brief The width in pixels of the output svg figure.
 */

/**@fn int WriteOptions::region_precision
 * @brief The numeric precision with which to store the region point coordinates.
 */

/**@fn double WriteOptions::region_opacity
 * @brief The opacity with which to draw the regions.
 *
 * This must be at most 1.
 *
 * For negative values, the opacity of the input regions is used instead.
 */

/**@fn double WriteOptions::bead_opacity
 * @brief The opacity with which to draw the beads.
 *
 * This must be at most 1.
 *
 * For negative values, the opacity of the input regions is used instead.
 */

/**@fn double WriteOptions::bead_id_font_size_px
 * @brief The font size in pixels of the bead IDs.
 *
 * This font size must be strictly positive.
 */

/**@fn bool WriteOptions::draw_necklace_curve
 * @brief Whether to draw the necklace curve.
 */

/**@fn bool WriteOptions::draw_necklace_kernel
 * @brief Whether to draw the necklace kernel.
 */

/**@fn bool WriteOptions::draw_bead_ids
 * @brief Whether to draw the bead IDs in the beads.
 */

/**@fn bool WriteOptions::draw_feasible_intervals
 * @brief Whether to draw the feasible intervals.
 */

/**@fn bool WriteOptions::draw_valid_intervals
 * @brief Whether to draw the valid intervals.
 */

/**@fn bool WriteOptions::draw_region_angles
 * @brief Whether to draw lines from the necklace kernel through the region centroids.
 */

/**@fn bool WriteOptions::draw_bead_angles
 * @brief Whether to draw lines from the necklace kernel to the beads.
 */

} // namespace necklace_map
} // namespace cartocrow
