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

#include <iostream>

#include <cmake/geoviz_config.h>
#include <geoviz/necklace_map/necklace_map.h>
#include <geoviz/flow_diagram/flow_diagram.h>

#include "console/necklace_map/internal/test_internal.h"

// TODO(tvl) add dependency: glog and gflags for logging and capturing command line parameters (check for other, better libraries).

std::string copyrightNotice()
{
    return
        "necklace_map_cla  Copyright (C) 2019  Netherlands eScience Center\n"
        "This program comes with ABSOLUTELY NO WARRANTY; for details type `show w'.\n"
        "This is free software, and you are welcome to redistribute it\n"
        "under certain conditions; type `show c' for details.";
}

int main(int argc, char** argv)
{
    std::cerr << "Args: " << std::endl;
    for ( int i = 1; i < argc; ++i )
        std::cerr << "\t" << argv[i] << std::endl;

    std::cerr << "Necklace map: " << geoviz::proc_necklace_map() << std::endl;
    std::cerr << "Flow Diagram: " << geoviz::proc_flow_diagram() << std::endl;

    // TODO(tvl) place in 'usage' print.
    std::cerr << "GeoViz version: " << GEOVIZ_VERSION << std::endl;

    std::cerr << "Internal number: " << geoviz::internal::test() << std::endl;

    std::cout << "<div>";
    std::cout << "<!--To be loaded as support card.-->";
    std::cout << "<h1>Application-based component</h1>";
    std::cout << "<p>This page is just to test running a server-side application using PHP.</p>";
    std::cout << "<p>The following command line arguments were caught:<br>";
    for ( int i = 1; i < argc; ++i )
        std::cout << argv[i] << "<br>";
    std::cout << "</p>";
    std::cout << "</div>";

    return 0;
}
