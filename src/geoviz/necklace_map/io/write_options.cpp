/*
The Necklace Map library implements the algorithmic
geo-visualization method by the same name, developed by
Bettina Speckmann and Kevin Verbeek at TU Eindhoven
(DOI: 10.1109/TVCG.2010.180 & 10.1142/S021819591550003X).
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

#include "write_options.h"


namespace geoviz
{
namespace necklace_map
{

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

} // namespace necklace_map
} // namespace geoviz
