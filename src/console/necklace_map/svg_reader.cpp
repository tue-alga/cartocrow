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

#include "svg_reader.h"

DEFINE_bool
(
  strict_validity,
  true,
  "Whether failures in validity should generate a breaking error. Otherwise, some faults"
  " may be corrected silently. Note that this may break some assumptions on input-output"
  " data similarity."
);


namespace geoviz
{

/**@class SvgReader
 * @brief A file reader for SVG necklace map input geometry.
 */

/**@brief Construct a file reader for SVG necklace map input geometry.
 * @param regions the collection in which to collect the regions in the input.
 * @param necklace where to place the necklace.
 */
SvgReader::SvgReader(std::vector<MapElement>& elements, std::vector<NecklaceTypePtr>& necklaces)
  : visitor_(elements, necklaces, FLAGS_strict_validity) {}

/**@brief Read necklace map SVG input from a file.
 * @param filename the file to read.
 * @return whether the read operation could be completed successfully.
 */
bool SvgReader::Read(const std::string& filename)
{
  tinyxml2::XMLDocument doc;
  tinyxml2::XMLError result = doc.LoadFile(filename.c_str());
  //tinyxml2::XMLError result = doc.Parse(content_str); // This would be how to parse a string instead of a file using tinyxml2
  if (result != tinyxml2::XML_SUCCESS) return false;

  doc.Accept(&visitor_);
  return true;
}

} // namespace geoviz
