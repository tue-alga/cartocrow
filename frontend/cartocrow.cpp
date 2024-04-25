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

#include "cartocrow/core/centroid.h"
#include "cartocrow/core/region_arrangement.h"
#include "cartocrow/core/region_map.h"
#include "cartocrow/flow_map/painting.h"
#include "cartocrow/flow_map/parameters.h"
#include "cartocrow/flow_map/place.h"
#include "cartocrow/flow_map/spiral_tree.h"
#include "cartocrow/flow_map/spiral_tree_unobstructed_algorithm.h"
#include "cartocrow/isoline_simplification/ipe_isolines.h"
#include "cartocrow/isoline_simplification/isoline_simplifier.h"
#include "cartocrow/isoline_simplification/simple_isoline_painting.h"
#include "cartocrow/necklace_map/circle_necklace.h"
#include "cartocrow/necklace_map/necklace_map.h"
#include "cartocrow/necklace_map/painting.h"
#include "cartocrow/necklace_map/parameters.h"
#include "cartocrow/renderer/geometry_painting.h"
#include "cartocrow/renderer/geometry_widget.h"
#include "cartocrow/renderer/ipe_renderer.h"

using namespace cartocrow;
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

	std::shared_ptr<renderer::GeometryPainting> painting;
	std::shared_ptr<renderer::GeometryPainting> debugPainting;

	if (projectData["type"] == "necklace_map") {
		RegionMap map = ipeToRegionMap(projectFilename.parent_path() / projectData["map"]);
		auto map_ptr = std::make_shared<RegionMap>(map);

		std::shared_ptr<necklace_map::NecklaceMap> necklaceMap =
		    std::make_shared<necklace_map::NecklaceMap>(map_ptr);
		necklace_map::Parameters& parameters = necklaceMap->parameters();
		parameters.wedge_interval_length_min_rad = 0.1 * M_PI;
		parameters.centroid_interval_length_rad = 0.2 * M_PI;
		parameters.order_type = cartocrow::necklace_map::OrderType::kAny;
		parameters.aversion_ratio = 0.5;

		for (json& n : projectData["necklaces"]) {
			auto necklace = necklaceMap->addNecklace(std::make_unique<necklace_map::CircleNecklace>(
			    Circle<Inexact>(Point<Inexact>(n["shape"]["center"][0], n["shape"]["center"][1]),
			                    std::pow(n["shape"]["radius"].get<double>(), 2))));
			for (std::string b : n["beads"]) {
				necklaceMap->addBead(b, projectData["data"][b], necklace);
			}
		}
		necklaceMap->compute();

		necklace_map::Painting::Options options;
		painting = std::make_shared<necklace_map::Painting>(necklaceMap, options);

	} else if (projectData["type"] == "flow_map") {
		RegionMap map = ipeToRegionMap(projectFilename.parent_path() / projectData["map"]);
		auto map_ptr = std::make_shared<RegionMap>(map);

		// TODO [ws] this is temporary: draw the spiral tree until the flow map
		// is implemented
		Region& root = (*map_ptr)[projectData["root"]];
		std::shared_ptr<flow_map::SpiralTree> tree = std::make_shared<flow_map::SpiralTree>(
		    approximate(centroid(root.shape)), projectData["parameters"]["angle"].get<double>());
		for (auto it = projectData["data"].begin(); it != projectData["data"].end(); ++it) {
			tree->addPlace(it.key(), approximate(centroid((*map_ptr)[it.key()].shape)),
			               it.value().get<double>());
		}
		tree->addShields();

		flow_map::SpiralTreeUnobstructedAlgorithm algorithm(*tree);
		algorithm.run();
		debugPainting = algorithm.debugPainting();

		flow_map::Painting::Options options;
		painting = std::make_shared<flow_map::Painting>(map_ptr, tree, options);

	} else if (projectData["type"] == "isoline_simplification") {
		auto isolines = isoline_simplification::ipeToIsolines(projectFilename.parent_path() / projectData["isolines"]);
		isoline_simplification::IsolineSimplifier simplifier(isolines);
		int target = projectData["target"];
		simplifier.simplify(target);
		painting = std::make_shared<isoline_simplification::SimpleIsolinePainting>(simplifier.m_simplified_isolines);
	} else {
		std::cerr << "Unknown type \"" << projectData["type"] << "\" specified\n";
	}

	if (outputFilename == "") {
		QApplication a(argc, argv);
		a.setApplicationName("CartoCrow GUI");

		cartocrow::renderer::GeometryWidget widget(painting);
		if (debugPainting) {
			widget.addPainting(debugPainting, "Debug visualization");
		}
		widget.show();
		return a.exec();

	} else {
		cartocrow::renderer::IpeRenderer renderer(painting);
		renderer.save(outputFilename);
		return 0;
	}
}
