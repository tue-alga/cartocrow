/*
The Flow Map library implements the algorithmic geo-visualization
method by the same name, developed by Kevin Verbeek, Kevin Buchin,
and Bettina Speckmann at TU Eindhoven
(DOI: 10.1007/s00453-013-9867-z & 10.1109/TVCG.2011.202).
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 04-09-2020
*/

#ifndef GEOVIZ_FLOW_MAP_IO_DETAIL_SVG_VISITOR_H
#define GEOVIZ_FLOW_MAP_IO_DETAIL_SVG_VISITOR_H

#include <memory>
#include <unordered_map>
#include <vector>

#include <tinyxml2.h>

#include "geoviz/common/core_types.h"
#include "geoviz/common/region.h"
#include "geoviz/common/detail/svg_visitor.h"
#include "geoviz/flow_map/node.h"


namespace geoviz
{
namespace flow_map
{
namespace detail
{

class SvgVisitor : public geoviz::detail::SvgVisitor
{
 private:
  using Region = geoviz::Region;
  using Node = flow_map::Node;
  using LookupTable = std::unordered_map<std::string, size_t>;

 public:
  SvgVisitor
  (
    std::vector<geoviz::Region>& context,
    std::vector<geoviz::flow_map::Node>& nodes,
    const bool strict_validity = true
  );

 private:
  bool VisitExit(const tinyxml2::XMLElement& element);

  void VisitSvg(const tinyxml2::XMLAttribute* attributes);

  bool VisitCircle(const Point& center, const Number& radius, const tinyxml2::XMLAttribute* attributes);

  bool VisitPath(const std::string& commands, const tinyxml2::XMLAttribute* attributes);

  bool FinalizeSvg();

  bool AddNode(const std::string& node_id, const Point& point);

  bool AddNode(const std::string& node_id, const std::string& commands);

  bool AddRegion(const std::string& commands, const std::string& style);

  std::vector<geoviz::Region>& context_;
  std::vector<geoviz::flow_map::Node>& nodes_;

  LookupTable id_to_node_index_;

  bool strict_validity_;
}; // class SvgVisitor

} // namespace detail
} // namespace flow_map
} // namespace geoviz

#endif //GEOVIZ_FLOW_MAP_IO_DETAIL_SVG_VISITOR_H
