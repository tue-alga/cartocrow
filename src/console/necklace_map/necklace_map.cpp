#include <iostream>

#include <cmake/geoviz_config.h>
#include <geoviz/necklace_map/necklace_map.h>
#include <geoviz/flow_diagram/flow_diagram.h>

#include "console/necklace_map/internal/test_internal.h"

int main(int argc, char** argv) {
    std::cerr << "Args: " << std::endl;
    for ( int i = 1; i < argc; ++i )
        std::cerr << "\t" << argv[i] << std::endl;

    std::cerr << "Necklace map: " << geoviz::proc_necklace_map() << std::endl;
    std::cerr << "Flow Diagram: " << geoviz::proc_flow_diagram() << std::endl;

    // TODO(tvl) place in 'usage' print.
    std::cerr << "GeoViz version: " << GEOVIZ_VERSION << std::endl;

    std::cerr << "Internal number: " << internal::test() << std::endl;

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
