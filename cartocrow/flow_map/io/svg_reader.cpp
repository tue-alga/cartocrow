/*
The Flow Map library implements the algorithmic geo-visualization
method by the same name, developed by Kevin Verbeek, Kevin Buchin,
and Bettina Speckmann at TU Eindhoven
(DOI: 10.1007/s00453-013-9867-z & 10.1109/TVCG.2011.202).
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 04-09-2020
*/

#include "svg_reader.h"

#include <fstream>

#include <glog/logging.h>

#include "cartocrow/flow_map/io/detail/svg_visitor.h"


namespace cartocrow
{
namespace flow_map
{

/**@class SvgReader
 * @brief A reader for SVG flow map input geometry.
 */

/**@brief Construct a reader for SVG flow map input geometry.
 */
SvgReader::SvgReader() {}

/**@brief Read flow map SVG input from a file.
 * @param filename the file to read.
 * @param context the collection in which to collect the context regions in the input.
 * @param places the collection in which to collect the places on the flow map (e.g. root and leaf nodes).
 * @param max_retries the maximum number of times to retry reading the file.
 * @return whether the read operation could be completed successfully.
 */
bool SvgReader::ReadFile
(
  const std::filesystem::path& filename,
  std::vector<cartocrow::Region>& context,
  std::vector<Place::Ptr>& places,
  int max_retries /*= 2*/
)
{
  std::string input;
  int retry = 0;
  do
  {
    try
    {
      std::fstream fin(filename);
      if (fin)
      {
        using Iterator = std::istreambuf_iterator<char>;
        input.assign(Iterator(fin), Iterator());
        break;
      }
    }
    catch (const std::exception& e)
    {
      LOG(ERROR) << e.what();
    }

    if (max_retries < retry++)
    {
      LOG(INFO) << "Failed to open flow map geometry file: " << filename;
      return false;
    }
  } while (true);

  return Parse(input, context, places);
}


/**@brief Parse flow map SVG input from a string.
 * @param input the string to parse.
 * @param context the collection in which to collect the context regions in the input.
 * @param places the collection in which to collect the places on the flow map (e.g. root and leaf nodes).
 * @return whether the string could be parsed successfully.
 */
bool SvgReader::Parse
(
  const std::string& input,
  std::vector<cartocrow::Region>& context,
  std::vector<Place::Ptr>& places
)
{
  tinyxml2::XMLDocument doc;
  tinyxml2::XMLError result = doc.Parse(input.data());
  //tinyxml2::XMLError result = doc.Parse(content_str); // This would be how to parse a string instead of a file using tinyxml2
  if (result != tinyxml2::XML_SUCCESS) return false;

  using Visitor = detail::SvgVisitor;
  Visitor visitor(context, places);
  doc.Accept(&visitor);

  LOG(INFO) << "Successfully parsed flow map geometry for " << places.size() << " place(s).";

  return true;
}

} // namespace flow_map
} // namespace cartocrow
