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

#ifndef CONSOLE_NECKLACE_MAP_SVG_READER_H
#define CONSOLE_NECKLACE_MAP_SVG_READER_H

#include <string>
#include <unordered_map>
#include <vector>

#include <gflags/gflags.h>

#include "geoviz/common/region.h"
#include "console/necklace_map/detail/svg_visitor.h"
#include "geoviz/necklace_map/necklace.h"

DECLARE_bool(strict_validity);


namespace geoviz
{

class SvgReader
{
  using Visitor = detail::NecklaceMapSvgVisitor;
 public:
  using MapElement = Visitor::MapElement;
  using NecklaceTypePtr = Visitor::NecklaceTypePtr;

 public:
  SvgReader(std::vector<MapElement>& elements, std::vector<NecklaceTypePtr>& necklaces);

  bool Read(const std::string& filename);

 private:
  Visitor visitor_;
}; // class SvgReader

} // namespace geoviz

#endif //CONSOLE_NECKLACE_MAP_SVG_READER_H
