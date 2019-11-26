/*
Generic XML visitor for SVG geometry.
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

#ifndef CONSOLE_COMMON_DETAIL_SVG_VISITOR_H
#define CONSOLE_COMMON_DETAIL_SVG_VISITOR_H

#include <string>
#include <tinyxml2.h>

#include "geoviz/common/core_types.h"


namespace geoviz
{
namespace detail
{

// Note that while the SVG input is stored in an XML file, it does not make use of the
// main feature that makes XML preferable over JSON: validatibility (i.e. assigning a
// schema). This means that we do not need to use a comprehensive XML library such as
// xerces; we can instead use a lightweight library such as tinyxml.
class SvgVisitor : public tinyxml2::XMLVisitor
{
 public:
  bool 	VisitEnter
  (
    const tinyxml2::XMLElement& element,
    const tinyxml2::XMLAttribute* attributes
  );

 protected:
  virtual bool visitLine
  (
    const Point& point_1,
    const Point& point_2,
    const tinyxml2::XMLAttribute* attributes
  );

  virtual bool visitRectangle
  (
    const Point& corner,
    const Number& width,
    const Number& height,
    const tinyxml2::XMLAttribute* attributes
  );

  virtual bool visitPolygon
  (
    const std::string& points,
    const tinyxml2::XMLAttribute* attributes
  );

  virtual bool visitPolyline
  (
    const std::string& points,
    const tinyxml2::XMLAttribute* attributes
  );

  virtual bool visitCircle
  (
    const Point& center,
    const Number& radius,
    const tinyxml2::XMLAttribute* attributes
  );

  virtual bool visitEllipse
  (
    const Point& center,
    const Number& radius_x,
    const Number& radius_y,
    const tinyxml2::XMLAttribute* attributes
  );

  virtual bool visitPath
  (
    const std::string& commands,
    const tinyxml2::XMLAttribute* attributes
  );

  bool findAttribute
  (
    const tinyxml2::XMLAttribute* attributes,
    const std::string& name,
    std::string& value
  ) const;

  bool findAttributes
  (
    const tinyxml2::XMLAttribute* attributes,
    const size_t num,
    const std::string* names,
    std::string* values
  ) const;
}; // class SvgVisitor

} // namespace detail
} // namespace geoviz

#endif //CONSOLE_COMMON_DETAIL_SVG_VISITOR_H
