/*
The Necklace Map console application implements the algorithmic
geo-visualization method by the same name, developed by
Bettina Speckmann and Kevin Verbeek at TU Eindhoven
(DOI: 10.1109/TVCG.2010.180 & 10.1142/S021819591550003X).
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

#include "svg_visitor.h"

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

constexpr const char* kElementSvg = "svg";

constexpr const char* kAttributeRegionId = "id";
constexpr const char* kAttributeNecklaceId = "necklace_id";
constexpr const char* kAttributeStyle = "style";
constexpr const char* kAttributeKernelX = "kx";
constexpr const char* kAttributeKernelY = "ky";

constexpr const char* kStyleMarkerNecklace = "stroke-dasharray";

constexpr const char* kCommandsRestrictionArcNecklace = "LlZzCcQqSsTt";

} // anonymous namespace

/**@class NecklaceMapSvgVisitor
 * @brief An XML visitor for handling SVG necklace map input geometry.
 */

/**@typedef NecklaceMapSvgVisitor::NecklaceTypePtr
 * @brief a pointer for a dynamic necklace type.
 */

/**@brief Construct an XML visitor for handling SVG necklace map input geometry.
 * @param regions the collection in which to collect the regions in the input.
 * @param necklaces the collection in which to collect the necklaces in the input.
 */
NecklaceMapSvgVisitor::NecklaceMapSvgVisitor
(
  std::vector<necklace_map::MapElement::Ptr>& elements,
  std::vector<necklace_map::Necklace::Ptr>& necklaces,
  const bool strict_validity /*= true*/
) : SvgVisitor(), elements_(elements), necklaces_(necklaces), strict_validity_(strict_validity)
{
  // Add the regions to the lookup table, while checking for duplicates.
  for (const MapElement::Ptr& element : elements_)
  {
    CHECK_NOTNULL(element);

    const size_t next_index = id_to_region_index_.size();
    const size_t n = id_to_region_index_.insert({element->region.id, next_index}).first->second;
    CHECK_EQ(next_index, n);
  }
  necklace_ids_.resize(elements_.size());

  // Note that the necklace IDs only apply while in the same SVG, so their ID to index table does not have to be rebuilt.
}

bool NecklaceMapSvgVisitor::VisitExit(const tinyxml2::XMLElement& element)
{
  std::string name = element.Name();
  std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) { return std::tolower(c); });

  if (name == kElementSvg)
  {
    return FinalizeSvg();
  }
  return true;
}

bool
NecklaceMapSvgVisitor::VisitCircle(const Point& center, const Number& radius, const tinyxml2::XMLAttribute* attributes)
{
  // Circles must be necklaces and necklaces must be dashed.
  std::string style;
  CHECK(FindAttribute(attributes, kAttributeStyle, style));
  CHECK(style.find(kStyleMarkerNecklace) != std::string::npos);

  // Necklaces must have an ID.
  std::string necklace_id;
  CHECK(FindAttribute(attributes, kAttributeNecklaceId, necklace_id));

  AddCircleNecklace(necklace_id, center, radius);
  return false;
}

bool NecklaceMapSvgVisitor::VisitPath(const std::string& commands, const tinyxml2::XMLAttribute* attributes)
{
  CHECK(!commands.empty());

  std::string id, necklace_id, style;
  const bool has_necklace_id = FindAttribute(attributes, kAttributeNecklaceId, necklace_id);

  if
  (
    FindAttribute(attributes, kAttributeStyle, style) &&
    style.find(kStyleMarkerNecklace) != std::string::npos
  )
  {
    // All dashed elements are necklaces.
    // Note that this may have to change into some identifying attribute.

    // Necklaces must have an ID.
    CHECK(has_necklace_id);

    if (commands.find_first_of(kCommandsRestrictionArcNecklace) == std::string::npos)
      return AddArcNecklace(necklace_id, commands);
    else
    {
      const std::string names[] =
      {
        kAttributeKernelX,
        kAttributeKernelY
      };
      std::string values[2];
      CHECK(FindAttributes(attributes, 2, names, values));

      try
      {
        SvgPointParser pp;
        return AddGenericNecklace(necklace_id, commands, pp.Pt(values[0], values[1]));
      }
      catch (...) { return false; }
    }
  }
  else if (FindAttribute(attributes, kAttributeRegionId, id))
  {
    // Path elements with an ID are regions.
    CHECK(!id.empty());
    return AddMapElement(id, commands, necklace_id, style);;
  }

  // Traverse other elements.
  return true;
}

