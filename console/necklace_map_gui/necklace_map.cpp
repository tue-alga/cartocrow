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

bool readData(std::string data_filename, std::string value_name,
              std::vector<cartocrow::necklace_map::MapElement::Ptr>& elements) {
	cartocrow::necklace_map::DataReader data_reader;
	return data_reader.ReadFile(data_filename, value_name, elements);
}

bool readMap(std::string map_filename,
             std::vector<cartocrow::necklace_map::MapElement::Ptr>& elements,
             std::vector<cartocrow::necklace_map::Necklace::Ptr>& necklaces,
             cartocrow::Number& scale_factor) {
	cartocrow::necklace_map::SvgReader svg_reader;
	return svg_reader.ReadFile(map_filename, elements, necklaces, scale_factor);
}

void applyNecklaceDrawBounds(std::vector<cartocrow::necklace_map::Necklace::Ptr>& necklaces,
                             const std::string& bound_necklaces_deg) {
	std::stringstream stream(bound_necklaces_deg);
	while (stream && !stream.eof()) {
		std::string token;
		stream >> token;

		std::vector<std::string> bits;
		while (!token.empty()) {
			const size_t pos = token.find(";");
			bits.push_back(token.substr(0, pos));
			token = pos == std::string::npos ? "" : token.substr(pos + 1);
		}
		if (bits.size() < 3) {
			continue;
		}

		const std::string necklace_id = bits[0];
		double cw_deg, ccw_deg;

		try {
			cw_deg = std::stod(bits[1]);
			ccw_deg = std::stod(bits[2]);
		} catch (...) {
			continue;
		}

		for (cartocrow::necklace_map::Necklace::Ptr& necklace : necklaces) {
			if (necklace->id != necklace_id) {
				continue;
			}

			cartocrow::necklace_map::CircleNecklace::Ptr shape =
			    std::dynamic_pointer_cast<cartocrow::necklace_map::CircleNecklace>(necklace->shape);
			if (!shape) {
				continue;
			}

			shape->draw_bounds_cw_rad() = cw_deg * M_PI / 180;
			shape->draw_bounds_ccw_rad() = ccw_deg * M_PI / 180;
			break;
		}
	}
}

int main(int argc, char* argv[]) {
	if (argc != 4) {
		std::cout << "Usage: necklace_map_gui <map_file> <data_file> <value_name>\n";
		return 1;
	}

	using MapElement = cartocrow::necklace_map::MapElement;
	using Necklace = cartocrow::necklace_map::Necklace;
	std::vector<MapElement::Ptr> elements;
	std::vector<Necklace::Ptr> necklaces;

	const std::string map_filename = argv[1];
	const std::string data_filename = argv[2];
	const std::string value_name = argv[3];

	cartocrow::Number scale_factor;
	const bool success_read_svg = readMap(map_filename, elements, necklaces, scale_factor);
	if (!success_read_svg) {
		std::cout << "Couldn't read data file\n";
		return 1;
	}
	const bool success_read_data = readData(data_filename, value_name, elements);
	if (!success_read_data) {
		std::cout << "Couldn't read data file\n";
		return 1;
	}
	cartocrow::necklace_map::Parameters parameters;

	scale_factor = cartocrow::necklace_map::ComputeScaleFactor(parameters, elements, necklaces);
	applyNecklaceDrawBounds(necklaces, "");

	QApplication a(argc, argv);
	a.setApplicationName("CartoCrow necklace map demo");
	cartocrow::necklace_map::Painting painting(elements, necklaces, scale_factor);
	cartocrow::renderer::GeometryWidget widget(painting);
	widget.show();

	return a.exec();
}
