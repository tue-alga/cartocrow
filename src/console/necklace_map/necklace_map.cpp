/*
The Necklace Map application exposes the functionality of the library
by the same name.
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 10-09-19
*/

// Note dependences:
// - glog
// - gflags

#include <iostream>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <cmake/geoviz_config.h>
#include <geoviz/necklace_map/necklace_map.h>
#include <geoviz/flow_diagram/flow_diagram.h>
#include "console/necklace_map/internal/test_internal.h"


DEFINE_string(test_flag, "", "A test for the gflags dependency for parsing command-line arguments");
// TODO(tvl) add dependency: glog and gflags for logging and capturing command line parameters (check for other, better libraries).

/**
 * Example long doxygen comment describing this method.
 * @return the copyright notice.
 */
std::string copyrightNotice()
{
  return
    "necklace_map_cla  Copyright (C) 2019  Netherlands eScience Center\n"
    "This program comes with ABSOLUTELY NO WARRANTY; for details type `show w'.\n"
    "This is free software, and you are welcome to redistribute it\n"
    "under certain conditions; type `show c' for details.";
}

/// Example brief doxygen comment describing this method.
std::string getUsageMessage(std::string executable_name)
{
  std::string usage = "This is where I should set the usage message.  Sample usage:\n";
  usage += executable_name + " <uselessarg1> <uselessarg2>";
  return usage;
}

int main(int argc, char **argv)
{
  google::InitGoogleLogging(argv[0]);

  gflags::SetUsageMessage(getUsageMessage(argv[0]));
  gflags::SetVersionString(GEOVIZ_VERSION);
  gflags::ParseCommandLineFlags(&argc, &argv, true);

  FLAGS_logtostderr = true;
  LOG(INFO) << "Google logging enabled!";

  LOG(INFO) << "Flags:";
  LOG(INFO) << FLAGS_test_flag;
  // Note that here is a good place to check the validity of the flags.
  // While this can be done by adding flag validators using gflags,
  // These generally only allow validating independent flags,
  // they do not allow validating programmatically set flags,
  // and they may throw inconvenient compiler warnings about unused variables
  // (depending on the implementation).

  LOG(INFO) << "Args:";
  for (int i = 1; i < argc; ++i)
    LOG(INFO) << "\t" << argv[i];

  LOG(INFO) << "Necklace map: " << geoviz::proc_necklace_map();
  LOG(INFO) << "Flow Diagram: " << geoviz::proc_flow_diagram();

  // TODO(tvl) place in 'usage' print.
  LOG(INFO) << "GeoViz version: " << GEOVIZ_VERSION;

  LOG(INFO) << "Internal number: " << geoviz::internal::test();

  std::cout << "<div>";
  std::cout << "<!--To be loaded as support card.-->";
  std::cout << "<h1>Application-based component</h1>";
  std::cout << "<p>This page is just to test running a server-side application using PHP.</p>";
  std::cout << "<p>The following command line arguments were caught:<br>";
  for (int i = 1; i < argc; ++i)
    std::cout << argv[i] << "<br>";
  std::cout << "</p>";
  std::cout << "</div>";

  return 0;
}
