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

#ifndef CARTOCROW_FLOW_MAP_IO_DATA_READER_H
#define CARTOCROW_FLOW_MAP_IO_DATA_READER_H

#include <filesystem>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#include "cartocrow/core/detail/table_parser.h"
#include "cartocrow/flow_map/place.h"

namespace cartocrow {
namespace flow_map {

class DataReader : public cartocrow::detail::TableParser {
  public:
	DataReader();

	bool readFile(const std::filesystem::path& filename, const std::string& value_name,
	              std::vector<std::shared_ptr<Place>>& places, size_t& index_root,
	              int max_retries = 2);

	bool parse(std::istream& in, const std::string& value_name,
	           std::vector<std::shared_ptr<Place>>& places, size_t& index_root,
	           const std::string& version = "1.0");
}; // class DataReader

} // namespace flow_map
} // namespace cartocrow

#endif //CARTOCROW_FLOW_MAP_IO_DATA_READER_H