/**@brief Connect the regions to their respective necklace.
 * @return whether the SVG was finalized correctly.
 */
bool NecklaceMapSvgVisitor::FinalizeSvg()
{
  CHECK_EQ(elements_.size(), necklace_ids_.size());
  for (size_t n = 0; n < elements_.size(); ++n)
  {
    MapElement::Ptr& element = elements_[n];
    CHECK_NOTNULL(element);

    const std::string& necklace_id = necklace_ids_[n];
    LookupTable::const_iterator index_iter = id_to_necklace_index_.find(necklace_id);
    CHECK(index_iter != id_to_necklace_index_.end());

    necklaces_[index_iter->second]->beads.push_back(element);
  }
  return true;
}

/**@brief Add a circle necklace.
 * @param id the necklace ID.
 * @param center the center of the circle.
 * @param radius the radius of the circle.
 * @return whether the necklace was constructed correctly.
 */
bool NecklaceMapSvgVisitor::AddCircleNecklace(const std::string& id, const Point& center, const Number& radius)
{
  // Necklace IDs must be unique.
  const size_t next_index = necklaces_.size();
  const size_t n = id_to_necklace_index_.insert({id, next_index}).first->second;
  CHECK_EQ(next_index, n);

  necklaces_.push_back
  (
    std::make_shared<necklace_map::Necklace>
    (
    std::make_shared<necklace_map::CircleNecklace>(Circle(center, radius * radius))
    )
  );
  return true;
}

/**@brief Add a circular arc necklace.
 * @param id the necklace ID.
 * @param commands the SVG path commands (including point coordinates).
 * @return whether the necklace was constructed correctly.
 */
bool NecklaceMapSvgVisitor::AddArcNecklace(const std::string& id, const std::string& commands)
{
  LOG(FATAL) << "Not implemented yet.";
  return false;
}

/**@brief Add a generic necklace.
 * @param id the necklace ID.
 * @param commands the SVG path commands (including point coordinates).
 * @param kernel the kernel of the star-shaped necklace polygon.
 * @return whether the necklace was constructed correctly.
 */
bool NecklaceMapSvgVisitor::AddGenericNecklace(const std::string& id, const std::string& commands, const Point& kernel)
{
  LOG(FATAL) << "Not implemented yet.";
  return false;
}

/**@brief Add a necklace element based on an SVG path.
 * @param commands the SVG path commands (including point coordinates).
 * @param id the ID of the region.
 *
 * See @f Region::Region(const std::string& id) for details on this ID.
 *
 * Note that the ID does not have to unique. If a duplicate ID is encountered, the
 * polygon is added to the region with the same ID.
 * @param style the CSS style of the region polygon. Note that this style will be reused
 * for the output regions.
 * @return whether the region could be parsed correctly.
 */
bool NecklaceMapSvgVisitor::AddMapElement
(
  const std::string& id,
  const std::string& commands,
  const std::string& necklace_id,
  const std::string& style
)
{
  // Get the region with the given ID, or create a new one if it does not yet exist.
  const size_t n = id_to_region_index_.insert({id, elements_.size()}).first->second;
  if (n == elements_.size()) elements_.emplace_back(std::make_shared<necklace_map::MapElement>(id));
  CHECK_NOTNULL(elements_[n]);
  Region& region = elements_[n]->region;
  CHECK_EQ(id, region.id);

  // Interpret the commands as a region.
  SvgPolygonConverter converter(region.shape);
  SvgPathParser()(commands, converter);

  region.style = style;
  necklace_ids_.resize(n + 1);
  necklace_ids_[n] = necklace_id;

  if (strict_validity_)
    CHECK(region.IsValid()) << "Invalid region: " << region.id;
  else
    region.MakeValid();
  return true;
}

} // namespace detail
} // namespace geoviz
