/*
The print_arguments application prints its arguments formatted as a div HTML element.
Copyright (C) 2019  Netherlands eScience Center

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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 09-10-2019
*/

#include <iostream>

#include <gflags/gflags.h>

#include <cmake/geoviz_config.h>


std::string copyrightNotice()
{
  return
    "print_arguments  Copyright (C) 2019  Netherlands eScience Center\n"
    "This program comes with ABSOLUTELY NO WARRANTY.\n"
    "This is free software, and you are welcome to redistribute it\n"
    "under certain conditions.";
}

/// Example brief doxygen comment describing this method.
std::string getUsageMessage(std::string executable_name)
{
  std::string usage = "Prints its arguments formatted as a div HTML element.\nSample usage:\n";
  usage += executable_name + " [args...]";
  return usage;
}

int main(int argc, char **argv)
{
  gflags::SetUsageMessage(getUsageMessage(argv[0]));
  gflags::SetVersionString(GEOVIZ_VERSION);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  std::cout << "<div>";
  std::cout << "<h1>Externally generated content</h1>";
  std::cout << "<p>This page is just to test running a server-side application using PHP.</p>";
  std::cout << "<p>The following command line arguments were caught:<br>";
  for (int i = 1; i < argc; ++i)
    std::cout << argv[i] << "<br>";
  std::cout << "</p>";
  std::cout << "</div>";

  return 0;
}
