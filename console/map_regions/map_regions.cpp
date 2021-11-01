/*
The map_regions application inspects a SVG map file, checks if it is
valid, and returns a list of regions in the map.
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 10-09-2019
*/

#include <fstream>
#include <sstream>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include <cartocrow/core/timer.h>
#include <cartocrow/necklace_map/necklace_map.h>

#include "console/common/utils_cla.h"
#include "console/common/utils_flags.h"

DEFINE_string(in_geometry_filename, "", "The input map geometry filename.");

void validateFlags() {
	bool correct = true;
	LOG(INFO) << "map_regions_cla flags:";

	// Note that we mainly print flags to enable reproducibility.
	// Other flags are validated, but only printed if not valid.
	// Note that we may skip some low-level flags that almost never change.
	using namespace validate;

	// There must be an input map.
	correct &= CheckAndPrintFlag(FLAGS_NAME_AND_VALUE(in_geometry_filename), ExistsFile);

	PrintFlag(FLAGS_NAME_AND_VALUE(stderrthreshold));
	PrintFlag(FLAGS_NAME_AND_VALUE(v));

	if (!correct) {
		LOG(FATAL) << "Errors in flags; Terminating.";
	}
}

bool readGeometry(std::vector<cartocrow::necklace_map::MapElement::Ptr>& elements,
                  std::vector<cartocrow::necklace_map::Necklace::Ptr>& necklaces,
                  cartocrow::Number& scale_factor) {
	cartocrow::necklace_map::IpeReader reader;
	return reader.readFile(FLAGS_in_geometry_filename, elements, necklaces);
}

void writeOutput(const std::vector<cartocrow::necklace_map::MapElement::Ptr>& elements,
                 const std::vector<cartocrow::necklace_map::Necklace::Ptr>& necklaces) {
	for (auto element : elements) {
		std::cout << element->region.id << "\n";
	}
}

int main(int argc, char** argv) {
	InitApplication(argc, argv, "Checks the validity of an SVG map, and outputs its regions.",
	                {"--in_geometry_filename=<file>"});

	// Validate the settings.
	validateFlags();

	using MapElement = cartocrow::necklace_map::MapElement;
	using Necklace = cartocrow::necklace_map::Necklace;
	std::vector<MapElement::Ptr> elements;
	std::vector<Necklace::Ptr> necklaces;

	// Read the map.
	cartocrow::Number scale_factor;
	const bool success_read_svg = readGeometry(elements, necklaces, scale_factor);
	CHECK(success_read_svg) << "Map invalid";

	// Write the output.
	writeOutput(elements, necklaces);

	return 0;
}
