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

#include "svg_visitor.h"

#include <glog/logging.h>

#include "geoviz/common/detail/svg_point_parser.h"


namespace geoviz
{
namespace detail
{
namespace
{

constexpr const char* kElementSvg = "svg";
constexpr const char* kElementPath = "path";
constexpr const char* kElementCircle = "circle";
constexpr const char* kElementLine = "line";
constexpr const char* kElementPolygon = "polygon";
constexpr const char* kElementPolyline = "polyline";
constexpr const char* kElementRectangle = "rect";
constexpr const char* kElementEllipse = "ellipse";

constexpr const char* kAttributePathCommands = "d";
constexpr const char* kAttributeCircleCenterX = "cx";
constexpr const char* kAttributeCircleCenterY = "cy";
constexpr const char* kAttributeCircleRadius = "r";
constexpr const char* kAttributeLineX_1 = "x1";
constexpr const char* kAttributeLineY_1 = "y1";
constexpr const char* kAttributeLineX_2 = "x2";
constexpr const char* kAttributeLineY_2 = "y2";
constexpr const char* kAttributePolygonPoints = "points";
constexpr const char* kAttributePolylinePoints = "points";
constexpr const char* kAttributeRectangleCornerX = "x";
constexpr const char* kAttributeRectangleCornerY = "y";
constexpr const char* kAttributeRectangleWidth = "width";
constexpr const char* kAttributeRectangleHeight = "height";
constexpr const char* kAttributeEllipseCenterX = "cx";
constexpr const char* kAttributeEllipseCenterY = "cy";
constexpr const char* kAttributeEllipseRadiusX = "rx";
constexpr const char* kAttributeEllipseRadiusY = "ry";

} // anonymous namespace


/**@class SvgVisitor
 * @brief An XML visitor that handles SVG geometry elements.
 *
 * Note that only a subset of the SVG elements are visited; the rest is traversed but otherwise ignored.
 */

/**@brief Enter an XML element.
 * @param element the XML element.
 * @param attributes the first attribute and a pointer to the next.
 * @return whether the element should be traversed further.
 */
bool SvgVisitor::VisitEnter(const tinyxml2::XMLElement& element, const tinyxml2::XMLAttribute* attributes)
{
  std::string name = element.Name();
  std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) { return std::tolower(c); });

  if (name == kElementSvg)
  {
    VisitSvg(attributes);
    return true;
  }
  if (name == kElementPath)
  {
    std::string commands;
    CHECK(FindAttribute(attributes, kAttributePathCommands, commands));

    return VisitPath(commands, attributes);
  }
  else if (name == kElementCircle)
  {
    const std::string names[] =
    {
      kAttributeCircleCenterX,
      kAttributeCircleCenterY,
      kAttributeCircleRadius
    };
    std::string values[3];
    CHECK(FindAttributes(attributes, 3, names, values));

    try
    {
      SvgPointParser pp;
      return VisitCircle(pp.Pt(values[0], values[1]), pp.N(values[2]), attributes);
    }
    catch (...) { return false; }
  }
  else if (name == kElementLine)
  {
    const std::string names[] =
    {
      kAttributeLineX_1,
      kAttributeLineY_1,
      kAttributeLineX_2,
      kAttributeLineY_2
    };
    std::string values[4];
    CHECK(FindAttributes(attributes, 4, names, values));

    try
    {
      SvgPointParser pp;
      return VisitLine(pp.Pt(values[0], values[1]), pp.Pt(values[2], values[3]), attributes);
    }
    catch (...) { return false; }
  }
  else if (name == kElementPolygon)
  {
    std::string points;
    CHECK(FindAttribute(attributes, kAttributePolygonPoints, points));

    return VisitPolygon(points, attributes);
  }
  else if (name == kElementPolyline)
  {
    std::string points;
    CHECK(FindAttribute(attributes, kAttributePolylinePoints, points));

    return VisitPolyline(points, attributes);
  }
  else if (name == kElementRectangle)
  {
    const std::string names[] =
    {
      kAttributeRectangleCornerX,
      kAttributeRectangleCornerY,
      kAttributeRectangleWidth,
      kAttributeRectangleHeight
    };
    std::string values[] = {0, 0, 0, 0};
    FindAttributes(attributes, 4, names, values);

    try
    {
      SvgPointParser pp;
      return VisitRectangle(pp.Pt(values[0], values[1]), pp.N(values[2]), pp.N(values[3]), attributes);
    }
    catch (...) { return false; }
  }
  else if (name == kElementEllipse)
  {
    const std::string names[] =
    {
      kAttributeEllipseCenterX,
      kAttributeEllipseCenterY,
      kAttributeEllipseRadiusX,
      kAttributeEllipseRadiusY
    };
    std::string values[4];
    CHECK(FindAttributes(attributes, 4, names, values));

    try
    {
      SvgPointParser pp;
      return VisitEllipse(pp.Pt(values[0], values[1]), pp.N(values[2]), pp.N(values[3]), attributes);
    }
    catch (...) { return false; }
  }

