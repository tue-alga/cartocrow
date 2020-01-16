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

#ifndef CONSOLE_NECKLACE_MAP_DETAIL_SVG_VISITOR_H
#define CONSOLE_NECKLACE_MAP_DETAIL_SVG_VISITOR_H

#include <memory>
#include <unordered_map>
#include <vector>

#include <tinyxml2.h>

#include "console/common/detail/svg_visitor.h"
#include "geoviz/common/core_types.h"
#include "geoviz/necklace_map/necklace.h"
#include "geoviz/necklace_map/necklace_glyph.h"
#include "geoviz/necklace_map/map_element.h"


namespace geoviz
{
namespace detail
{

class NecklaceMapSvgVisitor : public SvgVisitor
{
 private:
  using MapElement = necklace_map::MapElement;
  using NecklaceGlyph = necklace_map::NecklaceGlyph;
  using Necklace = necklace_map::Necklace;
  using LookupTable = std::unordered_map<std::string, size_t>;

 public:
  NecklaceMapSvgVisitor
  (
    std::vector<necklace_map::MapElement::Ptr>& elements,
    std::vector<necklace_map::Necklace::Ptr>& necklaces,
    const bool strict_validity = true
  );

 private:
  bool VisitExit(const tinyxml2::XMLElement& element);

  bool VisitCircle(const Point& center, const Number& radius, const tinyxml2::XMLAttribute* attributes);

  bool VisitPath(const std::string& commands, const tinyxml2::XMLAttribute* attributes);

  bool FinalizeSvg();

  bool AddCircleNecklace(const std::string& id, const Point& center, const Number& radius);
  bool AddArcNecklace(const std::string& id, const std::string& commands);
  bool AddGenericNecklace(const std::string& id, const std::string& commands, const Point& kernel);

  bool AddMapElement
  (
    const std::string& id,
    const std::string& commands,
    const std::string& necklace_id,
    const std::string& style
  );

  std::vector<MapElement::Ptr>& elements_;
  std::vector<std::string> necklace_ids_;
  std::vector<Necklace::Ptr>& necklaces_;

  LookupTable id_to_region_index_;
  LookupTable id_to_necklace_index_;

  bool strict_validity_;
}; // class NecklaceMapSvgVisitor

} // namespace detail
} // namespace geoviz

#endif //CONSOLE_NECKLACE_MAP_DETAIL_SVG_VISITOR_H
