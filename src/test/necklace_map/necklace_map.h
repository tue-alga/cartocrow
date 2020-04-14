/*
The Necklace Map console application implements the algorithmic
geo-visualization method by the same name, developed by
Bettina Speckmann and Kevin Verbeek at TU Eindhoven
(DOI: 10.1109/TVCG.2010.180 & 10.1142/S021819591550003X).
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 10-04-2020
*/

#ifndef GEOVIZ_TEST_NECKLACE_MAP_NECKLACE_MAP_H
#define GEOVIZ_TEST_NECKLACE_MAP_NECKLACE_MAP_H

#include <glog/logging.h>

#include <cmake/geoviz_test_config.h>
#include <console/common/utils_filesystem.h>
#include <console/necklace_map_io/necklace_map_io.h>
#include <geoviz/necklace_map/necklace_map.h>

#include "test/test.h"


void TestNecklaceMap() {}  // Linking hack, each new test cpp file has it.

UNITTEST_SUITE(NecklaceMap) {

  bool ReadData
  (
    const std::string& in_geometry_filename,
    const std::string& in_data_filename,
    const std::string& in_value_name,
    std::vector<geoviz::necklace_map::MapElement::Ptr>& elements,
    std::vector<geoviz::necklace_map::Necklace::Ptr>& necklaces
  )
  {
    // Read the data and geometry.
    geoviz::DataReader data_reader;
    geoviz::SvgReader svg_reader;

    const bool success_read_data = data_reader.ReadFile(in_data_filename, in_value_name, elements);
    const bool success_read_svg = svg_reader.ReadFile(in_geometry_filename, elements, necklaces);

    return success_read_svg && success_read_data;
  }

  UNITTEST_TEST(WesternEurope)
  {
    using MapElement = geoviz::necklace_map::MapElement;
    using Necklace = geoviz::necklace_map::Necklace;
    std::vector<MapElement::Ptr> elements;
    std::vector<Necklace::Ptr> necklaces;

    const filesystem::path test_dir = GEOVIZ_TEST_DIR;

    const filesystem::path in_geometry_path = test_dir / "wEU.xml";
    const filesystem::path in_data_path = test_dir / "wEU.txt";
    const std::string in_value_name = "value";

    FLAGS_minloglevel = 2;
    UNITTEST_CHECK(ReadData(in_geometry_path, in_data_path, in_value_name, elements, necklaces));

    geoviz::necklace_map::Parameters parameters;
    parameters.interval_type = geoviz::necklace_map::IntervalType::kWedge;
    parameters.centroid_interval_length_rad = 0.2 * M_PI;
    parameters.ignore_point_regions = false;
    parameters.order_type = geoviz::necklace_map::OrderType::kAny;
    parameters.buffer_rad = 0;
    parameters.aversion_ratio = 0.001;

    const geoviz::Number scale_factor = ComputeScaleFactor(parameters, elements, necklaces);
    UNITTEST_CHECK_CLOSE(scale_factor, 1.675, 0.001);
  }

} // suite_NecklaceMap

#endif //GEOVIZ_TEST_NECKLACE_MAP_NECKLACE_MAP_H
