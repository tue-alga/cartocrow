#include <iostream>

#include <cmake/geoviz_config.h>
#include <geoviz/necklace_map/necklace_map.h>
#include <geoviz/flow_diagram/flow_diagram.h>

#include "console/necklace_map/internal/test_internal.h"

int main(int argc, char** argv) {
    std::cout << "Args: " << std::endl;
    for ( int i = 1; i < argc; ++i )
        std::cout << "\t" << argv[i] << std::endl;

    std::cout << "Necklace map: " << geoviz::proc_necklace_map() << std::endl;
    std::cout << "Flow Diagram: " << geoviz::proc_flow_diagram() << std::endl;

    // TODO(tvl) place in 'usage' print.
    std::cout << "GeoViz version: " << GEOVIZ_VERSION << std::endl;

    std::cout << "Internal number: " << internal::test() << std::endl;

    return 0;
}
