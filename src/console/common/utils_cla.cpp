/*
Generic methods for command-line applications.
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

#include "utils_cla.h"

#include <sstream>
#include <experimental/filesystem>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <cmake/geoviz_config.h>


std::string copyrightNotice(const bool interactive /*= false*/)
{
  std::stringstream out;
  out << "Copyright (C) 2019  Netherlands eScience Center and TU Eindhoven" << std::endl;
  out << "This program comes with ABSOLUTELY NO WARRANTY" << (interactive ? "; type `show w' for details." : ".") << std::endl;
  out << "This is free software, and you are welcome to redistribute it under certain" << std::endl;
  out << "conditions" << (interactive ? "; type `show c' for details." : ".");
  return out.str();
}

std::string getUsageMessage
(
  const std::string& executable_name,
  const std::string& description,
  const std::vector<std::string>& sample_arguments /*= {}*/
)
{
  // Note that GCC 7.4.0 implements the C++17 filesystem specification as experimental.
  // This experimental implementation requires linking the "stdc++fs" static library.
  // Note that this choice for compiler and version is made for convenience:
  // it is currently shipped with my operating system.
  // Other compilers or later versions may fully implement the C++17 standard,
  // in which case this namespace will need to be updated as well.
  namespace filesystem = std::experimental::filesystem;

  std::stringstream out;
  out << std::endl << copyrightNotice() << std::endl << std::endl;
  out << description << std::endl;
  out << "Sample usage:" << std::endl;
  out << filesystem::path(executable_name).filename().string();
  for (const std::string& argument : sample_arguments)
    out << std::endl << "\t" << argument;
  return out.str();
}

void initApplication
(
  int argc,
  char **argv,
  const std::string& description,
  const std::vector<std::string>& sample_arguments /*= {}*/
)
{
  google::InitGoogleLogging(argv[0]);

  gflags::SetUsageMessage(getUsageMessage(argv[0], description, sample_arguments));
  gflags::SetVersionString(GEOVIZ_VERSION);

  // Note that the command line flags are parsed after applying the defaults above.
  gflags::ParseCommandLineFlags(&argc, &argv, true);
}
