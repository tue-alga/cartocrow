/*
The Flow Map library implements the algorithmic geo-visualization
method by the same name, developed by Kevin Verbeek, Kevin Buchin,
and Bettina Speckmann at TU Eindhoven
(DOI: 10.1007/s00453-013-9867-z & 10.1109/TVCG.2011.202).
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 04-09-2020
*/

#include "svg_visitor.h"

#include <glog/logging.h>

#include "geoviz/common/detail/svg_bezier_parser.h"
#include "geoviz/common/detail/svg_path_parser.h"
#include "geoviz/common/detail/svg_point_parser.h"
#include "geoviz/common/detail/svg_polygon_parser.h"


namespace geoviz
{
namespace flow_map
{
namespace detail
{
namespace
{

constexpr const char* kElementSvg = "svg";

constexpr const char* kAttributePlaceId = "node_id";
constexpr const char* kAttributeStyle = "style";

constexpr const char* kCommandsRestrictionContextRegion = "CcQqSsTt";

} // anonymous namespace

/**@class SvgVisitor
 * @brief An XML visitor for handling SVG flow map input geometry.
 */

/**@brief Construct an XML visitor for handling SVG necklace map input geometry.
 * @param context the collection in which to collect the context regions in the input.
 * @param places the collection in which to collect the places on the flow map (e.g. root and leaf nodes).
 * @param strict_validity @parblock whether the context regions must be strictly valid.
 *
 * Otherwise some regions may be corrected if this will make them valid.
 * @endparblock
 */
SvgVisitor::SvgVisitor
(
  std::vector<Region>& context,
  std::vector<Place::Ptr>& places,
  const bool strict_validity /*= true*/
) :
  geoviz::detail::SvgVisitor(),
  context_(context),
  places_(places),
  strict_validity_(strict_validity)
{
  // Add the regions to the lookup table, while checking for duplicates.
  for (const Place::Ptr& place : places_)
  {
    const size_t next_index = id_to_place_index_.size();
    const size_t n = id_to_place_index_.insert({place->id, next_index}).first->second;
    CHECK_EQ(next_index, n);
  }
}

bool SvgVisitor::VisitExit(const tinyxml2::XMLElement& element)
{
  std::string name = element.Name();
  std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) { return std::tolower(c); });

  if (name == kElementSvg)
  {
    return FinalizeSvg();
  }
  return true;
}


void SvgVisitor::VisitSvg(const tinyxml2::XMLAttribute* attributes)
{}

bool
SvgVisitor::VisitCircle(const Point& center, const Number& radius, const tinyxml2::XMLAttribute* attributes)
{
  // Circles without id are ignored.
  std::string id;
  if (!FindAttribute(attributes, kAttributePlaceId, id))
    return false;

  // Add a place.
  AddPlace(id, center);
  return false;
}

bool SvgVisitor::VisitPath(const std::string& commands, const tinyxml2::XMLAttribute* attributes)
{
  CHECK(!commands.empty());

  // Add a context region.
  std::string style;
  FindAttribute(attributes, kAttributeStyle, style);

  // Context regions may not contain non-linear arcs.
  if(commands.find_first_of(kCommandsRestrictionContextRegion) == std::string::npos)
  {
    return AddRegion(commands, style);
  }
  else
  {
    // This path must be circular and represent a node.
    std::string id;
    CHECK(FindAttribute(attributes, kAttributePlaceId, id));

    // Add a place.
    AddPlace(id, commands);
  }
  return true;
}

bool SvgVisitor::FinalizeSvg()
{
  return true;
}

/**@brief Add a place to the flow map.
 * @param id the place ID.
 * @param point the position of the place in Cartesian coordinates.
 * @return whether the place was constructed correctly.
 */
bool SvgVisitor::AddPlace(const std::string& id, const geoviz::Point& point)
{
  PolarPoint position(point);

  // IDs must be unique.
  const size_t next_index = places_.size();
  const size_t n = id_to_place_index_.insert({id, next_index}).first->second;

  if (next_index == n)
    places_.push_back(std::make_shared<Place>(id, position));
  else
    places_[n]->position = position;

  return true;
}

/**@brief Add a node from a Bezier spline.
 * @param id the place ID.
 * @param commands the SVG path commands (including point coordinates).
 * @return whether the place was constructed correctly.
 */
bool SvgVisitor::AddPlace(const std::string& id, const std::string& commands)
{
  // Interpret the commands as a bezier spline.
  BezierSpline spline;
  geoviz::detail::SvgBezierConverter converter(spline);
  geoviz::detail::SvgPathParser()(commands, converter);

  // The spline must approximate a circle.
  Circle circle;
  CHECK(spline.ToCircle(circle, 0.05));

  PolarPoint position(circle.center());

  // Node IDs must be unique.
  const size_t next_index = places_.size();
  const size_t n = id_to_place_index_.insert({id, next_index}).first->second;

  if (next_index == n)
    places_.push_back(std::make_shared<Place>(id, position));
  else
    places_[n]->position = position;

  return true;
}

/**@brief Add a context region based on an SVG path.
 * @param commands the SVG path commands (including point coordinates).
 * @param style the CSS style of the region polygon. Note that this style will be reused
 * for the output regions.
 * @return whether the region could be parsed correctly.
 */
bool SvgVisitor::AddRegion(const std::string& commands, const std::string& style)
{
  // Note that these regions do not need an ID.
  context_.emplace_back("");
  Region& region = context_.back();

  // Interpret the commands as a region.
  geoviz::detail::SvgPolygonConverter converter(region.shape);
  geoviz::detail::SvgPathParser()(commands, converter);

  region.style = style;

  if (strict_validity_)
    CHECK(region.IsValid()) << "Invalid region: " << region.id;
  else
    region.MakeValid();

  return true;
}

} // namespace detail
} // namespace flow_map
} // namespace geoviz
