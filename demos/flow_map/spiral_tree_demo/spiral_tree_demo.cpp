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
*/

#include <filesystem>
#include <fstream>
#include <sstream>

#include <QApplication>

#include "cartocrow/core/core.h"
#include "cartocrow/flow_map/painting.h"
#include "cartocrow/flow_map/parameters.h"
#include "cartocrow/flow_map/place.h"
#include "cartocrow/flow_map/spiral_tree.h"
#include "cartocrow/flow_map/spiral_tree_obstructed_algorithm.h"
#include "cartocrow/renderer/geometry_painting.h"
#include "cartocrow/renderer/geometry_widget.h"
#include "cartocrow/renderer/ipe_renderer.h"

using namespace cartocrow;
using namespace cartocrow::flow_map;

int main(int argc, char* argv[]) {

	auto tree = std::make_shared<SpiralTree>(Point<Inexact>(0, 0), 0.5061454830783556);
	tree->addPlace("p1", Point<Inexact>(0, 400), 1);
	Polygon<Inexact> obstacle;
	obstacle.push_back(Point<Inexact>(0, 50));
	obstacle.push_back(Point<Inexact>(8, 95));
	obstacle.push_back(Point<Inexact>(50, 140));
	obstacle.push_back(Point<Inexact>(-43, 134));
	obstacle.push_back(Point<Inexact>(-50, 100));
	tree->addObstacle(obstacle);
	SpiralTreeObstructedAlgorithm algorithm(tree);
	algorithm.run();

	Painting::Options options;
	Painting p(nullptr, tree, options);
	auto painting = std::make_shared<Painting>(nullptr, tree, options);

	QApplication app(argc, nullptr);
	renderer::GeometryWidget renderer(painting);
	renderer.addPainting(algorithm.debugPainting());
	renderer.show();
	app.exec();
}
