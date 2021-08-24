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

#ifndef CARTOCROW_NECKLACE_MAP_IO_WRITE_OPTIONS_H
#define CARTOCROW_NECKLACE_MAP_IO_WRITE_OPTIONS_H

#include <memory>


namespace cartocrow
{
namespace necklace_map
{

struct WriteOptions
{
  using Ptr = std::shared_ptr<WriteOptions>;

  static Ptr Default();

  static Ptr Debug();

  int pixel_width;

  int region_precision;
  double region_opacity;
  double bead_opacity;
  double bead_id_font_size_px;

  bool draw_necklace_curve;
  bool draw_necklace_kernel;
  bool draw_bead_ids;

  bool draw_feasible_intervals;
  bool draw_valid_intervals;
  bool draw_region_angles;
  bool draw_bead_angles;
}; // struct WriteOptions

} // namespace necklace_map
} // namespace cartocrow

#endif //CARTOCROW_NECKLACE_MAP_IO_WRITE_OPTIONS_H
