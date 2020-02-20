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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 10-02-2020
*/

#include "utils_flags.h"

// Note that GCC 7.4.0 implements the C++17 filesystem specification as experimental.
// This experimental implementation requires linking the "stdc++fs" static library.
// Note that this choice for compiler and version is made for convenience:
// it is currently shipped with my operating system.
// Other compilers or later versions may fully implement the C++17 standard,
// in which case this namespace will need to be updated as well.
#include <experimental/filesystem>


namespace validate
{

namespace filesystem = std::experimental::filesystem;

bool IsFile(const std::string& value)
{
  return !IsDirectory(value);
}

bool IsDirectory(const std::string& value)
{
  try
  {
    return filesystem::is_directory(value);
  }
  catch (const std::exception& e)
  {
    LOG(INFO) << e.what();
    return false;
  }
}

bool ExistsFile(const std::string& value)
{
  try
  {
    return filesystem::is_regular_file(value) && ExistsPath(value);
  }
  catch (const std::exception& e)
  {
    LOG(INFO) << e.what();
    return false;
  }
}

bool ExistsDirectory(const std::string& value)
{
  try
  {
    return IsDirectory(value) && ExistsPath(value);
  }
  catch (const std::exception& e)
  {
    LOG(INFO) << e.what();
    return false;
  }
}

bool ExistsPath(const std::string& value)
{
  try
  {
    return filesystem::exists(value);
  }
  catch (const std::exception& e)
  {
    LOG(INFO) << e.what();
    return false;
  }
}

bool AvailableFile(const std::string& value)
{
  try
  {
    filesystem::path path(value);
    if (path.has_parent_path() && !ExistsDirectory(path.parent_path()))
      return false;
    return IsFile(value) && !ExistsPath(value);
  }
  catch (const std::exception& e)
  {
    LOG(INFO) << e.what();
    return false;
  }
}

bool NotEmpty(const std::string& value)
{
  return !value.empty();
}

} // namespace validate
