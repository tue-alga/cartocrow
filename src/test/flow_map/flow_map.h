/*
The Flow Map console application implements the algorithmic
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

#ifndef GEOVIZ_TEST_FLOW_MAP_FLOW_MAP_H
#define GEOVIZ_TEST_FLOW_MAP_FLOW_MAP_H

#include <glog/logging.h>

#include <cmake/geoviz_test_config.h>
#include <console/common/utils_filesystem.h>
#include <geoviz/flow_map/flow_map.h>

#include "test/test.h"


void TestFlowMap() {}  // Linking hack, each new test cpp file has it.

struct FlowData
{
  FlowData()
  {
    // Disable logging to INFO and WARNING.
    FLAGS_minloglevel = 2;
  }

  std::vector<geoviz::Region> context;
  std::vector<geoviz::flow_map::Node> nodes;
}; // struct FlowData

static const filesystem::path kDataDir = filesystem::path(GEOVIZ_TEST_DATA_DIR) / "flow_map";

static FlowData kUsa = FlowData();
static FlowData kWorld = FlowData();


UNITTEST_SUITE(FlowMap)
{

void DefaultParameters(geoviz::flow_map::Parameters& parameters)
{
//  parameters.interval_type = geoviz::necklace_map::IntervalType::kWedge;
//  parameters.centroid_interval_length_rad = 0.2 * M_PI;
//  parameters.ignore_point_regions = false;
//
//  parameters.order_type = geoviz::necklace_map::OrderType::kAny;
//  parameters.buffer_rad = 0;
//  parameters.aversion_ratio = 0.001;
}


struct FlowDataUsa
{
  FlowDataUsa() :
    data(kUsa)
  {
    if (data.nodes.empty())
    {
      // Read the geometry.
      geoviz::flow_map::SvgReader svg_reader;
      const filesystem::path in_geometry_path = kDataDir / "USA.svg";
      UNITTEST_REQUIRE UNITTEST_CHECK(svg_reader.ReadFile(in_geometry_path, data.context, data.nodes));
    }
  }

  bool ReadValues(const std::string& in_value_name)
  {
    if (in_value_name == value_name)
      return true;
    value_name = in_value_name;

    geoviz::flow_map::DataReader data_reader;
    const filesystem::path in_data_path = kDataDir / "USA.csv";
    return data_reader.ReadFile(in_data_path, in_value_name, data.nodes);
  }

  FlowData& data;
  std::string value_name;

  geoviz::flow_map::Parameters parameters;
}; // struct FlowDataUsa

UNITTEST_TEST_FIXTURE(FlowDataUsa, UsaGreedy)
{
  const std::string in_value_name = "CA";
  UNITTEST_CHECK(ReadValues(in_value_name));

  DefaultParameters(parameters);
//  parameters.interval_type = geoviz::necklace_map::IntervalType::kCentroid;
//  parameters.order_type = geoviz::necklace_map::OrderType::kFixed;

//  const geoviz::Number scale_factor = ComputeScaleFactor(parameters, data.elements, data.necklaces);
//  UNITTEST_CHECK_CLOSE(1.580, scale_factor, 0.001);
}


struct FlowDataWorld
{
  FlowDataWorld() :
    data(kWorld)
  {
    if (data.nodes.empty())
    {
      // Read the geometry.
      geoviz::flow_map::SvgReader svg_reader;
      const filesystem::path in_geometry_path = kDataDir / "World.svg";
      UNITTEST_REQUIRE UNITTEST_CHECK(svg_reader.ReadFile(in_geometry_path, data.context, data.nodes));
    }
  }

  bool ReadValues(const std::string& in_value_name)
  {
    if (in_value_name == value_name)
      return true;
    value_name = in_value_name;

    geoviz::flow_map::DataReader data_reader;
    const filesystem::path in_data_path = kDataDir / "World.csv";
    return data_reader.ReadFile(in_data_path, in_value_name, data.nodes);
  }

  FlowData& data;
  std::string value_name;

  geoviz::flow_map::Parameters parameters;
}; // struct FlowDataWorld

UNITTEST_TEST_FIXTURE(FlowDataWorld, EastAsiaAgriculture)
{
  const std::string in_value_name = "Karstner";
  UNITTEST_CHECK(ReadValues(in_value_name));

  DefaultParameters(parameters);

//  const geoviz::Number scale_factor = ComputeScaleFactor(parameters, data.elements, data.necklaces);
//  UNITTEST_CHECK_CLOSE(1.005, scale_factor, 0.001);
}

} // UNITTEST_SUITE(FlowMap)

#endif //GEOVIZ_TEST_FLOW_MAP_FLOW_MAP_H
