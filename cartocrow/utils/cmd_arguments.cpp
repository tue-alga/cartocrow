/*
Copyright (C) 2026  TU Eindhoven

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
*/

#include "cmd_arguments.h"

#include <iostream>
#include <algorithm>

namespace cartocrow::utils {

CommandLineArguments::CommandLineArguments(int argc, char* argv[]) {
	for (int c = 0; c < argc; ++c) {
		m_arguments.push_back(argv[c]);
	}
}

bool CommandLineArguments::has_any_arguments() const {
	return m_arguments.size() > 1; // NB: the first argument provided is just the program itself
}

bool CommandLineArguments::has_argument(const std::string val, const int params) const {
	int index = find_argument_index(val);
	return 0 <= index && index + params < m_arguments.size();
}

int CommandLineArguments::find_argument_index(const std::string val) const {
	int index = 0;
	while (index < m_arguments.size()) {
		if (m_arguments[index] == val) {
			return index;
		}
		index++;
	}
	return -1;
}

std::optional<std::string> CommandLineArguments::get_optional_string(const std::string val,
                                                                     const int offset) const {
	int index = find_argument_index(val);
	if (index < 0) {
		return std::nullopt;
	}
	return get_optional_string(index + offset);
}

std::optional<std::string> CommandLineArguments::get_optional_string(const int index) const {
	if (index < 0 || index >= m_arguments.size()) {
		return std::nullopt;
	}
	return m_arguments[index];
}

std::string CommandLineArguments::get_string(const std::string val, const std::string deft,
                                             const int offset) const {
	auto v = get_optional_string(val, offset);
	if (!v) {
		return deft;
	}
	return *v;
}

std::string CommandLineArguments::get_string(const int index, const std::string deft) const {
	auto v = get_optional_string(index);
	if (!v) {
		return deft;
	}
	return *v;
}

std::optional<int> CommandLineArguments::get_optional_integer(const std::string val,
                                                              const int offset) const {
	auto v = get_optional_string(val, offset);
	if (!v) {
		return std::nullopt;
	}
	return std::stoi(*v);
}

std::optional<int> CommandLineArguments::get_optional_integer(const int index) const {
	auto v = get_optional_string(index);
	if (!v) {
		return std::nullopt;
	}
	return std::stoi(*v);
}

int CommandLineArguments::get_integer(const std::string val, const int deft, const int offset) const {
	auto v = get_optional_string(val, offset);
	if (!v) {
		return deft;
	}
	return std::stoi(*v);
}

int CommandLineArguments::get_integer(const int index, const int deft) const {
	auto v = get_optional_string(index);
	if (!v) {
		return deft;
	}
	return std::stoi(*v);
}

std::optional<double> CommandLineArguments::get_optional_double(const std::string val,
                                                                const int offset) const {
	auto v = get_optional_string(val, offset);
	if (!v) {
		return std::nullopt;
	}
	return std::stod(*v);
}

std::optional<double> CommandLineArguments::get_optional_double(const int index) const {
	auto v = get_optional_string(index);
	if (!v) {
		return std::nullopt;
	}
	return std::stod(*v);
}

double CommandLineArguments::get_double(const std::string val, const double deft,
                                        const int offset) const {
	auto v = get_optional_string(val, offset);
	if (!v) {
		return deft;
	}
	return std::stod(*v);
}

double CommandLineArguments::get_double(const int index, const double deft) const {
	auto v = get_optional_string(index);
	if (!v) {
		return deft;
	}
	return std::stod(*v);
}

void CommandLineArguments::print_arguments(const std::string newline_prefix,
                                           const std::string newline_start,
                                           const std::string sep) const {
	bool first = true;
	for (std::string arg : m_arguments) {
		if (first) {
			first = false;
		} else {
			auto res = std::mismatch(newline_prefix.begin(), newline_prefix.end(), arg.begin());
			if (res.first == newline_prefix.end()) {
				std::cout << std::endl << newline_start;
			} else {
				std::cout << sep;
			}
		}
		std::cout << arg;
	}
	std::cout << std::endl;
}

} // namespace cartocrow::utils