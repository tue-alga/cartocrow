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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 07-11-2019
*/

#ifndef GEOVIZ_NECKLACE_MAP_NECKLACE_H
#define GEOVIZ_NECKLACE_MAP_NECKLACE_H

#include <memory>
#include <vector>

#include "geoviz/common/core_types.h"
#include "geoviz/necklace_map/bead.h"
#include "geoviz/necklace_map/bezier_necklace.h"
#include "geoviz/necklace_map/circle_necklace.h"
#include "geoviz/necklace_map/necklace_shape.h"


namespace geoviz
{
namespace necklace_map
{

struct Necklace
{
  using Ptr = std::shared_ptr<Necklace>;

  Necklace(const std::string& id, const NecklaceShape::Ptr& shape);

  void SortBeads();

  std::string id;

  NecklaceShape::Ptr shape;
  std::vector<Bead::Ptr> beads;
}; // struct Necklace

} // namespace necklace_map
} // namespace geoviz

#endif //GEOVIZ_NECKLACE_MAP_NECKLACE_H
