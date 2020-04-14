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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 14-04-2020
*/

#ifndef CONSOLE_COMMON_UTILS_FILESYSTEM_H
#define CONSOLE_COMMON_UTILS_FILESYSTEM_H

// Note that GCC 7.4.0 implements the C++17 filesystem specification as experimental.
// This experimental implementation requires linking the "stdc++fs" static library.
// Note that this choice for compiler and version is made for convenience:
// it is currently shipped with my operating system.
// Other compilers or later versions may fully implement the C++17 standard,
// in which case this namespace will need to be updated as well.
#include <experimental/filesystem>

namespace filesystem = std::experimental::filesystem;

#endif //CONSOLE_COMMON_UTILS_FILESYSTEM_H
