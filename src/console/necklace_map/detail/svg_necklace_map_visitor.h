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

#ifndef CONSOLE_NECKLACE_MAP_DETAIL_SVG_NECKLACE_MAP_VISITOR_H
#define CONSOLE_NECKLACE_MAP_DETAIL_SVG_NECKLACE_MAP_VISITOR_H

#include <memory>
#include <unordered_map>
#include <vector>

#include <tinyxml2.h>

#include "console/common/detail/svg_visitor.h"
#include "geoviz/common/core_types.h"
#include "geoviz/necklace_map/necklace.h"
#include "geoviz/necklace_map/region.h"


namespace geoviz
{
namespace detail
{

namespace gnm = geoviz::necklace_map;

class SvgNecklaceMapVisitor : public geoviz::detail::SvgVisitor
{
 public:
  using NecklaceType = gnm::NecklaceType;
  using Region = gnm::Region;

 public:
  SvgNecklaceMapVisitor
  (
    std::vector<Region>& regions,
    std::shared_ptr<NecklaceType>& necklace,
    std::unordered_map<std::string, size_t>& region_index_by_id
  );

 private:
  bool visitCircle
  (
    const Point& center,
    const Number& radius,
    const tinyxml2::XMLAttribute* attributes
  );

  bool visitPath
  (
    const std::string& commands,
    const tinyxml2::XMLAttribute* attributes
  );

  bool SetCircleNecklace(const Point& center, const Number& radius);
  bool SetArcNecklace(const std::string& commands);
  bool SetGenericNecklace(const std::string& commands, const Point& kernel);

  bool AddRegion(const std::string& commands, const std::string& id, const std::string& style);

  std::vector<Region>& regions_;
  std::shared_ptr<NecklaceType>& necklace_;

  std::unordered_map<std::string, size_t>& region_index_by_id_;
}; // class SvgNecklaceMapVisitor

} // namespace detail
} // namespace geoviz

#endif //CONSOLE_NECKLACE_MAP_DETAIL_SVG_NECKLACE_MAP_VISITOR_H
