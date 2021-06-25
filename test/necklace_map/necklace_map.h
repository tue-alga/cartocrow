/*
The Necklace Map library implements the algorithmic geo-visualization
method by the same name, developed by Bettina Speckmann and Kevin Verbeek
at TU Eindhoven (DOI: 10.1109/TVCG.2010.180 & 10.1142/S021819591550003X).
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 10-04-2020
*/

#ifndef GEOVIZ_TEST_NECKLACE_MAP_NECKLACE_MAP_H
#define GEOVIZ_TEST_NECKLACE_MAP_NECKLACE_MAP_H

#include <filesystem>
#include <memory>

#include <glog/logging.h>

#include <cmake/geoviz_test_config.h>
#include <geoviz/common/timer.h>
#include <geoviz/necklace_map/necklace_map.h>

#include "test/test.h"
#include "test/test_registry_timer.h"


void TestNecklaceMap() {}  // Linking hack, each new test cpp file has it.

constexpr const size_t kP = 2;
constexpr const size_t kV = 2;

using PT = PrintTimes<double, kP, kV>;
using Reg = PT::R_;

static Reg kRegistry;


struct NecklaceData
{
  NecklaceData()
  {
    // Disable logging to INFO and WARNING.
    FLAGS_minloglevel = 2;
  }

  std::vector<geoviz::necklace_map::MapElement::Ptr> elements;
  std::vector<geoviz::necklace_map::Necklace::Ptr> necklaces;
}; // struct NecklaceData

static const std::filesystem::path kDataDir = std::filesystem::path(GEOVIZ_TEST_DATA_DIR) / "necklace_map";

static NecklaceData kWesternEurope = NecklaceData();
static NecklaceData kEastAsia = NecklaceData();


