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

#include "table_parser.h"

#include <algorithm>
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

/**@fn std::string DataColumn::name;
 * @brief The name of the column.
 */


/**@class TableParser
 * @brief A parser for tabular data input.
 */

/**@brief Construct a parser for tabular data input.
 */
TableParser::TableParser() : table_() {}

/**@brief Parse an input stream.
 * The stream must be composed of string 'tokens' separated by whitespace characters.
 *
 * The first token must be an integer describing the number of data elements.
 * Note that if this number is 0, the stream is still checked for integrity.
 *
 * The second token must be the format token: a string describing the value types per element.
 * Allowed format tokens are 's' for strings, 'd' for double values, and 'i' for integer values.
 *
 * The following tokens give the name per value.
 *
 * The remainder of the tokens are the element values, grouped per element and ordered as described in the format token.
 *
 * For example, a stream starting with "5 ssd ID name value " should be followed by five elements with each two string values and one double value. The string columns are called "ID" and "name"; the double column is called "value".
 *
 * Tokens for string values may contain whitespace if the string starts and end with quotation marks (").
 * @param[in/out] in the stream to parse.
 * @return whether the table could be parsed successfully.
 */
bool TableParser::Parse(std::istream& in)
{
  if (!in)
    return false;

  // Read the number of data elements.
  size_t num_elements;
  in >> num_elements;
  if (!in)
    return false;

  // Read the element format.
  std::string format;
  in >> format;
  if (!in)
    return false;
  CHECK(!format.empty());

  std::transform(format.begin(), format.end(), format.begin(), [](unsigned char c) { return std::tolower(c); });

  // Read the name per value.
  table_.clear();
  for(const char& type : format)
  {
    std::string name;
    in >> name;
    if (!in)
      return false;
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
    for (Table::iterator column_iter = table_.begin(); column_iter != table_.end();)
    {
      std::string value;
      in >> value;
      if (!in)
        return false;

      while (!value.empty())
      {
        size_t pos = value.find("\"");
        if (pos != std::string::npos)
        {
          if (0 < pos)
          {
            const std::string before = value.substr(0, pos);
            (*column_iter++)->push_back(before);
            value = value.substr(pos);
          }

          // Parse quoted string.
          std::stringstream stream;
          stream << value;
          do
          {
            in >> value;
            if (!in)
              return false;
            pos = value.find("\"");
            if (pos == std::string::npos)
              stream << " " << value;
            else
              stream << " " << value.substr(0, pos+1);
          } while (pos == std::string::npos);
          const std::string quote = stream.str();
          (*column_iter++)->push_back(quote);

          if (pos + 1 < value.length())
            value = value.substr(pos+1);
          else
            value = "";
        }
        else
        {
          (*column_iter++)->push_back(value);
          value = "";
        }
      }
    }
  }

  return true;
}

} // namespace detail
} // namespace geoviz
