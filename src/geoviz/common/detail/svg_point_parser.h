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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 26-11-2019
*/

#ifndef GEOVIZ_COMMON_DETAIL_SVG_POINT_PARSER_H
#define GEOVIZ_COMMON_DETAIL_SVG_POINT_PARSER_H

#include <sstream>
#include <string>

#include "geoviz/common/core_types.h"


namespace geoviz
{
namespace detail
{

class SvgPointParser
{
 public:
  Number N(const std::string& str);
  Number X(const std::string& str);
  Number Y(const std::string& str);
  Point Pt(const std::string& str_x, const std::string& str_y);
  Vector Vec(const std::string& str_x, const std::string& str_y);

  Number N(std::stringstream& ss);
  Number X(std::stringstream& ss);
  Number Y(std::stringstream& ss);
  Point Pt(std::stringstream& ss);
  Vector Vec(std::stringstream& ss);
}; // class SvgPointParser

} // namespace detail
} // namespace geoviz

#endif //GEOVIZ_COMMON_DETAIL_SVG_POINT_PARSER_H
