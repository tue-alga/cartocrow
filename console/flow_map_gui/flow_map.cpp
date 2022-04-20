/*
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
*/

#include <fstream>
#include <sstream>

#include <QApplication>

#include "cartocrow/flow_map/io/data_reader.h"
#include "cartocrow/flow_map/io/ipe_reader.h"
#include "cartocrow/flow_map/painting.h"
#include "cartocrow/flow_map/parameters.h"
#include "cartocrow/flow_map/place.h"
#include "cartocrow/flow_map/spiral_tree.h"
#include "cartocrow/flow_map/spiral_tree_unobstructed_algorithm.h"
#include "cartocrow/renderer/geometry_widget.h"
#include "cartocrow/renderer/ipe_renderer.h"

int main(int argc, char* argv[]) {
	if (argc != 4) {
		std::cout << "Usage: flow_map_gui <map_file> <data_file> <value_name>\n";
		return 1;
	}

	const std::string map_filename = argv[1];
	const std::string data_filename = argv[2];
	const std::string value_name = argv[3];

	using Region = cartocrow::Region;
	using Place = cartocrow::flow_map::Place;
	std::vector<Region> regions;
	std::vector<Region> obstacles;
	std::vector<std::shared_ptr<Place>> places;
	//std::vector<std::shared_ptr<Place>> waypoints;
	cartocrow::flow_map::IpeReader ipe_reader;
	bool success_read_ipe = ipe_reader.readFile(map_filename, regions, obstacles, places);
	if (!success_read_ipe) {
		std::cout << "Couldn't read map file\n";
		return 1;
	}

	size_t index_root;
	cartocrow::flow_map::DataReader data_reader;
	bool success_read_data = data_reader.readFile(data_filename, value_name, places, index_root);
	if (!success_read_data) {
		std::cout << "Couldn't read data file\n";
		return 1;
	}
	cartocrow::flow_map::Parameters parameters;

	const cartocrow::Point root = places[index_root]->position.to_cartesian();
	cartocrow::flow_map::SpiralTree spiral_tree(root, parameters.restricting_angle);
	for (const auto& place : places) {
		if (place->flow_in > 0) {
			spiral_tree.addPlace(*place);
		}
	}
	for (const auto& obstacle : obstacles) {
		for (const auto& polygon : obstacle.shape) {
			spiral_tree.addObstacle(polygon.outer_boundary());
		}
	}
	cartocrow::flow_map::SpiralTreeUnobstructedAlgorithm algorithm(spiral_tree);
	algorithm.run();

	QApplication a(argc, argv);
	a.setApplicationName("CartoCrow flow map demo");
	cartocrow::flow_map::Painting::Options options;
	cartocrow::flow_map::Painting painting(spiral_tree, regions, obstacles, options);
	cartocrow::renderer::IpeRenderer renderer(painting);

	cartocrow::renderer::GeometryWidget widget(painting);
	widget.show();
	return a.exec();
}
