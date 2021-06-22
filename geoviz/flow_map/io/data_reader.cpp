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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 06-10-2020
*/

#include "data_reader.h"

#include <cstring>
#include <exception>
#include <fstream>

#include <glog/logging.h>


namespace geoviz
{
namespace
{

constexpr const char* kMagicCharacters = "FlMp";

constexpr const char* kNameId = "id";

} // anonymous namespace

namespace flow_map
{

/**@class DataReader
 * @brief A reader for flow map values.
 */

/**@brief Construct a reader for flow map values.
 */
DataReader::DataReader()
  : detail::TableParser() {}

/**@brief Read a flow map data file.
 *
 * The table in the file must contain a string column called "id" (case sensitive) and a numeric column describing the incoming flows.
 *
 * See @f detail::TableParser::Parse(std::istream& in) for more info on the file format.
 * @param filename the name of the file.
 * @param value_name the name of the data column (case sensitive).
 * @param places the places on the flow map (e.g. root and leaf nodes) associated with the values.
 * @param index_root the index of the root node of the flow map.
 * @param max_retries the maximum number of times to retry reading after an error.
 * @return whether the element values could be read successfully.
 */
bool DataReader::ReadFile
(
  const std::string& filename,
  const std::string& value_name,
  std::vector<Place::Ptr>& places,
  size_t& index_root,
  int max_retries /*= 2*/
)
{
  std::fstream fin;
  int retry = 0;
  do
  {
    try
    {
      fin.open(filename);
      if (fin)
        break;
    }
    catch (const std::exception& e)
    {
      LOG(ERROR) << e.what();
    }

    if (max_retries < retry++)
    {
      LOG(INFO) << "Failed to open flow map data file: " << filename;
      return false;
    }
  } while (true);

  // Data files must start with four magic characters and the data file version.
  char magic[4] = {'\0', '\0', '\0', '\0'};
  fin.read(magic, 4);
  if (!fin || std::strcmp(magic, kMagicCharacters) != 0)
    return false;

  // Read the version.
  std::string version;
  fin >> version;
  if (!fin)
    return false;

  return Parse(fin, value_name, places, index_root, version);
}

/**@brief Parse a flow map data string.
 *
 * The string must be composed of string 'tokens' separated by whitespace characters.
 *
 * The first token must be an integer describing the number of data elements.
 * Note that if this number is 0, the stream is still checked for integrity.
 *
 * The second token must be the format token: a string describing the value types per element.
 * Allowed format tokens are 's' for strings, 'd' for double values, and 'i' for integer values.
 *
 * The following tokens give the name per value.
 *
 * The columns must include a string column called "id" (case sensitive) and a double column containing the incoming flows.
 *
 * The remainder of the tokens are the flow values, grouped per place and ordered as described in the format token.
 *
 * For example, a stream starting with "5 ssd ID name value " should be followed by five places with each two string values and one double value. The string columns are called "ID" and "name"; the double column is called "value".
 *
 * Tokens for string values may contain whitespace if the string starts and end with quotation marks (").
 * @param in the string to parse.
 * @param value_name the name of the data column (case sensitive).
 * @param places the places on the flow map (e.g. root and nodes) associated with the values.
 * @param index_root the index of the root node of the flow map.
 * @param version the data format version.
 * @return whether the element values could be read successfully.
 */
bool DataReader::Parse
(
  std::istream& in,
  const std::string& value_name,
  std::vector<Place::Ptr>& places,
  size_t& index_root,
  const std::string& version /*= "1.0"*/
)
{
  // Parse the data.
  if (!detail::TableParser::Parse(in))
    return false;

  // Find the ID and value columns and check that they are the correct types.
  using ColumnString = detail::ValueColumn<std::string>;
  using DataColumn = detail::DataColumn;
  const ColumnString* column_id = nullptr;
  const DataColumn* column_value = nullptr;

  for (const detail::TableParser::ColumnPtr& column : table_)
  {
    if (column->name == kNameId)
      column_id = dynamic_cast<const ColumnString*>(column.get());
    else if (column->name == value_name)
      column_value = column.get();
  }
  if (column_id == nullptr || column_value == nullptr || column_id->values.size() != column_value->size())
    return false;


  using ColumnDouble = detail::ValueColumn<double>;
  using ColumnInteger = detail::ValueColumn<int>;
  const ColumnDouble* column_double = dynamic_cast<const ColumnDouble*>(column_value);
  const ColumnInteger* column_int = dynamic_cast<const ColumnInteger*>(column_value);
  if (column_double == nullptr && column_int == nullptr)
    return false;

  // Create a lookup table for the elements.
  // Additionally, determine the root node.
  using LookupTable = std::unordered_map<std::string, size_t>;
  LookupTable id_to_element_index;
  index_root = places.size();
  for (size_t index_place = 0; index_place < places.size(); ++index_place)
  {
    Place::Ptr& place = places[index_place];

    const size_t next_index = id_to_element_index.size();
    const size_t n = id_to_element_index.insert({place->id, next_index}).first->second;
    CHECK_EQ(next_index, n);

    // Set the value to 0 in case the elements are reused.
    place->flow_in = 0;

    if (place->id == value_name)
      index_root = index_place;
  }
  CHECK(index_root != places.size());

  // Add the values to their associated element.
  for (size_t v = 0; v < column_id->values.size(); ++v)
  {
    const std::string& id = column_id->values[v];

    // Get the region with the given ID, or create a new one if it does not yet exist.
    std::pair<LookupTable::iterator, bool> result = id_to_element_index.emplace(id, places.size());
    const size_t n = result.first->second;
    if (n == places.size())
      places.push_back(std::make_shared<Place>(id, PolarPoint()));
    Place::Ptr& place = places[n];
    CHECK_EQ(id, place->id);

    if (column_double == nullptr)
      place->flow_in = column_int->values[v];
    else
      place->flow_in = column_double->values[v];
  }

  LOG(INFO) << "Successfully parsed flow map data for " << places.size() << " place(s).";
  return true;
}

} // namespace flow_map
} // namespace geoviz
