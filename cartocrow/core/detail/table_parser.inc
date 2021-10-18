/*
The CartoCrow library implements algorithmic geo-visualization methods,
developed at TU Eindhoven.
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

#ifndef CARTOCROW_CORE_DETAIL_TABLE_PARSER_INC
#define CARTOCROW_CORE_DETAIL_TABLE_PARSER_INC

#include <glog/logging.h>

namespace cartocrow {
namespace detail {

/**@struct ValueColumn
 * @brief A type-specific implementation of a DataColumn.
 * @tparam T_ the value type.
 *
 * Implementations for the integer, double, and std::string types are provided.
 */

/**@brief Construct a named column with a specific value type.
 * @tparam T_ the value type.
 * @param name the name of the column.
 * @param size the number of values.
 *
 * Note that all columns in a table must have matching sizes.
 */
template <typename T_>
inline ValueColumn<T_>::ValueColumn(const std::string& name, const size_t size)
    : DataColumn(name), values() {
	values.reserve(size);
}
#ifndef DOXYGEN_RUNNING
template <> inline void ValueColumn<int>::push_back(const std::string& value) {
	try {
		values.push_back(std::stoi(value));
	} catch (...) {
		LOG(FATAL) << "Failed conversion to integer: " << value;
	}
}

template <> inline void ValueColumn<double>::push_back(const std::string& value) {
	try {
		values.push_back(std::stod(value));
	} catch (...) {
		LOG(FATAL) << "Failed conversion to double: " << value;
	}
}

template <> inline void ValueColumn<std::string>::push_back(const std::string& value) {
	values.push_back(value);
}
#endif //DOXYGEN_RUNNING
} // namespace detail
} // namespace cartocrow

#endif //CARTOCROW_CORE_DETAIL_TABLE_PARSER_INC
