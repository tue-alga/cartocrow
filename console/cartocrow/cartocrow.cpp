/*
The Necklace Map console application implements the algorithmic
geo-visualization method by the same name, developed by
Bettina Speckmann and Kevin Verbeek at TU Eindhoven
(DOI: 10.1109/TVCG.2010.180 & 10.1142/S021819591550003X).
Copyright (C) 2021  Netherlands eScience Center and TU Eindhoven

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

#include <filesystem>
#include <fstream>
#include <sstream>

#include <QApplication>

#include <nlohmann/json.hpp>

#include "cartocrow/core/region_map.h"
#include "cartocrow/necklace_map/circle_necklace.h"
#include "cartocrow/necklace_map/necklace_map.h"
#include "cartocrow/necklace_map/painting.h"
#include "cartocrow/necklace_map/parameters.h"
#include "cartocrow/renderer/geometry_widget.h"
#include "cartocrow/renderer/ipe_renderer.h"

using namespace cartocrow;
using namespace cartocrow::necklace_map;
using json = nlohmann::json;

int main(int argc, char* argv[]) {
	if (argc != 2 && argc != 3) {
		std::cout << "Usage: cartocrow <project_file> [<output_file>]\n";
		std::cout << "where <project_file> is a JSON file describing the map to generate,\n";
		std::cout << "and <output_file> is the file to write the output to. If <output_file>\n";
		std::cout << "is omitted, an interactive GUI will be opened instead.\n";
		return 1;
	}

	const std::filesystem::path projectFilename = argv[1];
	std::string outputFilename = "";
	if (argc == 3) {
		outputFilename = argv[2];
	}
	std::ifstream f(projectFilename);
	json projectData = json::parse(f);

	std::cout << projectFilename.parent_path() / projectData["map"] << std::endl;
	RegionMap map = ipeToRegionMap(projectFilename.parent_path() / projectData["map"]);
	auto map_ptr = std::make_shared<RegionMap>(map);

	NecklaceMap necklaceMap(map_ptr);
	Parameters& parameters = necklaceMap.parameters();
	parameters.wedge_interval_length_min_rad = 0.1 * M_PI;
	parameters.centroid_interval_length_rad = 0.2 * M_PI;
	parameters.order_type = cartocrow::necklace_map::OrderType::kAny;
	parameters.aversion_ratio = 0.5;

	for (json& n : projectData["necklaces"]) {
		auto necklace = necklaceMap.addNecklace(std::make_unique<CircleNecklace>(
		    Circle<Inexact>(Point<Inexact>(n["shape"]["center"][0], n["shape"]["center"][1]),
		                    std::pow(n["shape"]["radius"].get<double>(), 2))));
		for (std::string b : n["beads"]) {
			necklaceMap.addBead(b, projectData["data"][b], necklace);
		}
	}

	necklaceMap.compute();

	QApplication a(argc, argv);
	a.setApplicationName("CartoCrow necklace map demo");
	cartocrow::necklace_map::Painting::Options options;
	cartocrow::necklace_map::Painting painting(necklaceMap, options);

	cartocrow::renderer::GeometryWidget widget(painting);
	widget.show();
	return a.exec();
}
