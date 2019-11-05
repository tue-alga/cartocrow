/*
Application to expose the functionality of the library by the same name.
Copyright (C) 2019  Netherlands eScience Center and [IP's institution]

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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 10-09-2019
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

#include "console/common/utils_cla.h"
#include "console/necklace_map/internal/test_internal.h"


DEFINE_string(test_flag, "", "A test for the gflags dependency for parsing command-line arguments");


int main(int argc, char **argv)
{
  initApplication
  (
    argc,
    argv,
    "Command line application that exposes the functionality of the GeoViz necklace map.",
    {"<some arg>", "<another arg>"}
  );

  // Writing to the standard output is reserved for text that should be returned to a calling website.
  FLAGS_logtostderr = true;

  LOG(INFO) << "Flags:";
  LOG(INFO) << FLAGS_test_flag;

  // Note that here is a good place to check the validity of the flags.
  // While this can be done by adding flag validators using gflags,
  // These generally only allow (statically) validating independent flags,
  // they do not allow validating programmatically set flags,
  // and they may throw inconvenient compiler warnings about unused variables
  // (depending on the implementation).
  // Instead of registering validators, I prefer to add a "ValidateFlags()"
  // method call here that performs all the validation and error messaging.
  // For added value, this method could always log the values of important flags
  // to make it easier to reproduce a particular program execution.

  LOG(INFO) << "Args:";
  for (int i = 1; i < argc; ++i)
    LOG(INFO) << "\t" << argv[i];

  LOG(INFO) << "Necklace map: " << geoviz::proc_necklace_map();
  LOG(INFO) << "Flow Diagram: " << geoviz::proc_flow_diagram();

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
