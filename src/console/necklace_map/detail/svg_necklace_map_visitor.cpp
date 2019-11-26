/*
XML visitor for SVG necklace map input.
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

#include "svg_necklace_map_visitor.h"

#include <glog/logging.h>

#include "console/common/detail/svg_path_parser.h"
#include "console/common/detail/svg_point_parser.h"
#include "console/common/detail/svg_polygon_parser.h"


namespace geoviz
{
namespace detail
{
namespace
{

constexpr const char* kAttributeStyle = "style";
constexpr const char* kAttributeKernelX = "kx";
constexpr const char* kAttributeKernelY = "ky";

constexpr const char* kStyleMarkerNecklace = "stroke-dasharray";

constexpr const char* kCommandsRestrictionArcNecklace = "LlZzCcQqSsTt";

} // anonymous namespace

/**@class SvgNecklaceMapVisitor
 * @brief A XML visitor that handles SVG necklace map input geometry.
 */

/**@brief Main constructor.
 * @param regions the collection in which to collect the regions in the input.
 * @param necklace where to place the necklace.
 * @param region_index_by_id a map from region ID to index in the regions argument.
 */
SvgNecklaceMapVisitor::SvgNecklaceMapVisitor
(
  std::vector<Region>& regions,
  std::shared_ptr<NecklaceType>& necklace,
  std::unordered_map<std::string, size_t>& region_index_by_id
) : regions_(regions), necklace_(necklace), region_index_by_id_(region_index_by_id) {}


bool SvgNecklaceMapVisitor::visitCircle
(
  const Point& center,
  const Number& radius,
  const tinyxml2::XMLAttribute* attributes
)
{
  // Circles must be necklaces and necklaces must be dashed.
  std::string style;
  CHECK(findAttribute(attributes, kAttributeStyle, style));
  CHECK(style.find(kStyleMarkerNecklace) != std::string::npos);

  SetCircleNecklace(center, radius);
  return false;
}

bool SvgNecklaceMapVisitor::visitPath
(
  const std::string& commands,
  const tinyxml2::XMLAttribute* attributes
)
{
  CHECK(!commands.empty());

  std::string style, id;
  if
  (
    findAttribute(attributes, kAttributeStyle, style) &&
    style.find(kStyleMarkerNecklace) != std::string::npos
  )
  {
    // All dashed elements are necklaces.
    // Note that this may have to change into some identifying attribute.
    if (commands.find_first_of(kCommandsRestrictionArcNecklace) == std::string::npos)
      return SetArcNecklace(commands);
    else
    {
      const std::string names[] =
      {
        kAttributeKernelX,
        kAttributeKernelY
      };
      std::string values[2];
      CHECK(findAttributes(attributes, 2, names, values));

      try
      {
        SvgPointParser pp;
        return SetGenericNecklace(commands, pp.toPoint(values[0], values[1]));
      }
      catch (...) { return false; }
    }
  }
  else if (findAttribute(attributes, "id", id))
  {
    // Path elements with an ID are regions.
    CHECK(!id.empty());
    return AddRegion(commands, id, style);;
  }

  // Traverse other elements.
  return true;
}

/**@brief Set the necklace as a newly created circle necklace.
 * @param center the center of the circle.
 * @param radius the radius of the circle.
 * @return whether the necklace could be constructed.
 */
bool SvgNecklaceMapVisitor::SetCircleNecklace(const Point& center, const Number& radius)
{
  necklace_ = std::make_shared<gnm::CircleNecklace>(Circle(center, radius * radius));
  return true;
}

/**@brief Set the necklace as a newly created circular arc necklace.
 * @param commands the SVG path commands (including point coordinates).
 * @return whether the necklace could be constructed.
 */
bool SvgNecklaceMapVisitor::SetArcNecklace(const std::string& commands)
{
  LOG(FATAL) << "Not implemented yet.";
  return false;
}

/**@brief Set the necklace as a newly created generic necklace.
 * @param commands the SVG path commands (including point coordinates).
 * @param kernel the kernel of the star-shaped necklace polygon.
 * @return whether the necklace could be constructed.
 */
bool SvgNecklaceMapVisitor::SetGenericNecklace(const std::string& commands, const Point& kernel)
{
  LOG(FATAL) << "Not implemented yet.";
  return false;
}

/**@brief Add a region based on a SVG path.
 * @param commands the SVG path commands (including point coordinates).
 * @param id the ID of the region.
 *
 * These IDs often follow ISO-3166-2 (ISO-3166-1 alpha-2, possibly followed by a
 * subdivision number), or ISO-3166-1 alpha-3. However, any set of unique IDs is
 * allowed.
 *
 * Note that the ID does not have to unique. If a duplicate ID is encountered, the
 * polygon is added to the region with the same ID.
 * @param style the CSS style of teh region polygon. Note that this style will be reused
 * for the output regions.
 * @return whether the region could be parsed correctly.
 */
bool SvgNecklaceMapVisitor::AddRegion(const std::string& commands, const std::string& id, const std::string& style)
{
  // Get the region with the given ID, or create a new one if it does not yet exist.
  const size_t n = region_index_by_id_.insert({id, regions_.size()}).first->second;
  if (n == regions_.size()) regions_.emplace_back(id);
  Region& region = regions_[n];
  CHECK_EQ(id, region.id);

  region.style = style;

  // Interpret the commands as a region.
  SvgPolygonConverter converter(region.shape);
  SvgPathParser()(commands, converter);
  return true;
}

} // namespace detail
} // namespace geoviz
