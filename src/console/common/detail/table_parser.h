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

#ifndef CONSOLE_COMMON_DETAIL_TABLE_PARSER_H
#define CONSOLE_COMMON_DETAIL_TABLE_PARSER_H

#include <iostream>
#include <memory>
#include <vector>


namespace geoviz
{
namespace detail
{

struct DataColumn
{
  DataColumn(const std::string& name);
  virtual void push_back(const std::string& value) = 0;
  std::string name;
}; // struct DataColumn

template<typename T_>
struct ValueColumn : public DataColumn
{
  ValueColumn(const std::string& name, const size_t size);
  void push_back(const std::string& value) override;
  std::vector<T_> values;
}; // struct ValueColumn


class TableParser
{
 public:
  TableParser();

  bool Parse(std::istream& in);

 protected:
  using ColumnPtr = std::unique_ptr<DataColumn>;
  using Table = std::vector<ColumnPtr>;
  Table table_;
}; // class TableParser

} // namespace detail
} // namespace geoviz

#include "table_parser.inc"

#endif //CONSOLE_COMMON_DETAIL_TABLE_PARSER_H
