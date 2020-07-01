/*
The GeoViz console applications implement algorithmic geo-visualization
methods, developed at TU Eindhoven.
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 13-05-2020
*/

#ifndef CONSOLE_COMMON_DETAIL_SVG_BEZIER_PARSER_H
#define CONSOLE_COMMON_DETAIL_SVG_BEZIER_PARSER_H

#include "geoviz/common/core_types.h"

#include "console/common/detail/svg_path_parser.h"
#include "geoviz/necklace_map/bezier_necklace.h"


namespace geoviz
{
namespace detail
{

class SvgBezierConverter : public SvgPathConverter
{
 public:
  using BezierNecklace = necklace_map::BezierNecklace;

 public:
  explicit SvgBezierConverter(BezierNecklace& shape);

 private:
  void MoveTo_(const Point& to) override;
  void LineTo_(const Point& to) override;
  void QuadBezierTo_(const Point& control, const Point& to) override;
  void CubeBezierTo_(const Point& control_1, const Point& control_2, const Point& to) override;
  void Close_() override;

  BezierNecklace& shape_;

  Point source_;
}; // class SvgBezierConverter

} // namespace detail
} // namespace geoviz

#endif //CONSOLE_COMMON_DETAIL_SVG_BEZIER_PARSER_H
