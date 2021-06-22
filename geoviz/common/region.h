/*
The GeoViz library implements algorithmic geo-visualization methods,
developed at TU Eindhoven.
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 07-11-2019
*/

#ifndef GEOVIZ_NECKLACE_MAP_REGION_H
#define GEOVIZ_NECKLACE_MAP_REGION_H

#include <string>
#include <vector>

#include "geoviz/common/core_types.h"


namespace geoviz
{

///@brief A geographically significant shape.
class Region
{
 public:
  //using PolygonSet = CGAL::Polygon_set_2<Kernel>; // Polygon_set_2 fails to initialize with the EPIC kernel: using simple list instead.
  using PolygonSet = std::vector<Polygon_with_holes>;

 public:
  explicit Region(const std::string& id);

  bool IsPoint() const;

  bool IsValid() const;

  bool MakeValid();

  void MakeSimple(Polygon& simple);

  std::string id;
  PolygonSet shape;

  std::string style;
}; // class Region

} // namespace geoviz

#endif //GEOVIZ_NECKLACE_MAP_REGION_H
