/*
Parser for SVG point coordinates.
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

#ifndef CONSOLE_COMMON_DETAIL_SVG_POINT_PARSER_H
#define CONSOLE_COMMON_DETAIL_SVG_POINT_PARSER_H

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
  Number toNumber(const std::string& str);
  Number toX(const std::string& str);
  Number toY(const std::string& str);
  Point toPoint(const std::string& str_x, const std::string& str_y);
  Vector toVector(const std::string& str_x, const std::string& str_y);

  Number getNumber(std::stringstream& ss);
  Number getX(std::stringstream& ss);
  Number getY(std::stringstream& ss);
  Point getPoint(std::stringstream& ss);
  Vector getVector(std::stringstream& ss);
}; // class SvgPointParser

} // namespace detail
} // namespace geoviz

#endif //CONSOLE_COMMON_DETAIL_SVG_POINT_PARSER_H