UNITTEST_SUITE(NecklaceMap)
{

void DefaultParameters(geoviz::necklace_map::Parameters& parameters)
{
  parameters.interval_type = geoviz::necklace_map::IntervalType::kWedge;
  parameters.centroid_interval_length_rad = 0.2 * M_PI;
  parameters.ignore_point_regions = false;

  parameters.order_type = geoviz::necklace_map::OrderType::kAny;
  parameters.buffer_rad = 0;
  parameters.aversion_ratio = 0.001;
}


struct NecklaceDataWesternEurope
{
  NecklaceDataWesternEurope() :
    data(kWesternEurope)
  {
    if (data.elements.empty())
    {
      kRegistry.Register(&data, "NecklaceDataWesternEurope");

      // Read the geometry.
      geoviz::necklace_map::SvgReader svg_reader;
      const std::filesystem::path in_geometry_path = kDataDir / "wEU.svg";

      geoviz::Timer time;
      UNITTEST_REQUIRE UNITTEST_CHECK(svg_reader.ReadFile(in_geometry_path, data.elements, data.necklaces));
      kRegistry(&data, 0) = time.Stamp();
    }
  }

  bool ReadValues(const std::string& in_value_name)
  {
    if (in_value_name == value_name)
      return true;
    value_name = in_value_name;

    geoviz::necklace_map::DataReader data_reader;
    const std::filesystem::path in_data_path = kDataDir / "wEU.txt";

    geoviz::Timer time;
    const bool result = data_reader.ReadFile(in_data_path, in_value_name, data.elements);
    kRegistry(&data, 1) = time.Stamp();

    return result;
  }

  NecklaceData& data;
  std::string value_name;

  geoviz::necklace_map::Parameters parameters;
}; // struct NecklaceDataWesternEurope

UNITTEST_TEST_FIXTURE(NecklaceDataWesternEurope, WestEuropeCentroidFixed)
{
  const std::string in_value_name = "value";
  UNITTEST_CHECK(ReadValues(in_value_name));

  DefaultParameters(parameters);
  parameters.interval_type = geoviz::necklace_map::IntervalType::kCentroid;
  parameters.order_type = geoviz::necklace_map::OrderType::kFixed;

  const geoviz::Number scale_factor = ComputeScaleFactor(parameters, data.elements, data.necklaces);
  UNITTEST_CHECK_CLOSE(1.580, scale_factor, 0.001);
}

UNITTEST_TEST_FIXTURE(NecklaceDataWesternEurope, WestEuropeCentroidFixedNopoints)
{
  const std::string in_value_name = "value";
  UNITTEST_CHECK(ReadValues(in_value_name));

  DefaultParameters(parameters);
  parameters.interval_type = geoviz::necklace_map::IntervalType::kCentroid;
  parameters.ignore_point_regions = true;
  parameters.order_type = geoviz::necklace_map::OrderType::kFixed;

  const geoviz::Number scale_factor = ComputeScaleFactor(parameters, data.elements, data.necklaces);
  UNITTEST_CHECK_CLOSE(1.813, scale_factor, 0.001);
}

UNITTEST_TEST_FIXTURE(NecklaceDataWesternEurope, WestEuropeCentroidFixedBuffer)
{
  const std::string in_value_name = "value";
  UNITTEST_CHECK(ReadValues(in_value_name));

  DefaultParameters(parameters);
  parameters.interval_type = geoviz::necklace_map::IntervalType::kCentroid;
  parameters.order_type = geoviz::necklace_map::OrderType::kFixed;
  parameters.buffer_rad = 0.22;
    
  const geoviz::Number scale_factor = ComputeScaleFactor(parameters, data.elements, data.necklaces);
  UNITTEST_CHECK_CLOSE(0.629, scale_factor, 0.001);
}

UNITTEST_TEST_FIXTURE(NecklaceDataWesternEurope, WestEurope)
{
  const std::string in_value_name = "value";
  UNITTEST_CHECK(ReadValues(in_value_name));

  DefaultParameters(parameters);

  const geoviz::Number scale_factor = ComputeScaleFactor(parameters, data.elements, data.necklaces);
  UNITTEST_CHECK_CLOSE(1.675, scale_factor, 0.001);
}

UNITTEST_TEST_FIXTURE(NecklaceDataWesternEurope, WestEuropeDegenerateCentroid)
{
  const std::string in_value_name = "value";
  UNITTEST_CHECK(ReadValues(in_value_name));

  DefaultParameters(parameters);
  parameters.interval_type = geoviz::necklace_map::IntervalType::kCentroid;
  parameters.centroid_interval_length_rad = 0;

  const geoviz::Number scale_factor = ComputeScaleFactor(parameters, data.elements, data.necklaces);
  UNITTEST_CHECK_CLOSE(0.403, scale_factor, 0.001);
}

UNITTEST_TEST_FIXTURE(NecklaceDataWesternEurope, WestEuropeNopoints)
{
  const std::string in_value_name = "value";
  UNITTEST_CHECK(ReadValues(in_value_name));

  DefaultParameters(parameters);
  parameters.ignore_point_regions = true;

  const geoviz::Number scale_factor = ComputeScaleFactor(parameters, data.elements, data.necklaces);
  UNITTEST_CHECK_CLOSE(1.675, scale_factor, 0.001);
}

UNITTEST_TEST_FIXTURE(NecklaceDataWesternEurope, WestEuropeBuffer)
{
  const std::string in_value_name = "value";
  UNITTEST_CHECK(ReadValues(in_value_name));

  DefaultParameters(parameters);
  parameters.buffer_rad = 0.0349; // Roughly 2 degrees.

  const geoviz::Number scale_factor = ComputeScaleFactor(parameters, data.elements, data.necklaces);
  UNITTEST_CHECK_CLOSE(1.470, scale_factor, 0.001);
}

UNITTEST_TEST_FIXTURE(NecklaceDataWesternEurope, WestEuropeExact)
{
  const std::string in_value_name = "value";
  UNITTEST_CHECK(ReadValues(in_value_name));

  DefaultParameters(parameters);
  parameters.heuristic_cycles = 0;

  const geoviz::Number scale_factor = ComputeScaleFactor(parameters, data.elements, data.necklaces);
  UNITTEST_CHECK_CLOSE(1.675, scale_factor, 0.001);
}

UNITTEST_TEST_FIXTURE(NecklaceDataWesternEurope, WestEuropeExactBuffer)
{
  const std::string in_value_name = "value";
  UNITTEST_CHECK(ReadValues(in_value_name));

  DefaultParameters(parameters);
  parameters.buffer_rad = 0.0349; // Roughly 2 degrees.
  parameters.heuristic_cycles = 0;

  const geoviz::Number scale_factor = ComputeScaleFactor(parameters, data.elements, data.necklaces);
  UNITTEST_CHECK_CLOSE(1.470, scale_factor, 0.001);
}

UNITTEST_TEST_FIXTURE(NecklaceDataWesternEurope, WestEuropeSmaller)
{
  const std::string in_value_name = "test";
  UNITTEST_CHECK(ReadValues(in_value_name));

  DefaultParameters(parameters);

  const geoviz::Number scale_factor = ComputeScaleFactor(parameters, data.elements, data.necklaces);
  UNITTEST_CHECK_CLOSE(2.507, scale_factor, 0.001);
}


struct NecklaceDataEastAsia
{
  NecklaceDataEastAsia() :
    data(kEastAsia)
  {
    if (data.elements.empty())
    {
      kRegistry.Register(&data, "NecklaceDataEastAsia");

      // Read the geometry.
      geoviz::necklace_map::SvgReader svg_reader;
      const std::filesystem::path in_geometry_path = kDataDir / "eAsia.svg";

      geoviz::Timer time;
      UNITTEST_REQUIRE UNITTEST_CHECK(svg_reader.ReadFile(in_geometry_path, data.elements, data.necklaces));
      kRegistry(&data, 0) = time.Stamp();
    }
  }

  bool ReadValues(const std::string& in_value_name)
  {
    if (in_value_name == value_name)
      return true;
    value_name = in_value_name;

    geoviz::necklace_map::DataReader data_reader;
    const std::filesystem::path in_data_path = kDataDir / "eAsia.txt";

    geoviz::Timer time;
    const bool result = data_reader.ReadFile(in_data_path, in_value_name, data.elements);
    kRegistry(&data, 1) = time.Stamp();

    return result;
  }

  NecklaceData& data;
  std::string value_name;

  geoviz::necklace_map::Parameters parameters;
}; // struct NecklaceDataEastAsia

UNITTEST_TEST_FIXTURE(NecklaceDataEastAsia, EastAsiaAgriculture)
{
  const std::string in_value_name = "agriculture";
  UNITTEST_CHECK(ReadValues(in_value_name));

  DefaultParameters(parameters);

  const geoviz::Number scale_factor = ComputeScaleFactor(parameters, data.elements, data.necklaces);
  UNITTEST_CHECK_CLOSE(1.005, scale_factor, 0.001);
}

UNITTEST_TEST_FIXTURE(NecklaceDataEastAsia, EastAsiaExactAgriculture)
{
  const std::string in_value_name = "agriculture";
  UNITTEST_CHECK(ReadValues(in_value_name));

  DefaultParameters(parameters);
  parameters.heuristic_cycles = 0;

  const geoviz::Number scale_factor = ComputeScaleFactor(parameters, data.elements, data.necklaces);
  UNITTEST_CHECK_CLOSE(1.005, scale_factor, 0.001);
}

UNITTEST_TEST_FIXTURE(NecklaceDataEastAsia, EastAsiaPoverty)
{
  const std::string in_value_name = "poverty";
  UNITTEST_CHECK(ReadValues(in_value_name));

  DefaultParameters(parameters);

  const geoviz::Number scale_factor = ComputeScaleFactor(parameters, data.elements, data.necklaces);
  UNITTEST_CHECK_CLOSE(1.003, scale_factor, 0.001);
}

UNITTEST_TEST_FIXTURE(NecklaceDataEastAsia, EastAsiaExactPoverty)
{
  const std::string in_value_name = "poverty";
  UNITTEST_CHECK(ReadValues(in_value_name));

  DefaultParameters(parameters);
  parameters.heuristic_cycles = 0;

  const geoviz::Number scale_factor = ComputeScaleFactor(parameters, data.elements, data.necklaces);
  UNITTEST_CHECK_CLOSE(1.003, scale_factor, 0.001);
}

UNITTEST_TEST_FIXTURE(NecklaceDataEastAsia, EastAsiaInternet)
{
  const std::string in_value_name = "internet";
  UNITTEST_CHECK(ReadValues(in_value_name));

  DefaultParameters(parameters);

  const geoviz::Number scale_factor = ComputeScaleFactor(parameters, data.elements, data.necklaces);
  UNITTEST_CHECK_CLOSE(1.511, scale_factor, 0.001);
}

UNITTEST_TEST_FIXTURE(NecklaceDataEastAsia, EastAsiaExactInternet)
{
  const std::string in_value_name = "internet";
  UNITTEST_CHECK(ReadValues(in_value_name));

  DefaultParameters(parameters);
  parameters.heuristic_cycles = 0;

  const geoviz::Number scale_factor = ComputeScaleFactor(parameters, data.elements, data.necklaces);
  UNITTEST_CHECK_CLOSE(1.511, scale_factor, 0.001);
}

} // UNITTEST_SUITE(NecklaceMap)

#endif //GEOVIZ_TEST_NECKLACE_MAP_NECKLACE_MAP_H
