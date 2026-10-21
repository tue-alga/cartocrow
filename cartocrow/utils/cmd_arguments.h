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
*/                                                                                                \

#pragma once

#include <vector>
#include <string>
#include <optional>

namespace cartocrow::utils {

class CommandLineArguments {
  private:
	std::vector<std::string> m_arguments;

  public:
	CommandLineArguments(int argc, char* argv[]);

	// checks whether there are arguments provided
	bool has_any_arguments() const;

	// checks whether the given string is one of the arguments, and whether there are the given number of parameters afterwards
	bool has_argument(const std::string val, const int params = 0) const;

	// finds the index of the given argument; will be -1 if the index was not found
	int find_argument_index(const std::string val) const;

	std::optional<std::string> get_optional_string(const std::string val, const int offset = 1) const;
	std::optional<std::string> get_optional_string(const int index) const;
	std::string get_string(const std::string val, const std::string deft = "",
	                       const int offset = 1) const;
	std::string get_string(const int index, const std::string deft = "") const;

	std::optional<int> get_optional_integer(const std::string val, const int offset = 1) const;
	std::optional<int> get_optional_integer(const int index) const;
	int get_integer(const std::string val, const int deft = 0, const int offset = 1) const;
	int get_integer(const int index, const int deft = 0) const;

	std::optional<double> get_optional_double(const std::string val, const int offset = 1) const;
	std::optional<double> get_optional_double(const int index) const;
	double get_double(const std::string val, const double deft = 0, const int offset = 1) const;
	double get_double(const int index, const double deft = 0) const;

	void print_arguments(const std::string newline_prefix = "-",
	                     const std::string newline_start = "  ", const std::string sep = " ") const;
};

} // namespace cartocrow::utils