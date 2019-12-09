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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 20-11-2019
*/

#include "svg_path_parser.h"

#include <cmath>
#include <sstream>

#include <glog/logging.h>

#include "console/common/detail/svg_point_parser.h"


namespace geoviz
{
namespace detail
{
namespace
{

constexpr char kAbsoluteMove = 'M';
constexpr char kAbsoluteLine = 'L';
constexpr char kAbsoluteHorizontalLine = 'H';
constexpr char kAbsoluteVerticalLine = 'V';
constexpr char kAbsoluteQuadraticBezier = 'Q';
constexpr char kAbsoluteContinueQuadraticBezier = 'T';
constexpr char kAbsoluteCubicBezier = 'C';
constexpr char kAbsoluteContinueCubicBezier = 'S';
constexpr char kAbsoluteClose = 'Z';

constexpr char kRelativeMove = 'm';
constexpr char kRelativeLine = 'l';
constexpr char kRelativeHorizontalLine = 'h';
constexpr char kRelativeVerticalLine = 'v';
constexpr char kRelativeQuadraticBezier = 'q';
constexpr char kRelativeContinueQuadraticBezier = 't';
constexpr char kRelativeCubicBezier = 'c';
constexpr char kRelativeContinueCubicBezier = 's';
constexpr char kRelativeClose = 'z';

} // anonymous namespace


/**@class SvgPathConverter
 * @brief An interface for converting an SVG path element to another data type.
 */

/**@brief Move to a point.
 * @param to the absolute coordinates of the point.
 */
inline void SvgPathConverter::MoveTo(const Point& to)
{
  MoveTo_(to);
  previous_ = to;
}

/**@brief Draw a straight line segment.
 *
 * This line segment is incident to the previous (end)point.
 * @param to @parblock the absolute coordinates of the other endpoint of the line segment.
 *
 * Either coordinate may be std::nan to indicate an axis aligned line segment.
 * @endparblock
 */
inline void SvgPathConverter::LineTo(const Point& to)
{
  if (std::isnan(to.x())) LineTo_(Point(previous_.x(), to.y()));
  else if (std::isnan(to.x())) LineTo_(Point(to.x(), previous_.x()));
  else LineTo_(to);
  previous_ = to;
}

/**@brief Draw a quadratic Bezier curve.
 *
 * This Bezier curve starts at the previous (end)point.
 * @param control the absolute coordinates of the Bezier control point.
 * @param to the absolute coordinates of the endpoint of the curve.
 */
inline void SvgPathConverter::QuadBezierTo(const Point& control, const Point& to)
{
  QuadBezierTo_(control, to);
  previous_ = to;
  previous_control_ = previous_ - control;
}

/**@brief Draw a quadratic Bezier curve as continuation of the previous Bezier curve.
 *
 * This Bezier curve starts at the previous endpoint. Its control point mirrors the control point of the previous curve in the previous endpoint.
 * @param to the absolute coordinates of the endpoint of the curve.
 */
inline void SvgPathConverter::ContinueQuadBezierTo(const Point& to)
{
  const Point control = previous_ + previous_control_;
  QuadBezierTo_(control, to);
  previous_ = to;
  previous_control_ = previous_ - control;
}

/**@brief Draw a cubic Bezier curve.
 *
 * This Bezier curve starts at the previous (end)point.
 * @param control_1 the absolute coordinates of the first Bezier control point.
 * @param control_2 the absolute coordinates of the second Bezier control point.
 * @param to the absolute coordinates of the endpoint of the curve.
 */
inline void SvgPathConverter::CubeBezierTo(const Point& control_1, const Point& control_2, const Point& to)
{
  CubeBezierTo_(control_1, control_2, to);
  previous_ = to;
  previous_control_ = previous_ - control_2;
}

/**@brief Draw a cubic Bezier curve as continuation of the previous Bezier curve.
 *
 * This Bezier curve starts at the previous endpoint. Its first control point mirrors the last control point of the previous curve in the previous endpoint.
 * @param control_2 the absolute coordinates of the second Bezier control point.
 * @param to the absolute coordinates of the endpoint of the curve.
 */
inline void SvgPathConverter::ContinueCubeBezierTo(const Point& control_2, const Point& to)
{
  const Point control_1 = previous_ + previous_control_;
  CubeBezierTo_(control_1, control_2, to);
  previous_ = to;
  previous_control_ = previous_ - control_2;
}

/**@brief Draw an arc on an ellipse.
 *
 * This arc curve starts at the previous (end)point.
 *
 * For a more in-depth description of these curves, please refer to SVG path documentation, e.g. https://developer.mozilla.org/en-US/docs/Web/SVG/Tutorial/Paths
 * @param radius_x the horizontal radius of the reference ellipse.
 * @param radius_y the vertical radius of the reference ellipse.
 * @param rotation_ccw_rad the counterclockwise rotation of the reference ellipse in radians.
 * @param long_arc whether the longer arc on the ellipse should be used.
 * @param sweep_ccw whether the arc should traverse the ellipse in counterclockwise direction from the previous point.
 * @param to the absolute coordinates of the endpoint of the arc.
 */
inline void SvgPathConverter::EllipticalArcTo
(
  const Number& radius_x,
  const Number& radius_y,
  const Number& rotation_ccw_rad,
  const bool long_arc,
  const bool sweep_ccw,
  const Point& to
)
{
  EllipticalArcTo_(radius_x, radius_y, rotation_ccw_rad, long_arc, sweep_ccw, to);
  previous_ = to;
}

/**@brief Move to a point.
 * @param to the coordinates of the point relative to the previous (end)point.
 */
inline void SvgPathConverter::MoveTo(const Vector& to)
{
  MoveTo(previous_ + to);
}

/**@brief Draw a straight line segment.
 *
 * This line segment is incident to the previous (end)point.
 * @param to @parblock the coordinates of the other endpoint of the line segment relative to the previous (end)point.
 *
 * Either coordinate may be std::nan to indicate an axis aligned line segment.
 * @endparblock
 */
inline void SvgPathConverter::LineTo(const Vector& to)
{
  LineTo(previous_ + to);
}

/**@brief Draw a quadratic Bezier curve.
 *
 * This Bezier curve starts at the previous (end)point.
 * @param control the coordinates of the Bezier control point relative to the previous (end)point.
 * @param to the coordinates of the endpoint of the curve relative to the previous (end)point.
 */
inline void SvgPathConverter::QuadBezierTo(const Vector& control, const Vector& to)
{
  QuadBezierTo(previous_ + control, previous_ + to);
}

/**@brief Draw a quadratic Bezier curve as continuation of the previous Bezier curve.
 *
 * This Bezier curve starts at the previous endpoint. Its control point mirrors the control point of the previous curve in the previous endpoint.
 * @param to the coordinates of the endpoint of the curve relative to the previous (end)point.
 */
inline void SvgPathConverter::ContinueQuadBezierTo(const Vector& to)
{
  ContinueQuadBezierTo(previous_ + to);
}

/**@brief Draw a cubic Bezier curve.
 *
 * This Bezier curve starts at the previous (end)point.
 * @param control_1 the coordinates of the first Bezier control point relative to the previous (end)point.
 * @param control_2 the coordinates of the second Bezier control point relative to the previous (end)point.
 * @param to the coordinates of the endpoint of the curve relative to the previous (end)point.
 */
inline void SvgPathConverter::CubeBezierTo(const Vector& control_1, const Vector& control_2, const Vector& to)
{
  CubeBezierTo(previous_ + control_1, previous_ + control_2, previous_ + to);
}

/**@brief Draw a cubic Bezier curve as continuation of the previous Bezier curve.
 *
 * This Bezier curve starts at the previous endpoint. Its first control point mirrors the last control point of the previous curve in the previous endpoint.
 * @param control_2 the coordinates of the second Bezier control point relative to the previous (end)point.
 * @param to the coordinates of the endpoint of the curve relative to the previous (end)point.
 */
inline void SvgPathConverter::ContinueCubeBezierTo(const Vector& control_2, const Vector& to)
{
  ContinueCubeBezierTo(previous_ + control_2, previous_ + to);
}

/**@brief Draw an arc on an ellipse.
 *
 * This arc curve starts at the previous (end)point.
 *
 * For a more in-depth description of these curves, please refer to SVG path documentation, e.g. https://developer.mozilla.org/en-US/docs/Web/SVG/Tutorial/Paths
 * @param radius_x the horizontal radius of the reference ellipse.
 * @param radius_y the vertical radius of the reference ellipse.
 * @param rotation_ccw_rad the counterclockwise rotation of the reference ellipse in radians.
 * @param long_arc whether the longer arc on the ellipse should be used.
 * @param sweep_ccw whether the arc should traverse the ellipse in counterclockwise direction from the previous point.
 * @param to the coordinates of the endpoint of the arc relative to the previous (end)point.
 */
inline void SvgPathConverter::EllipticalArcTo
(
  const Number& radius_x,
  const Number& radius_y,
  const Number& rotation_ccw_rad,
  const bool long_arc,
  const bool sweep_ccw,
  const Vector& to
)
{
  EllipticalArcTo(radius_x, radius_y, rotation_ccw_rad, long_arc, sweep_ccw, previous_ + to);
}

/**@brief Draw a straight line to the start of the path.
 */
inline void SvgPathConverter::Close()
{
  Close_();
}

void SvgPathConverter::LineTo_(const Point& to)
{
  LOG(FATAL) << "Adding line not implemented.";
}

void SvgPathConverter::QuadBezierTo_(const Point& control, const Point& to)
{
  LOG(FATAL) << "Adding quadratic Bezier curve not implemented.";
}

void SvgPathConverter::CubeBezierTo_(const Point& control_1, const Point& control_2, const Point& to)
{
  LOG(FATAL) << "Adding cubic Bezier curve not implemented.";
}

void SvgPathConverter::EllipticalArcTo_
(
  const Number& radius_x,
  const Number& radius_y,
  const Number& rotation_ccw_rad,
  const bool long_arc,
  const bool sweep_ccw,
  const Point& to
)
{
  LOG(FATAL) << "Adding circular curve not implemented.";
}

void SvgPathConverter::Close_()
{
  LOG(FATAL) << "Closing shape not implemented.";
}


/**@class SvgPathParser
 * @brief Parser for SVG path strings.
 */

/**@brief Convert an SVG path string to some other data format.
 * @param path the SVG path string.
 * @param converter the converter to include path elements in another data format.
 * @return whether the path could be parsed correctly.
 */
bool SvgPathParser::operator()(const std::string& path, SvgPathConverter& converter)
{
  // Interpret the path.
  std::stringstream ss(path);
  SvgPointParser pp;
  while (ss)
  {
    try
    {
      char command;
      ss >> command;
      if (ss.eof())
        break;

      // Note that SVG uses a y-down coordinate system, while Leaflet expects north-up coordinates.
      // Also note that the order of parsing the stream is important, so it cannot be done as part of the converter method parameters (C++ has undefined parameter evaluation order).
      switch (command)
      {
        case kAbsoluteMove:
          converter.MoveTo(pp.Pt(ss));
          break;
        case kAbsoluteLine:
          converter.LineTo(pp.Pt(ss));
          break;
        case kAbsoluteHorizontalLine:
          converter.LineTo(Point(pp.X(ss), 0.0/0.0));
          break;
        case kAbsoluteVerticalLine:
          converter.LineTo(Point(0.0/0.0, pp.Y(ss)));
          break;
        case kAbsoluteQuadraticBezier:
        {
          const Point control = pp.Pt(ss);
          const Point point = pp.Pt(ss);
          converter.QuadBezierTo(control, point);
          break;
        }
        case kAbsoluteContinueQuadraticBezier:
          converter.ContinueQuadBezierTo(pp.Pt(ss));
          break;
        case kAbsoluteCubicBezier:
        {
          const Point control_1 = pp.Pt(ss);
          const Point control_2 = pp.Pt(ss);
          const Point point = pp.Pt(ss);
          converter.CubeBezierTo(control_1, control_2, point);
          break;
        }
        case kAbsoluteContinueCubicBezier:
        {
          const Point control_2 = pp.Pt(ss);
          const Point point = pp.Pt(ss);
          converter.ContinueCubeBezierTo(control_2, point);
          break;
        }
        case kAbsoluteClose:
          converter.Close();
        case kRelativeMove:
          converter.MoveTo(pp.Vec(ss));
          break;
        case kRelativeLine:
          converter.LineTo(pp.Vec(ss));
          break;
        case kRelativeHorizontalLine:
          converter.LineTo(Vector(pp.X(ss), 0.0/0.0));
          break;
        case kRelativeVerticalLine:
          converter.LineTo(Vector(0.0/0.0, pp.Y(ss)));
          break;
        case kRelativeQuadraticBezier:
        {
          const Vector control = pp.Vec(ss);
          const Vector point = pp.Vec(ss);
          converter.QuadBezierTo(control, point);
          break;
        }
        case kRelativeContinueQuadraticBezier:
          converter.ContinueQuadBezierTo(pp.Vec(ss));
          break;
        case kRelativeCubicBezier:
        {
          const Vector control_1 = pp.Vec(ss);
          const Vector control_2 = pp.Vec(ss);
          const Vector point = pp.Vec(ss);
          converter.CubeBezierTo(control_1, control_2, point);
          break;
        }
        case kRelativeContinueCubicBezier:
        {
          const Vector control_2 = pp.Vec(ss);
          const Vector point = pp.Vec(ss);
          converter.ContinueCubeBezierTo(control_2, point);
          break;
        }
        case kRelativeClose:
          converter.Close();
          break;
        default:
          LOG(FATAL) << "Unknown path command: " << command;
      }
    }
    catch (...)
    {
      return false;
    }
  }
  return true;
}

} // namespace detail
} // namespace geoviz
