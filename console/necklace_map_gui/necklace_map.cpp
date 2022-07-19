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

#include <fstream>
#include <sstream>

#include <QApplication>

#include "cartocrow/core/region_map.h"
#include "cartocrow/necklace_map/circle_necklace.h"
#include "cartocrow/necklace_map/necklace_map.h"
#include "cartocrow/necklace_map/painting.h"
#include "cartocrow/necklace_map/parameters.h"
#include "cartocrow/renderer/geometry_widget.h"
#include "cartocrow/renderer/ipe_renderer.h"

using namespace cartocrow;
using namespace cartocrow::necklace_map;

int main(int argc, char* argv[]) {
	if (argc != 3) {
		std::cout << "Usage: necklace_map_gui <map_file> <data_json>\n";
		return 1;
	}

	const std::string map_filename = argv[1];
	const std::string data_json = argv[2];

	RegionMap map = ipeToRegionMap(map_filename);
	auto map_ptr = std::make_shared<RegionMap>(map);

	NecklaceMap necklaceMap(map_ptr);
	Parameters& parameters = necklaceMap.parameters();
	parameters.wedge_interval_length_min_rad = 0.1 * M_PI;
	parameters.centroid_interval_length_rad = 0.2 * M_PI;
	parameters.order_type = cartocrow::necklace_map::OrderType::kAny;
	parameters.aversion_ratio = 0;

	auto necklace = necklaceMap.addNecklace(
	    std::make_unique<CircleNecklace>(Circle<Inexact>(Point<Inexact>(300, 300), 120 * 120)));
	necklaceMap.addBead("RUS", 145934462, necklace);
	necklaceMap.addBead("DEU", 83783942, necklace);
	necklaceMap.addBead("GBR", 67886011, necklace);
	necklaceMap.addBead("FRA", 65273511, necklace);
	necklaceMap.addBead("ITA", 60461826, necklace);
	necklaceMap.addBead("ESP", 46754778, necklace);
	necklaceMap.addBead("UKR", 43733762, necklace);
	necklaceMap.addBead("POL", 37846611, necklace);
	necklaceMap.addBead("ROU", 19237691, necklace);
	necklaceMap.addBead("NLD", 17134872, necklace);
	necklaceMap.addBead("BEL", 11589623, necklace);
	necklaceMap.addBead("CZE", 10708981, necklace);
	necklaceMap.addBead("GRC", 10423054, necklace);
	necklaceMap.addBead("PR1", 10196709, necklace);
	necklaceMap.addBead("SWE", 10099265, necklace);
	necklaceMap.addBead("HUN", 9660351, necklace);
	necklaceMap.addBead("BLR", 9449323, necklace);
	necklaceMap.addBead("AUT", 9006398, necklace);
	necklaceMap.addBead("SRB", 8737371, necklace);
	necklaceMap.addBead("CHE", 8654622, necklace);
	necklaceMap.addBead("BGR", 6948445, necklace);
	necklaceMap.addBead("DNK", 5792202, necklace);
	necklaceMap.addBead("FIN", 5540720, necklace);
	necklaceMap.addBead("SVK", 5459642, necklace);
	necklaceMap.addBead("NOR", 5421241, necklace);
	necklaceMap.addBead("IRL", 4937786, necklace);
	necklaceMap.addBead("HRV", 4105267, necklace);
	necklaceMap.addBead("MDA", 4033963, necklace);
	necklaceMap.addBead("BIH", 3280819, necklace);
	necklaceMap.addBead("ALB", 2877797, necklace);
	necklaceMap.addBead("LTU", 2722289, necklace);
	necklaceMap.addBead("MKD", 2083374, necklace);
	necklaceMap.addBead("SVN", 2078938, necklace);
	necklaceMap.addBead("LVA", 1886198, necklace);
	necklaceMap.addBead("EST", 1326535, necklace);
	necklaceMap.addBead("MNE", 628066, necklace);
	necklaceMap.addBead("LUX", 625978, necklace);
	necklaceMap.addBead("MLT", 441543, necklace);
	necklaceMap.addBead("ISL", 341243, necklace);
	necklaceMap.addBead("JEY", 173863, necklace);
	necklaceMap.addBead("IMN", 85033, necklace);
	necklaceMap.addBead("AND", 77265, necklace);
	necklaceMap.addBead("FRO", 48863, necklace);
	necklaceMap.addBead("MCO", 39242, necklace);
	necklaceMap.addBead("LIE", 38128, necklace);
	necklaceMap.addBead("SMR", 33931, necklace);
	//necklaceMap.addBead("GIB", 33691, necklace);
	necklaceMap.addBead("VAT", 801, necklace);

	necklaceMap.compute();

	QApplication a(argc, argv);
	a.setApplicationName("CartoCrow necklace map demo");
	cartocrow::necklace_map::Painting::Options options;
	cartocrow::necklace_map::Painting painting(necklaceMap, options);

	cartocrow::renderer::GeometryWidget widget(painting);
	widget.show();
	return a.exec();
}
