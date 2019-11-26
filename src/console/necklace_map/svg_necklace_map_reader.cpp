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

#include "svg_necklace_map_reader.h"

namespace geoviz
{

/**@class SvgNecklaceMapReader
 * @brief A file reader for SVG necklace map input geometry.
 */

/**@brief Main constructor.
 * @param regions the collection in which to collect the regions in the input.
 * @param necklace where to place the necklace.
 * @param region_index_by_id a map from region ID to index in the regions argument.
 */
SvgNecklaceMapReader::SvgNecklaceMapReader
(
  std::vector<Region>& regions,
  std::shared_ptr<NecklaceType>& necklace,
  std::unordered_map<std::string, size_t>& region_index_by_id
)
  : visitor_(regions, necklace, region_index_by_id) {}

/**@brief Read necklace map SVG input from a file.
 * @param filename the file to read.
 * @return whether the read operation could be completed successfully.
 */
bool SvgNecklaceMapReader::read(const std::string& filename)
{
  tinyxml2::XMLDocument doc;
  tinyxml2::XMLError result = doc.LoadFile(filename.c_str());
  //tinyxml2::XMLError result = doc.Parse(content_str); // This would be how to parse a string instead of a file using tinyxml2
  if (result != tinyxml2::XML_SUCCESS) return false;

  doc.Accept(&visitor_);
  return true;
}

} // namespace geoviz