  // Traverse other elements.
  return true;
}

/**@brief Visit an svg element.
 *
 * Note that svg elements should always be traversed further.
 * @param attributes the first attribute and a pointer to the next.
 * Note that these attributes include the other parameters.
 */
void SvgVisitor::VisitSvg(const tinyxml2::XMLAttribute* attributes)
{}

/**@brief Visit a line element.
 * @param point_1 one endpoint.
 * @param point_2 the other endpoint.
 * @param attributes the first attribute and a pointer to the next.
 * Note that these attributes include the other parameters.
 * @return whether the element should be traversed further.
 */
bool SvgVisitor::VisitLine(const Point& point_1, const Point& point_2, const tinyxml2::XMLAttribute* attributes)
{
  return true;
}

/**@brief Visit a rectangle element.
 * @param corner one corner, generally the lexicographically smallest.
 * @param width the width.
 * @param height the height.
 * @param attributes the first attribute and a pointer to the next.
 * Note that these attributes include the other parameters.
 * @return whether the element should be traversed further.
 */
bool SvgVisitor::VisitRectangle
(
  const Point& corner,
  const Number& width,
  const Number& height,
  const tinyxml2::XMLAttribute* attributes
)
{
  return true;
}

/**@brief Visit a polygon element.
 * @param points a space-separated list of point coordinates; the coordinates per point are comma-separated.
 * @param attributes the first attribute and a pointer to the next.
 * Note that these attributes include the other parameters.
 * @return whether the element should be traversed further.
 */
bool SvgVisitor::VisitPolygon(const std::string& points, const tinyxml2::XMLAttribute* attributes)
{
  return true;
}

/**@brief Visit a polyline element.
 * @param points a space-separated list of point coordinates; the coordinates per point are comma-separated.
 * @param attributes the first attribute and a pointer to the next.
 * Note that these attributes include the other parameters.
 * @return whether the element should be traversed further.
 */
bool SvgVisitor::VisitPolyline(const std::string& points, const tinyxml2::XMLAttribute* attributes)
{
  return true;
}

/**@brief Visit a circle element.
 * @param center the circle center.
 * @param radius the circle radius.
 * @param attributes the first attribute and a pointer to the next.
 * Note that these attributes include the other parameters.
 * @return whether the element should be traversed further.
 */
bool SvgVisitor::VisitCircle(const Point& center, const Number& radius, const tinyxml2::XMLAttribute* attributes)
{
  return true;
}

/**@brief Visit a axis-aligned elipse element.
 * @param center the ellipse center.
 * @param radius_x the horizontal radius.
 * @param radius_y the vertical radius.
 * @param attributes the first attribute and a pointer to the next.
 * Note that these attributes include the other parameters.
 * @return whether the element should be traversed further.
 */
bool SvgVisitor::VisitEllipse
(
  const Point& center,
  const Number& radius_x,
  const Number& radius_y,
  const tinyxml2::XMLAttribute* attributes
)
{
  return true;
}

/**@brief Visit a path element.
 * @param commands a space-separated list of path commands.
 * Most commands will be followed by a space-separated list of coordinates or scalars.
 * @param attributes the first attribute and a pointer to the next.
 * Note that these attributes include the other parameters.
 * @return whether the element should be traversed further.
 */
bool SvgVisitor::VisitPath(const std::string& commands, const tinyxml2::XMLAttribute* attributes)
{
  return true;
}

/**@brief Find an attribute with a given name.
 * @param attributes the complete list of attributes.
 * @param name the attribute to search for.
 * @param value the string value of the attribute, if found or an empty string otherwise.
 * @return whether the searched attribute is part of the list.
 */
bool
SvgVisitor::FindAttribute(const tinyxml2::XMLAttribute* attributes, const std::string& name, std::string& value) const
{
  for (const tinyxml2::XMLAttribute* attribute = attributes; attribute != nullptr; attribute = attribute->Next())
  {
    if (name == attribute->Name())
    {
      value = attribute->Value();
      return true;
    }
  }
  return false;
}

/**@brief Find a set of attributes.
 * @param attributes the complete list of attributes.
 * @param num the number of attributes to search.
 * @param names an array of attributes to search for.
 * Note that if this collection may have duplicate names, in which case these will be assigned the same values.
 * @param values the string value of each attribute, if found or an empty string otherwise.
 * @return whether all searched attributes are part of the list.
 */
bool SvgVisitor::FindAttributes
(
  const tinyxml2::XMLAttribute* attributes,
  const size_t num,
  const std::string* names,
  std::string* values
) const
{
  std::vector<bool> found(num, false);
  for (const tinyxml2::XMLAttribute* attribute = attributes; attribute != nullptr; attribute = attribute->Next())
  {
    for (size_t n = 0; n < num; ++n)
    {
      if (names[n] == attribute->Name())
      {
        values[n] = attribute->Value();
        found[n] = true;
        break;
      }
    }
  }
  return std::all_of(found.begin(), found.end(), [](bool b) { return b; });
}

} // namespace detail
} // namespace geoviz
