/*
The Necklace Map library implements the algorithmic geo-visualization
method by the same name, developed by Bettina Speckmann and Kevin Verbeek
at TU Eindhoven (DOI: 10.1109/TVCG.2010.180 & 10.1142/S021819591550003X).
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 20-01-2020
*/

#ifndef GEOVIZ_NECKLACE_MAP_DETAIL_VALIDATE_SCALE_FACTOR_H
#define GEOVIZ_NECKLACE_MAP_DETAIL_VALIDATE_SCALE_FACTOR_H

#include <vector>

#include "geoviz/common/core_types.h"
#include "geoviz/necklace_map/necklace.h"


namespace geoviz
{
namespace necklace_map
{
namespace detail
{

class ValidateScaleFactor
{
 public:
  ValidateScaleFactor(const Number& scale_factor, const Number& buffer_rad = 0, const bool adjust_angle = true);

  bool operator()(Necklace::Ptr& necklace) const;

  bool operator()(std::vector<Necklace::Ptr>& necklaces) const;

  Number scale_factor;
  Number buffer_rad;
  bool adjust_angle;
}; // class ValidateScaleFactor

} // namespace detail
} // namespace necklace_map
} // namespace geoviz

#endif //GEOVIZ_NECKLACE_MAP_DETAIL_VALIDATE_SCALE_FACTOR_H
