/*
The GeoViz library implements algorithmic geo-visualization methods,
developed at TU Eindhoven.
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 20-11-2019
*/

#ifndef GEOVIZ_COMMON_DETAIL_SVG_PATH_PARSER_H
#define GEOVIZ_COMMON_DETAIL_SVG_PATH_PARSER_H

#include <string>

#include "geoviz/common/core_types.h"


namespace geoviz
{
namespace detail
{

class SvgPathConverter
{
 public:
  ///@name Absolute coordinate methods.
  ///@{
  void MoveTo(const Point& to);
  void LineTo(const Point& to);
  void QuadBezierTo(const Point& control, const Point& to);
  void ContinueQuadBezierTo(const Point& to);
  void CubeBezierTo(const Point& control_1, const Point& control_2, const Point& to);
  void ContinueCubeBezierTo(const Point& control_2, const Point& to);
  void EllipticalArcTo
  (
    const Number& radius_x,
    const Number& radius_y,
    const Number& rotation_ccw_rad,
    const bool long_arc,
    const bool sweep_ccw,
    const Point& to
  );
  ///@}

  ///@name Relative coordinate methods.
  ///@{
  void MoveTo(const Vector& to);
  void LineTo(const Vector& to);
  void QuadBezierTo(const Vector& control, const Vector& to);
  void ContinueQuadBezierTo(const Vector& to);
  void CubeBezierTo(const Vector& control_1, const Vector& control_2, const Vector& to);
  void ContinueCubeBezierTo(const Vector& control_2, const Vector& to);
  void EllipticalArcTo
  (
    const Number& radius_x,
    const Number& radius_y,
    const Number& rotation_ccw_rad,
    const bool long_arc,
    const bool sweep_ccw,
    const Vector& to
  );
  ///@}

  void Close();

 protected:
  // Methods that can be overwritten when implementing this interface.
  // When methods are called that are not reimplemented, a fatal error is logged.

  virtual void MoveTo_(const Point& to) = 0;
  virtual void LineTo_(const Point& to);
  virtual void QuadBezierTo_(const Point& control, const Point& to);
  virtual void CubeBezierTo_(const Point& control_1, const Point& control_2, const Point& to);
  virtual void EllipticalArcTo_
  (
    const Number& radius_x,
    const Number& radius_y,
    const Number& rotation_ccw_rad,
    const bool long_arc,
    const bool sweep_ccw,
    const Point& to
  );

  virtual void Close_();

 private:
  Point previous_;
  Vector previous_control_;  // From previous control to previous point.
}; // class SvgPathConverter


class SvgPathParser
{
 public:
  bool operator()(const std::string& path, SvgPathConverter& converter);
}; // class SvgPathParser

} // namespace detail
} // namespace geoviz

#endif //GEOVIZ_COMMON_DETAIL_SVG_PATH_PARSER_H
