/*
The Flow Map library implements the algorithmic geo-visualization
method by the same name, developed by Kevin Verbeek, Kevin Buchin,
and Bettina Speckmann at TU Eindhoven
(DOI: 10.1007/s00453-013-9867-z & 10.1109/TVCG.2011.202).
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

#ifndef CARTOCROW_TEST_FLOW_MAP_FLOW_MAP_H
#define CARTOCROW_TEST_FLOW_MAP_FLOW_MAP_H

#include <filesystem>
#include <iostream>

#include <glog/logging.h>

#include <cartocrow/core/timer.h>
#include <cartocrow/flow_map/flow_map.h>
#include <cmake/cartocrow_test_config.h>

#include "test/test.h"
#include "test/test_registry_timer.h"

void TestFlowMap() {} // Linking hack, each new test cpp file has it.

constexpr const size_t kP = 2;
constexpr const size_t kV = 2;

using PT = PrintTimes<double, kP, kV>;
using Reg = PT::R_;

static Reg kRegistry;

struct FlowData {
	FlowData() {
		// Disable logging to INFO and WARNING.
		FLAGS_minloglevel = 2;
	}

	std::vector<cartocrow::Region> context;
	std::vector<cartocrow::flow_map::Place::Ptr> places;
	size_t index_root;
}; // struct FlowData

static const std::filesystem::path kDataDir =
    std::filesystem::path(CARTOCROW_TEST_DATA_DIR) / "flow_map";

static FlowData kUsa = FlowData();
static FlowData kWorld = FlowData();

UNITTEST_SUITE(FlowMap) {

	void DefaultParameters(cartocrow::flow_map::Parameters & parameters) {
		//  parameters.interval_type = cartocrow::necklace_map::IntervalType::kWedge;
		//  parameters.centroid_interval_length_rad = 0.2 * M_PI;
		//  parameters.ignore_point_regions = false;
		//
		//  parameters.order_type = cartocrow::necklace_map::OrderType::kAny;
		//  parameters.buffer_rad = 0;
		//  parameters.aversion_ratio = 0.001;
	}

	struct FlowDataUsa {
		FlowDataUsa() : data(kUsa) {
			if (data.places.empty()) {
				kRegistry.Register(&data, "FlowDataUsa");

				// Read the geometry.
				cartocrow::flow_map::SvgReader svg_reader;
				const std::filesystem::path in_geometry_path = kDataDir / "USA.svg";

				cartocrow::Timer time;
				UNITTEST_REQUIRE UNITTEST_CHECK(
				    svg_reader.ReadFile(in_geometry_path, data.context, data.places));
				kRegistry(&data, 0) = time.Stamp();
			}
		}

		bool ReadValues(const std::string& in_value_name) {
			if (in_value_name == value_name) {
				return true;
			}
			value_name = in_value_name;

			cartocrow::flow_map::DataReader data_reader;
			const std::filesystem::path in_data_path = kDataDir / "USA.csv";

			cartocrow::Timer time;
			const bool result =
			    data_reader.ReadFile(in_data_path, in_value_name, data.places, data.index_root);
			kRegistry(&data, 1) = time.Stamp();

			return result;
		}

		FlowData& data;
		std::string value_name;

		cartocrow::flow_map::Parameters parameters;
	}; // struct FlowDataUsa

	UNITTEST_TEST_FIXTURE(FlowDataUsa, UsaGreedy) {
		const std::string in_value_name = "CA";
		UNITTEST_CHECK(ReadValues(in_value_name));

		DefaultParameters(parameters);
		//  parameters.interval_type = cartocrow::necklace_map::IntervalType::kCentroid;
		//  parameters.order_type = cartocrow::necklace_map::OrderType::kFixed;

		//  const cartocrow::Number scale_factor = ComputeScaleFactor(parameters, data.elements, data.necklaces);
		//  UNITTEST_CHECK_CLOSE(1.580, scale_factor, 0.001);
	}

	struct FlowDataWorld {
		FlowDataWorld() : data(kWorld) {
			if (data.places.empty()) {
				kRegistry.Register(&data, "FlowDataWorld");

				// Read the geometry.
				cartocrow::flow_map::SvgReader svg_reader;
				const std::filesystem::path in_geometry_path = kDataDir / "World.svg";

				cartocrow::Timer time;
				UNITTEST_REQUIRE UNITTEST_CHECK(
				    svg_reader.ReadFile(in_geometry_path, data.context, data.places));
				kRegistry(&data, 0) = time.Stamp();
			}
		}

		bool ReadValues(const std::string& in_value_name) {
			if (in_value_name == value_name) {
				return true;
			}
			value_name = in_value_name;

			cartocrow::flow_map::DataReader data_reader;
			const std::filesystem::path in_data_path = kDataDir / "World.csv";

			cartocrow::Timer time;
			const bool result =
			    data_reader.ReadFile(in_data_path, in_value_name, data.places, data.index_root);
			kRegistry(&data, 1) = time.Stamp();

			return result;
		}

		FlowData& data;
		std::string value_name;

		cartocrow::flow_map::Parameters parameters;
	}; // struct FlowDataWorld

	UNITTEST_TEST_FIXTURE(FlowDataWorld, EastAsiaAgriculture) {
		//  const std::string in_value_name = "Karstner";
		//  UNITTEST_CHECK(ReadValues(in_value_name));
		//
		//  DefaultParameters(parameters);

		//  const cartocrow::Number scale_factor = ComputeScaleFactor(parameters, data.elements, data.necklaces);
		//  UNITTEST_CHECK_CLOSE(1.005, scale_factor, 0.001);
	}

} // UNITTEST_SUITE(FlowMap)

#endif //CARTOCROW_TEST_FLOW_MAP_FLOW_MAP_H
