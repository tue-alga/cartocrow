/*
The Necklace Map library implements the algorithmic
geo-visualization method by the same name, developed by
Bettina Speckmann and Kevin Verbeek at TU Eindhoven
(DOI: 10.1109/TVCG.2010.180 & 10.1142/S021819591550003X).
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 26-11-2019
*/

#ifndef CARTOCROW_NECKLACE_MAP_IO_DETAIL_SVG_VISITOR_H
#define CARTOCROW_NECKLACE_MAP_IO_DETAIL_SVG_VISITOR_H

#include <memory>
#include <unordered_map>
#include <vector>

#include <tinyxml2.h>

#include "cartocrow/core/core_types.h"
#include "cartocrow/core/detail/svg_visitor.h"
#include "cartocrow/necklace_map/bead.h"
#include "cartocrow/necklace_map/map_element.h"
#include "cartocrow/necklace_map/necklace.h"

namespace cartocrow {
namespace necklace_map {
namespace detail {

class SvgVisitor : public cartocrow::detail::SvgVisitor {
  private:
	using MapElement = necklace_map::MapElement;
	using Bead = necklace_map::Bead;
	using Necklace = necklace_map::Necklace;
	using LookupTable = std::unordered_map<std::string, size_t>;

  public:
	SvgVisitor(std::vector<necklace_map::MapElement::Ptr>& elements,
	           std::vector<necklace_map::Necklace::Ptr>& necklaces, Number& scale_factor,
	           const bool strict_validity = true);

  private:
	bool VisitExit(const tinyxml2::XMLElement& element);

	void VisitSvg(const tinyxml2::XMLAttribute* attributes);

	bool VisitCircle(const Point& center, const Number& radius,
	                 const tinyxml2::XMLAttribute* attributes);

	bool VisitPath(const std::string& commands, const tinyxml2::XMLAttribute* attributes);

	bool FinalizeSvg();

	bool AddCircleNecklace(const std::string& necklace_id, const Point& center, const Number& radius);

	bool AddGenericNecklace(const std::string& necklace_id, const std::string& commands,
	                        const Point& kernel);

	bool AddMapElement(const std::string& commands, const std::string& angle_rad,
	                   const std::string& feasible, const std::string& region_id,
	                   const std::string& necklace_id, const std::string& style);

	std::vector<MapElement::Ptr>& elements_;
	std::vector<std::string> necklace_ids_;
	std::vector<Necklace::Ptr>& necklaces_;

	LookupTable id_to_region_index_;
	LookupTable id_to_necklace_index_;

	Number& scale_factor_;

	bool strict_validity_;
}; // class SvgVisitor

} // namespace detail
} // namespace necklace_map
} // namespace cartocrow

#endif //CARTOCROW_NECKLACE_MAP_IO_DETAIL_SVG_VISITOR_H
