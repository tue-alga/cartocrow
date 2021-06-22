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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 10-10-2019
*/

#ifndef CONSOLE_COMMON_UTILS_CLA_H
#define CONSOLE_COMMON_UTILS_CLA_H

#include <string>
#include <vector>


//@param interactive whether the executable accepts user input while running.
std::string CopyrightNotice(const bool interactive = false);

std::string UsageMessage
(
  const std::string& executable_name,
  const std::string& description,
  const std::vector<std::string>& sample_arguments = {}
);

void InitApplication
(
  int argc,
  char **argv,
  const std::string& description,
  const std::vector<std::string>& sample_arguments = {}
);

#endif //CONSOLE_COMMON_UTILS_CLA_H
