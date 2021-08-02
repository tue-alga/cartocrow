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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 03-12-2019
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

constexpr const char* kMagicCharacters = "NcMp";

constexpr const char* kNameId = "id";

} // anonymous namespace

namespace necklace_map
{

/**@class DataReader
 * @brief A reader for necklace map values.
 */

/**@brief Construct a reader for necklace map values.
 */
DataReader::DataReader()
  : detail::TableParser() {}

/**@brief Read a necklace map data file.
 *
 * The table in the file must contain a string column called "id" (case sensitive) and a double column containing the necklace element values.
 *
 * See @f detail::TableParser::Parse(std::istream& in) for more info on the file format.
 * @param filename the name of the file.
 * @param value_name the name of the data column (case sensitive).
 * @param elements the necklace map elements associated with the values.
 * @param max_retries the maximum number of times to retry reading after an error.
 * @return whether the element values could be read successfully.
 */
bool DataReader::ReadFile
(
  const std::filesystem::path& filename,
  const std::string& value_name,
  std::vector<MapElement::Ptr>& elements,
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
      LOG(INFO) << "Failed to open necklace map data file: " << filename;
      return false;
    }
  } while (true);

  // Data files must start with four magic characters and the data file version.
  char magic[5] = {'\0', '\0', '\0', '\0', '\0'};
  fin.read(magic, 4);
  if (!fin || std::strcmp(magic, kMagicCharacters) != 0) {
    LOG(INFO) << "Data file did not start with magic characters: " << filename;
    return false;
  }

  // Read the version.
  std::string version;
  fin >> version;
  if (!fin) {
      LOG(INFO) << "Data file did not start with a version: " << filename;
      return false;
  }

  return Parse(fin, value_name, elements, version);
}

/**@brief Parse a necklace map data string.
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
 * The columns must include a string column called "id" (case sensitive) and a double column containing the necklace element values.
 *
 * The remainder of the tokens are the element values, grouped per element and ordered as described in the format token.
 *
 * For example, a stream starting with "5 ssd ID name value " should be followed by five elements with each two string values and one double value. The string columns are called "ID" and "name"; the double column is called "value".
 *
 * Tokens for string values may contain whitespace if the string starts and end with quotation marks (").
 * @param in the string to parse.
 * @param value_name the name of the data column (case sensitive).
 * @param elements the necklace map elements associated with the values.
 * @param version the data format version.
 * @return whether the element values could be read successfully.
 */
bool DataReader::Parse
(
  std::istream& in,
  const std::string& value_name,
  std::vector<MapElement::Ptr>& elements,
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
  using LookupTable = std::unordered_map<std::string, size_t>;
  LookupTable id_to_element_index;
  for (necklace_map::MapElement::Ptr& element : elements)
  {
    CHECK_NOTNULL(element);
    const size_t next_index = id_to_element_index.size();
    const size_t n = id_to_element_index.insert({element->region.id, next_index}).first->second;
    CHECK_EQ(next_index, n);

    // Set the value to 0 in case the elements are reused.
    element->value = 0;
  }

  // Add the values to their associated element.
  for (size_t v = 0; v < column_id->values.size(); ++v)
  {
    const std::string& id = column_id->values[v];

    // Get the region with the given ID, or create a new one if it does not yet exist.
    std::pair<LookupTable::iterator, bool> result = id_to_element_index.emplace(id, elements.size());
    const size_t e = result.first->second;
    if (e == elements.size())
      elements.push_back(std::make_shared<necklace_map::MapElement>(id));
    necklace_map::MapElement::Ptr& element = elements[e];
    CHECK_NOTNULL(element);
    CHECK_EQ(id, element->region.id);

    if (column_double == nullptr)
      element->value = column_int->values[v];
    else
      element->value = column_double->values[v];
  }

  LOG(INFO) << "Successfully parsed necklace map data for " << elements.size() << " element(s).";
  return true;
}

} // namespace necklace_map
} // namespace geoviz
