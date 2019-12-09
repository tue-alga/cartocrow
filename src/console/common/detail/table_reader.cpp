/*
The GeoViz console applications implement algorithmic geo-visualization
methods, developed at TU Eindhoven.
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 03-12-2019
*/

#include "table_reader.h"

#include <algorithm>
#include <fstream>
#include <string>

#include <glog/logging.h>


namespace geoviz
{
namespace detail
{
namespace
{

constexpr const char kCharInteger = 'i';
constexpr const char kCharDouble = 'd';
constexpr const char kCharString = 's';

} // anonymous namespace

/**@struct DataColumn
 * @brief An interface for a named column in a table.
 */

/**@brief Construct a named column.
 * @param name the name of the column.
 */
DataColumn::DataColumn(const std::string& name) : name(name) {}

/**@fn virtual void DataColumn::push_back(const std::string& value) = 0
 * @brief Parse a string for a value to insert at the end of the column.
 * @param value the value as string.
 */


/**@class TableReader
 * @brief A file reader for tabular data input.
 */

/**@brief Construct a file reader for tabular data input.
 */
TableReader::TableReader() : table_() {}

/**@brief Read an input file.
 *
 * The file must be a plain text file with 'tokens' separated by whitespace characters.
 *
 * The first token must be an integer describing the number of data elements.
 * Note that if this number is 0, the file is still checked for integrity.
 *
 * The second token must be the format token: a string describing the value types per element.
 * Allowed format tokens are 's' for strings, 'd' for double values, and 'i' for integer values.
 *
 * The following tokens give the name per value.
 *
 * The remainder of the tokens are the element values, grouped per element and ordered as described in the format token.
 *
 * For example, a file starting with "5 ssd ID name value " will contain five elements with a two string values and one double value. The string columns are called "ID" and "name"; the double column is called "value".
 * @param filename the name of the file.
 * @return whether the table could be read successfully.
 */
bool TableReader::Read(const std::string& filename)
{
  std::fstream fin(filename);
  if (!fin) return false;

  // Read the number of data elements.
  size_t num_elements;
  fin >> num_elements;
  if (!fin) return false;

  // Read the element format.
  std::string format;
  fin >> format;
  if (!fin) return false;
  CHECK(!format.empty());

  std::transform(format.begin(), format.end(), format.begin(), [](unsigned char c) { return std::tolower(c); });

  // Read the name per value.
  table_.clear();
  for(const char& type : format)
  {
    std::string name;
    fin >> name;
    if (!fin) return false;
    switch (type)
    {
      case kCharInteger:
        table_.emplace_back(new ValueColumn<int>(name, num_elements));
        break;
      case kCharDouble:
        table_.emplace_back(new ValueColumn<double>(name, num_elements));
        break;
      case kCharString:
        table_.emplace_back(new ValueColumn<std::string>(name, num_elements));
        break;
      default:
        LOG(FATAL) << "Unknown value type: " << type;
    }
  }

  // Read the element values.
  for (size_t e = 0; e < num_elements; ++e)
  {
    for (ColumnPtr& ptr : table_)
    {
      std::string value;
      fin >> value;
      if (!fin) return false;
      ptr->push_back(value);
    }
  }

  // The file should not contain more tokens.
  if (fin.eof()) return true;
  std::string next;
  fin >> next;
  return !fin;
}

} // namespace detail
} // namespace geoviz
