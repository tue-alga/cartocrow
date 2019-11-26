/*
File reader for SVG necklace map input.
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

#ifndef GEOVIZ_SVG_NECKLACE_MAP_READER_H
#define GEOVIZ_SVG_NECKLACE_MAP_READER_H

#include <string>
#include <unordered_map>
#include <vector>

#include "console/necklace_map/detail/svg_necklace_map_visitor.h"
#include "geoviz/necklace_map/necklace.h"
#include "geoviz/necklace_map/region.h"


namespace geoviz
{

namespace gnm = geoviz::necklace_map;

class SvgNecklaceMapReader
{
 public:
  using NecklaceType = gnm::NecklaceType;
  using Region = gnm::Region;

 public:
  SvgNecklaceMapReader
  (
    std::vector<Region>& regions,
    std::shared_ptr<NecklaceType>& necklace,
    std::unordered_map<std::string, size_t>& region_index_by_id
  );

  bool read(const std::string& filename);

 private:
  geoviz::detail::SvgNecklaceMapVisitor visitor_;
};

} // namespace geoviz

#endif //GEOVIZ_SVG_NECKLACE_MAP_READER_H
