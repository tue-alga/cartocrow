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

#include "cartocrow/necklace_map/parameters.h"
#include <fstream>
#include <sstream>

#include <QApplication>

#include <cartocrow/necklace_map/necklace_map.h>
#include <cartocrow/necklace_map/painting.h>
#include <cartocrow/renderer/geometry_widget.h>

int main(int argc, char* argv[]) {
	if (argc != 4) {
		std::cout << "Usage: necklace_map_gui <map_file> <data_file> <value_name>\n";
		return 1;
	}

	const std::string map_filename = argv[1];
	const std::string data_filename = argv[2];
	const std::string value_name = argv[3];

	using MapElement = cartocrow::necklace_map::MapElement;
	using Necklace = cartocrow::necklace_map::Necklace;
	std::vector<MapElement::Ptr> elements;
	std::vector<Necklace::Ptr> necklaces;
	cartocrow::Number scale_factor;
	cartocrow::necklace_map::IpeReader ipe_reader;
	bool success_read_ipe = ipe_reader.readFile(map_filename, elements, necklaces, scale_factor);
	if (!success_read_ipe) {
		std::cout << "Couldn't read map file\n";
		return 1;
	}
	cartocrow::necklace_map::DataReader data_reader;
	bool success_read_data = data_reader.ReadFile(data_filename, value_name, elements);
	if (!success_read_data) {
		std::cout << "Couldn't read data file\n";
		return 1;
	}
	cartocrow::necklace_map::Parameters parameters;
	parameters.buffer_rad = 0;
	parameters.wedge_interval_length_min_rad = 0.1 * M_PI;
	parameters.centroid_interval_length_rad = 0.2 * M_PI;
	parameters.order_type = cartocrow::necklace_map::OrderType::kAny;
	parameters.aversion_ratio = 0;

	scale_factor = cartocrow::necklace_map::ComputeScaleFactor(parameters, elements, necklaces);

	QApplication a(argc, argv);
	a.setApplicationName("CartoCrow necklace map demo");
	cartocrow::necklace_map::Painting painting(elements, necklaces, scale_factor);
	cartocrow::renderer::GeometryWidget widget(painting);
	widget.show();

	return a.exec();
}
