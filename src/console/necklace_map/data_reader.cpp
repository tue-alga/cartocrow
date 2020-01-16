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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 03-12-2019
*/

#include "data_reader.h"

#include <glog/logging.h>


namespace geoviz
{
namespace
{

constexpr const char* kNameId = "id";

} // anonymous namespace

/**@class DataReader
 * @brief A file reader for necklace map values.
 */

/**@brief Construct a file reader for necklace map values.
 * @param elements the necklace map elements associated with the values.
 */
DataReader::DataReader(std::vector<necklace_map::MapElement::Ptr>& elements)
  : detail::TableReader(), elements_(elements)
{
  // Add the elements to the lookup table, while checking for duplicates.
  for (const necklace_map::MapElement::Ptr& element : elements_)
  {
    CHECK_NOTNULL(element);
    const size_t next_index = id_to_element_index_.size();
    const size_t n = id_to_element_index_.insert({element->region.id, next_index}).first->second;
    CHECK_EQ(next_index, n);
  }
}

/**@brief Read a necklace map value file.
 *
 * The table in the file must contain a string collumn called "ID" and a double column containing the necklace element values.
 *
 * See @f detail::TableReader::read(const std::string& filename) for more info on the file format.
 * @param filename the name of the file.
 * @param value_name the name of the data column.
 * @return whether the table could be read successfully.
 */
bool DataReader::Read(const std::string& filename, const std::string& value_name)
{
  CHECK(detail::TableReader::Read(filename));

  // Find the ID and value columns and check that they are the correct types.
  using ColumnDouble = detail::ValueColumn<double>;
  using ColumnString = detail::ValueColumn<std::string>;
  const ColumnString* column_id;
  const ColumnDouble* column_value;

  for (const detail::TableReader::ColumnPtr& column : table_)
  {
    std::string lower_name = column->name;
    std::transform
    (
      lower_name.begin(),
      lower_name.end(),
      lower_name.begin(),
      [](unsigned char c) { return std::tolower(c); }
    );

    if (lower_name == kNameId)
      column_id = dynamic_cast<const ColumnString*>(column.get());
    else if (lower_name == value_name)
      column_value = dynamic_cast<const ColumnDouble*>(column.get());
  }
  CHECK_NOTNULL(column_id);
  CHECK_NOTNULL(column_value);
  CHECK_EQ(column_id->values.size(), column_value->values.size());

  // Add the values to their associated element.
  for (size_t v = 0; v < column_id->values.size(); ++v)
  {
    const std::string& id = column_id->values[v];

    // Get the region with the given ID, or create a new one if it does not yet exist.
    std::pair<LookupTable::iterator, bool> result = id_to_element_index_.emplace(id, elements_.size());
    const size_t e = result.first->second;
    if (e == elements_.size())
      elements_.push_back(std::make_shared<necklace_map::MapElement>(id));
    necklace_map::MapElement::Ptr& element = elements_[e];
    CHECK_NOTNULL(element);
    CHECK_EQ(id, element->region.id);

    element->value = column_value->values[v];
  }

  return true;
}

} // namespace geoviz
