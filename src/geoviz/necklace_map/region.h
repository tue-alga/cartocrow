/*
The Necklace Map library implements the algorithmic geo-visualization
method by the same name, developed by Bettina Speckmann and Kevin Verbeek
at TU Eindhoven (DOI: 10.1109/TVCG.2010.180 & 10.1142/S021819591550003X).
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 07-11-19
*/

#ifndef GEOVIZ_NECKLACE_MAP_REGION_H
#define GEOVIZ_NECKLACE_MAP_REGION_H

#include <string>
#include <vector>

#include "geoviz/common/core_types.h"


namespace geoviz
{
namespace necklace_map
{

///@brief A region represents a country or other geographically significant shape.
class Region
{
 public:
  //using PolygonSet = CGAL::Polygon_set_2<Kernel>;
  // Polygon_set_2 fails to initialize with the EPIC kernel: using simple list instead.
  using PolygonSet = std::vector<Polygon>;  ///< The main shape of a region.

 public:
  Region(const std::string& id);

  ///@param strict whether the value must be strictly larger than 0.
  bool isValid(const bool strict = true) const;

  Polygon getExtent() const;

  std::string id;
  PolygonSet shape;
  Number value;

  std::string style; //TODO(tvl) this should probably be implemented differently. Note that preserving the style may still be necessary in some cases.
}; // class Region

} // namespace necklace_map
} // namespace geoviz

#endif //GEOVIZ_NECKLACE_MAP_REGION_H
