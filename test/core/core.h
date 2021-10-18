/*
The CartoCrow library implements algorithmic geo-visualization methods,
developed at TU Eindhoven.
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 16-11-2020
*/

#ifndef CARTOCROW_TEST_CORE_CORE_H
#define CARTOCROW_TEST_CORE_CORE_H

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>

#include <glog/logging.h>

#include <cartocrow/core/circulator.h>
#include <cartocrow/core/detail/polar_intersections.h>
#include <cartocrow/core/io/svg_writer.h>
#include <cartocrow/core/polar_line.h>
#include <cartocrow/core/polar_segment.h>
#include <cartocrow/core/spiral.h>
#include <cartocrow/core/spiral_segment.h>
#include <cartocrow/core/timer.h>
#include <cmake/cartocrow_test_config.h>

#include "test/test.h"
#include "test/test_registry_timer.h"

void TestCore() {} // Linking hack, each new test cpp file has it.

//constexpr const size_t kP = 2;
//constexpr const size_t kV = 2;

//using PT = PrintTimes<double, kP, kV>;
//using Reg = PT::R_;

//static Reg kRegistry;

static const std::filesystem::path kDataDir =
    std::filesystem::path(CARTOCROW_TEST_DATA_DIR) / "core";

UNITTEST_SUITE(Common) {

	UNITTEST_TEST(Circulator) {
		using Container = std::vector<int>;
		Container test = {0, 1, 2};
		const std::vector<int> expected = {0, 2, 0, 1, 1, 2, 1};

		{
			auto iter = cartocrow::make_circulator(test.begin(), test);
			const auto fixed_iter = iter;
			UNITTEST_CHECK_EQUAL(*iter, 0);
			UNITTEST_CHECK(fixed_iter == iter);

			Container::iterator ci = (Container::iterator) iter;
			UNITTEST_CHECK_EQUAL(*ci, 0);

			std::vector<int> results;

			results.push_back(*iter++); // 0
			results.push_back(*++iter); // 2
			results.push_back(*++iter); // 0 (cycled)
			results.push_back(*++iter); // 1
			results.push_back(*iter--); // 1
			results.push_back(*--iter); // 2 (cycled)
			results.push_back(*--iter); // 1

			UNITTEST_CHECK(fixed_iter != iter);
			UNITTEST_CHECK_EQUAL(expected.size(), results.size());
			UNITTEST_CHECK_ARRAY_EQUAL(expected, results, results.size());
		}

		{
			auto iter = cartocrow::make_circulator(test);
			const auto fixed_iter = iter;
			UNITTEST_CHECK_EQUAL(*iter, 0);
			UNITTEST_CHECK(fixed_iter == iter);

			Container::iterator ci = (Container::iterator) iter;
			UNITTEST_CHECK_EQUAL(*ci, 0);

			std::vector<int> results;

			results.push_back(*iter++); // 0
			results.push_back(*++iter); // 2
			results.push_back(*++iter); // 0 (cycled)
			results.push_back(*++iter); // 1
			results.push_back(*iter--); // 1
			results.push_back(*--iter); // 2 (cycled)
			results.push_back(*--iter); // 1

			UNITTEST_CHECK(fixed_iter != iter);
			UNITTEST_CHECK_EQUAL(expected.size(), results.size());
			UNITTEST_CHECK_ARRAY_EQUAL(expected, results, results.size());
		}
	}

	UNITTEST_TEST(StructCirculator) {
		struct MyStruct {
			MyStruct(const int& value) : value(value) {}
			operator int&() {
				return value;
			}
			int value;
		};

		using Container = std::vector<MyStruct>;
		Container test = {0, 1, 2};
		const std::vector<int> expected = {0, 2, 0, 1, 1, 2, 1};

		{
			auto iter = cartocrow::make_circulator(test.begin(), test);
			UNITTEST_CHECK_EQUAL(iter->value, 0);

			std::vector<int> results;

			results.push_back(*iter++); // 0
			results.push_back(*++iter); // 2
			results.push_back(*++iter); // 0 (cycled)
			results.push_back(*++iter); // 1
			results.push_back(*iter--); // 1
			results.push_back(*--iter); // 2 (cycled)
			results.push_back(*--iter); // 1

			UNITTEST_CHECK_EQUAL(expected.size(), results.size());
			UNITTEST_CHECK_ARRAY_EQUAL(expected, results, results.size());
		}
	}

	UNITTEST_TEST(ConstCirculator) {
		using Container = const std::vector<int>;
		Container test = {0, 1, 2};
		const std::vector<int> expected = {0, 2, 0, 1, 1, 2, 1};

		{
			auto iter = cartocrow::make_circulator(test.begin(), test);
			const auto fixed_iter = iter;
			UNITTEST_CHECK_EQUAL(*iter, 0);
			UNITTEST_CHECK(fixed_iter == iter);

			Container::const_iterator ci = (Container::const_iterator) iter;
			UNITTEST_CHECK_EQUAL(*ci, 0);

			std::vector<int> results;

			results.push_back(*iter++); // 0
			results.push_back(*++iter); // 2
			results.push_back(*++iter); // 0 (cycled)
			results.push_back(*++iter); // 1
			results.push_back(*iter--); // 1
			results.push_back(*--iter); // 2 (cycled)
			results.push_back(*--iter); // 1

			UNITTEST_CHECK(fixed_iter != iter);
			UNITTEST_CHECK_EQUAL(expected.size(), results.size());
			UNITTEST_CHECK_ARRAY_EQUAL(expected, results, results.size());
		}

		{
			auto iter = cartocrow::make_circulator(test);
			const auto fixed_iter = iter;
			UNITTEST_CHECK_EQUAL(*iter, 0);
			UNITTEST_CHECK(fixed_iter == iter);

			Container::const_iterator ci = (Container::const_iterator) iter;
			UNITTEST_CHECK_EQUAL(*ci, 0);

			std::vector<int> results;

			results.push_back(*iter++); // 0
			results.push_back(*++iter); // 2
			results.push_back(*++iter); // 0 (cycled)
			results.push_back(*++iter); // 1
			results.push_back(*iter--); // 1
			results.push_back(*--iter); // 2 (cycled)
			results.push_back(*--iter); // 1

			UNITTEST_CHECK(fixed_iter != iter);
			UNITTEST_CHECK_EQUAL(expected.size(), results.size());
			UNITTEST_CHECK_ARRAY_EQUAL(expected, results, results.size());
		}
	}

// Note, these are macros so the test output refers to the line number where the test is called.
#ifndef CHECK_POLAR_POINTS_CLOSE_R
#define CHECK_POLAR_POINTS_CLOSE_R(expected, actual, tolerance)                                    \
	UNITTEST_CHECK_CLOSE(expected, actual, tolerance);
#endif

#ifndef CHECK_POLAR_POINTS_CLOSE_PHI
#define CHECK_POLAR_POINTS_CLOSE_PHI(expected, actual, tolerance)                                  \
	UNITTEST_CHECK_CLOSE(0, cartocrow::Modulo(actual - expected, -M_PI), tolerance);
#endif

#ifndef CHECK_POLAR_POINTS_CLOSE
#define CHECK_POLAR_POINTS_CLOSE(expected, actual, tolerance)                                      \
	CHECK_POLAR_POINTS_CLOSE_R(expected.R(), actual.R(), tolerance);                               \
	if (tolerance < actual.R())                                                                    \
		CHECK_POLAR_POINTS_CLOSE_PHI(expected.phi(), actual.phi(), tolerance);
#endif

	UNITTEST_TEST(PolarStraightLines) {
		// Straight line and line segment that do not and do contain the pole.
		const cartocrow::PolarLine line(cartocrow::PolarPoint(cartocrow::Point(11, -2)),
		                                cartocrow::PolarPoint(cartocrow::Point(-1, 7)));
		const cartocrow::PolarLine line_pole(cartocrow::PolarPoint(cartocrow::Point(8, -6)),
		                                     cartocrow::PolarPoint(cartocrow::Point(-4, 3)));
		const cartocrow::PolarSegment segment(cartocrow::PolarPoint(cartocrow::Point(11, -2)),
		                                      cartocrow::PolarPoint(cartocrow::Point(-1, 7)));
		const cartocrow::PolarSegment segment_pole(cartocrow::PolarPoint(cartocrow::Point(8, -6)),
		                                           cartocrow::PolarPoint(cartocrow::Point(-4, 3)));

		// Straight line sub-segment that does not contain the point on the supporting line closest to the pole.
		const cartocrow::PolarSegment segment_farther(cartocrow::PolarPoint(cartocrow::Point(11, -2)),
		                                              cartocrow::PolarPoint(cartocrow::Point(7, 1)));

		////////////////////////////
		// Point closest to pole. //
		////////////////////////////

		const cartocrow::PolarPoint expected_close(cartocrow::Point(3, 4));
		const cartocrow::PolarPoint expected_close_pole(cartocrow::Point(0, 0));
		const cartocrow::PolarPoint expected_close_2(cartocrow::Point(7, 1));

		const cartocrow::PolarPoint closest_point_line = line.foot();
		const cartocrow::PolarPoint closest_point_line_pole = line_pole.foot();
		const cartocrow::PolarPoint closest_point_segment = segment.ComputeClosestToPole();
		const cartocrow::PolarPoint closest_point_segment_pole = segment_pole.ComputeClosestToPole();
		const cartocrow::PolarPoint closest_point_segment_farther =
		    segment_farther.ComputeClosestToPole();

		CHECK_POLAR_POINTS_CLOSE_R(
		    expected_close.R(), closest_point_line.R(),
		    0.001); // Note that the phi of the foot depends on the line's direction.
		CHECK_POLAR_POINTS_CLOSE(expected_close, closest_point_segment, 0.001);
		CHECK_POLAR_POINTS_CLOSE_R(
		    expected_close_pole.R(), closest_point_segment_pole.R(),
		    0.001); // Note that the phi of the foot depends on the line's direction.
		CHECK_POLAR_POINTS_CLOSE(expected_close_2, closest_point_segment_farther, 0.001);

		///////////////////////////////////////////////////
		// Whether a point at given distance is on line. //
		///////////////////////////////////////////////////

		// Note that the computations contain double precision errors in the order magnitude of ~10e-15.
		// This means that the mathematical closest point is not always on the line.
		const cartocrow::Number r_too_small = 1;
		const cartocrow::Number r_closest = 5.0000001;
		const cartocrow::Number r_2 = 6;
		const cartocrow::Number r_3 = 8;
		const cartocrow::Number r_4 = 14;
		const cartocrow::Number r_closest_pole = 0;
		const cartocrow::Number r_2_pole = 4;
		const cartocrow::Number r_3_pole = 6;
		const cartocrow::Number r_4_pole = 11;

		UNITTEST_CHECK_EQUAL(false, line.ContainsR(r_too_small));
		UNITTEST_CHECK_EQUAL(true, line.ContainsR(r_closest));
		UNITTEST_CHECK_EQUAL(true, line.ContainsR(r_2));
		UNITTEST_CHECK_EQUAL(true, line.ContainsR(r_3));
		UNITTEST_CHECK_EQUAL(true, line.ContainsR(r_4));

		UNITTEST_CHECK_EQUAL(false, segment.ContainsR(r_too_small));
		UNITTEST_CHECK_EQUAL(true, segment.ContainsR(r_closest));
		UNITTEST_CHECK_EQUAL(true, segment.ContainsR(r_2));
		UNITTEST_CHECK_EQUAL(true, segment.ContainsR(r_3));
		UNITTEST_CHECK_EQUAL(false, segment.ContainsR(r_4));

		// The mathematical closest point is not necessarily on the line in practice.
		//UNITTEST_CHECK_EQUAL(true, segment_pole.ContainsR(r_closest_pole));
		UNITTEST_CHECK_EQUAL(true, segment_pole.ContainsR(r_2_pole));
		UNITTEST_CHECK_EQUAL(true, segment_pole.ContainsR(r_3_pole));
		UNITTEST_CHECK_EQUAL(false, segment_pole.ContainsR(r_4_pole));

		UNITTEST_CHECK_EQUAL(false, segment_farther.ContainsR(r_too_small));
		UNITTEST_CHECK_EQUAL(false, segment_farther.ContainsR(r_closest));
		UNITTEST_CHECK_EQUAL(false, segment_farther.ContainsR(r_2));
		UNITTEST_CHECK_EQUAL(true, segment_farther.ContainsR(r_3));
		UNITTEST_CHECK_EQUAL(false, segment_farther.ContainsR(r_4));

		//////////////////////////////////////////////
		// Whether a point at given phi is on line. //
		//////////////////////////////////////////////

		//const cartocrow::PolarPoint expected_close(cartocrow::Point(3, 4));
		const cartocrow::PolarPoint on_line(cartocrow::Point(7, 1));
		const cartocrow::PolarPoint on_line_far(cartocrow::Point(-5, 10));
		const cartocrow::PolarPoint parallel(cartocrow::Point(4, -3));
		const cartocrow::PolarPoint off_line(cartocrow::Point(4, -4));

		UNITTEST_CHECK_EQUAL(true, line.ContainsPhi(expected_close.phi()));
		UNITTEST_CHECK_EQUAL(true, line.ContainsPhi(on_line.phi()));
		UNITTEST_CHECK_EQUAL(true, line.ContainsPhi(on_line_far.phi()));
		UNITTEST_CHECK_EQUAL(false, line.ContainsPhi(parallel.phi()));
		UNITTEST_CHECK_EQUAL(false, line.ContainsPhi(off_line.phi()));

		UNITTEST_CHECK_EQUAL(true, segment.ContainsPhi(expected_close.phi()));
		UNITTEST_CHECK_EQUAL(true, segment.ContainsPhi(on_line.phi()));
		UNITTEST_CHECK_EQUAL(false, segment.ContainsPhi(on_line_far.phi()));
		UNITTEST_CHECK_EQUAL(false, segment.ContainsPhi(parallel.phi()));
		UNITTEST_CHECK_EQUAL(false, segment.ContainsPhi(off_line.phi()));

		/////////////////////////////////////////////////////
		// Compute phi of point on line at given distance. //
		/////////////////////////////////////////////////////

		const cartocrow::PolarPoint expected_vertical(cartocrow::Point(0, 7 - (3 / 4.0)));
		const cartocrow::PolarPoint expected_smaller(cartocrow::Point(-4, 3));
		const cartocrow::PolarPoint expected_larger(cartocrow::Point(4, -3));

		int num;
		cartocrow::Number phi[2];

		num = line.CollectPhi(r_too_small, phi);
		UNITTEST_CHECK_EQUAL(0, num);

		// The mathematical closest point is not necessarily on the line in practice.
		num = line.CollectPhi(r_closest, phi);
		//  UNITTEST_CHECK_EQUAL(1, num);

		num = line.CollectPhi(r_2, phi);
		UNITTEST_CHECK_EQUAL(2, num);

		num = line.CollectPhi(r_4, phi);
		UNITTEST_CHECK_EQUAL(2, num);

		num = line.CollectPhi(expected_vertical.R(), phi);
		UNITTEST_CHECK_EQUAL(2, num);
		std::sort(phi, phi + 2);
		CHECK_POLAR_POINTS_CLOSE_PHI(M_PI_2, phi[1], 0.001);

		// The mathematical closest point is not necessarily on the line in practice.
		num = line_pole.CollectPhi(r_closest_pole, phi);
		//  UNITTEST_CHECK_EQUAL(1, num);

		num = line_pole.CollectPhi(r_2_pole, phi);
		UNITTEST_CHECK_EQUAL(2, num);
		std::sort(phi, phi + 2);
		CHECK_POLAR_POINTS_CLOSE_PHI(expected_smaller.phi(), phi[0], 0.001);
		CHECK_POLAR_POINTS_CLOSE_PHI(expected_larger.phi(), phi[1], 0.001);

		const cartocrow::PolarPoint sample_both_inside(cartocrow::Point(5, 2.5));
		const cartocrow::PolarPoint sample_other_inside(cartocrow::Point(1, 5.5));
		const cartocrow::PolarPoint sample_one_inside(cartocrow::Point(9, -0.5));
		const cartocrow::PolarPoint sample_both_outside(cartocrow::Point(15, -5));

		num = segment.CollectPhi(sample_both_inside.R(), phi);
		UNITTEST_CHECK_EQUAL(2, num);
		std::sort(phi, phi + 2);
		CHECK_POLAR_POINTS_CLOSE_PHI(sample_both_inside.phi(), phi[0], 0.001);
		CHECK_POLAR_POINTS_CLOSE_PHI(sample_other_inside.phi(), phi[1], 0.001);

		num = segment.CollectPhi(sample_one_inside.R(), phi);
		UNITTEST_CHECK_EQUAL(1, num);
		CHECK_POLAR_POINTS_CLOSE_PHI(sample_one_inside.phi(), phi[0], 0.001);

		num = segment.CollectPhi(sample_both_outside.R(), phi);
		UNITTEST_CHECK_EQUAL(0, num);

		const cartocrow::PolarPoint sample_both_inside_pole(cartocrow::Point(-2, 1.5));
		const cartocrow::PolarPoint sample_other_inside_pole(cartocrow::Point(2, -1.5));
		const cartocrow::PolarPoint sample_one_inside_pole(cartocrow::Point(6, -4.5));
		const cartocrow::PolarPoint sample_both_outside_pole(cartocrow::Point(12, -9));

		num = segment_pole.CollectPhi(sample_both_inside_pole.R(), phi);
		UNITTEST_CHECK_EQUAL(2, num);
		std::sort(phi, phi + 2);
		CHECK_POLAR_POINTS_CLOSE_PHI(sample_both_inside_pole.phi(), phi[0], 0.001);
		CHECK_POLAR_POINTS_CLOSE_PHI(sample_other_inside_pole.phi(), phi[1], 0.001);

		num = segment_pole.CollectPhi(sample_one_inside_pole.R(), phi);
		UNITTEST_CHECK_EQUAL(1, num);
		CHECK_POLAR_POINTS_CLOSE_PHI(sample_one_inside_pole.phi(), phi[0], 0.001);

		num = segment_pole.CollectPhi(sample_both_outside_pole.R(), phi);
		UNITTEST_CHECK_EQUAL(0, num);

		///////////////////////////////////////////
		// Compute line angle at given distance. //
		///////////////////////////////////////////

		const cartocrow::Number r_5 = std::sqrt(31.25);
		const cartocrow::Number r_6 = std::sqrt(50);
		const cartocrow::Number r_7 = std::sqrt(125);

		cartocrow::Number angle_rad;
		const cartocrow::Number expected_angle_closest = M_PI_2;
		const cartocrow::Number expected_angle_5 = std::atan2(5, 2.5);
		const cartocrow::Number expected_angle_6 = M_PI_4;
		const cartocrow::Number expected_angle_7 = std::atan2(5, 10);

		UNITTEST_CHECK_EQUAL(false, line.ComputeAngle(r_too_small, angle_rad));

		UNITTEST_CHECK_EQUAL(true, line.ComputeAngle(r_closest, angle_rad));
		CHECK_POLAR_POINTS_CLOSE_PHI(expected_angle_closest, angle_rad, 0.001);

		UNITTEST_CHECK_EQUAL(true, line.ComputeAngle(r_5, angle_rad));
		CHECK_POLAR_POINTS_CLOSE_PHI(expected_angle_5, angle_rad, 0.001);

		UNITTEST_CHECK_EQUAL(true, line.ComputeAngle(r_6, angle_rad));
		CHECK_POLAR_POINTS_CLOSE_PHI(expected_angle_6, angle_rad, 0.001);

		UNITTEST_CHECK_EQUAL(true, line.ComputeAngle(r_7, angle_rad));
		CHECK_POLAR_POINTS_CLOSE_PHI(expected_angle_7, angle_rad, 0.001);
	}

	UNITTEST_TEST(SpiralIntersections) {
		cartocrow::PolarPoint intersections[2];
		int num;

		const cartocrow::PolarLine line_1(cartocrow::PolarPoint(cartocrow::Point(11, -2)),
		                                  cartocrow::PolarPoint(cartocrow::Point(-1, 7)));
		const cartocrow::PolarLine line_2(cartocrow::PolarPoint(cartocrow::Point(-2, -4)),
		                                  cartocrow::PolarPoint(cartocrow::Point(1, 0)));
		const cartocrow::PolarLine line_3(cartocrow::PolarPoint(cartocrow::Point(4, -3)),
		                                  cartocrow::PolarPoint(cartocrow::Point(0, 0)));
		const cartocrow::PolarLine line_4(cartocrow::PolarPoint(cartocrow::Point(0, 0)),
		                                  cartocrow::PolarPoint(cartocrow::Point(4, 0)));
		const cartocrow::Spiral spiral_1(cartocrow::PolarPoint(cartocrow::Point(11, -3)),
		                                 M_PI * 3.0 / 8);
		const cartocrow::Spiral spiral_2(cartocrow::PolarPoint(cartocrow::Point(11, -3)),
		                                 -M_PI * 3.0 / 8);
		const cartocrow::Spiral spiral_3(cartocrow::PolarPoint(cartocrow::Point(2, 3)), M_PI_4);
		const cartocrow::Spiral spiral_4(cartocrow::PolarPoint(cartocrow::Point(-11, 3)),
		                                 M_PI * 3.0 / 8);
		const cartocrow::Spiral spiral_5(cartocrow::PolarPoint(cartocrow::Point(4, -3)), 0);

		const cartocrow::PolarSegment line_segment_1(cartocrow::PolarPoint(cartocrow::Point(11, -2)),
		                                             cartocrow::PolarPoint(cartocrow::Point(-1, 7)));
		const cartocrow::PolarSegment line_segment_2(cartocrow::PolarPoint(cartocrow::Point(11, -2)),
		                                             cartocrow::PolarPoint(cartocrow::Point(7, 1)));
		const cartocrow::SpiralSegment spiral_segment_1(
		    cartocrow::PolarPoint(cartocrow::Point(5, 5)), M_PI * 3.0 / 8, 0, 15);
		const cartocrow::SpiralSegment spiral_segment_2(
		    cartocrow::PolarPoint(cartocrow::Point(5, 5)), M_PI * 3.0 / 8, 0, 10);
		const cartocrow::SpiralSegment spiral_segment_3(
		    cartocrow::PolarPoint(cartocrow::Point(5, 5)), M_PI * 3.0 / 8, 6, 10);

		cartocrow::PolarPoint expected_intersection_line_1_line_2(5.0634, 0.7686);
		cartocrow::PolarPoint expected_intersection_line_3_line_4(0, 0);
		cartocrow::PolarPoint expected_intersection_spiral_1_spiral_2_0(3.1033, 2.8753);
		cartocrow::PolarPoint expected_intersection_spiral_1_spiral_2_1(11.4018, -0.2663);
		cartocrow::PolarPoint expected_intersection_spiral_2_spiral_3_0(1.8628, 1.6432);
		cartocrow::PolarPoint expected_intersection_spiral_2_spiral_3_1(11.7329, -0.1971);
		cartocrow::PolarPoint expected_intersection_line_1_spiral_1_0(51.0082, 2.3999);
		cartocrow::PolarPoint expected_intersection_line_1_spiral_1_1(10.9538, -0.1695);
		cartocrow::PolarPoint expected_intersection_line_2_spiral_3_0(4.5484, 0.7505);
		cartocrow::PolarPoint expected_intersection_line_2_spiral_5_0(0.8000, -0.6435);
		cartocrow::PolarPoint expected_intersection_line_3_spiral_1_0(13.3302, -0.6435);
		cartocrow::PolarPoint expected_intersection_line_3_spiral_1_1(3.6282, 2.4981);
		cartocrow::PolarPoint expected_intersection_line_3_spiral_5_0(0, 0);
		cartocrow::PolarPoint expected_intersection_line_3_spiral_5_1(5, -0.6435);
		cartocrow::PolarPoint expected_intersection_line_4_spiral_5_0(0, 0);

		// TODO(tvl) TMP DEBUG
		cartocrow::SvgWriter writer;
		writer.Add(line_1);
		writer.Add(line_2);
		writer.Add(line_3);
		writer.Add(line_4);
		writer.Add(spiral_1);
		writer.Add(spiral_2);
		writer.Add(spiral_3);
		writer.Add(spiral_4);
		writer.Add(spiral_5);
		writer.Add(line_segment_1);
		writer.Add(line_segment_2);
		writer.Add(spiral_segment_1);
		writer.Add(spiral_segment_2);
		writer.Add(spiral_segment_3);

		writer.Add(expected_intersection_line_1_line_2);
		writer.Add(expected_intersection_line_3_line_4);
		writer.Add(expected_intersection_spiral_1_spiral_2_0);
		writer.Add(expected_intersection_spiral_1_spiral_2_1);
		writer.Add(expected_intersection_spiral_2_spiral_3_0);
		writer.Add(expected_intersection_spiral_2_spiral_3_1);
		writer.Add(expected_intersection_line_1_spiral_1_0);
		writer.Add(expected_intersection_line_1_spiral_1_1);
		writer.Add(expected_intersection_line_2_spiral_3_0);
		writer.Add(expected_intersection_line_2_spiral_5_0);
		writer.Add(expected_intersection_line_3_spiral_1_0);
		writer.Add(expected_intersection_line_3_spiral_1_1);
		writer.Add(expected_intersection_line_3_spiral_5_0);
		writer.Add(expected_intersection_line_3_spiral_5_1);
		writer.Add(expected_intersection_line_4_spiral_5_0);

		cartocrow::WriteOptions::Ptr options = cartocrow::WriteOptions::Default();
		std::ofstream out("/storage/CartoCrow/wwwroot/data/tmp/core_out.svg");
		writer.Write(options, out);
		out.close();

		// Line - line.
		num = cartocrow::ComputeIntersections(line_1, line_2, intersections);
		UNITTEST_CHECK_EQUAL(1, num);
		CHECK_POLAR_POINTS_CLOSE(expected_intersection_line_1_line_2, intersections[0], 0.001);

		// Line - line (parallel).
		num = cartocrow::ComputeIntersections(line_1, line_3, intersections);
		UNITTEST_CHECK_EQUAL(0, num);

		// Line - line (pole).
		num = cartocrow::ComputeIntersections(line_3, line_4, intersections);
		UNITTEST_CHECK_EQUAL(1, num);
		CHECK_POLAR_POINTS_CLOSE(expected_intersection_line_3_line_4, intersections[0], 0.001);

		// Spiral - spiral (opposite angle).
		num = cartocrow::ComputeIntersections(spiral_1, spiral_2, intersections);
		UNITTEST_CHECK_EQUAL(2, num);
		CHECK_POLAR_POINTS_CLOSE(expected_intersection_spiral_1_spiral_2_0, intersections[0], 0.001);
		CHECK_POLAR_POINTS_CLOSE(expected_intersection_spiral_1_spiral_2_1, intersections[1], 0.001);

		// Spiral - spiral.
		num = cartocrow::ComputeIntersections(spiral_2, spiral_3, intersections);
		UNITTEST_CHECK_EQUAL(2, num);
		CHECK_POLAR_POINTS_CLOSE(expected_intersection_spiral_2_spiral_3_0, intersections[0], 0.001);
		CHECK_POLAR_POINTS_CLOSE(expected_intersection_spiral_2_spiral_3_1, intersections[1], 0.001);

		// Spiral - spiral (equal angle).
		num = cartocrow::ComputeIntersections(spiral_1, spiral_4, intersections);
		UNITTEST_CHECK_EQUAL(0, num);

		// Line - spiral.
		num = cartocrow::ComputeIntersections(line_1, spiral_1, intersections);
		UNITTEST_CHECK_EQUAL(2, num);
		CHECK_POLAR_POINTS_CLOSE(expected_intersection_line_1_spiral_1_0, intersections[0], 0.001);
		CHECK_POLAR_POINTS_CLOSE(expected_intersection_line_1_spiral_1_1, intersections[1], 0.001);

		// Line - spiral (one side).
		num = cartocrow::ComputeIntersections(line_2, spiral_3, intersections);
		UNITTEST_CHECK_EQUAL(1, num);
		CHECK_POLAR_POINTS_CLOSE(expected_intersection_line_2_spiral_3_0, intersections[0], 0.001);

		// Line (through pole) - spiral.
		num = cartocrow::ComputeIntersections(spiral_1, line_3, intersections);
		UNITTEST_CHECK_EQUAL(2, num);
		CHECK_POLAR_POINTS_CLOSE(expected_intersection_line_3_spiral_1_0, intersections[0], 0.001);
		CHECK_POLAR_POINTS_CLOSE(expected_intersection_line_3_spiral_1_1, intersections[1], 0.001);

		// Line - spiral (straight).
		num = cartocrow::ComputeIntersections(spiral_5, line_2, intersections);
		UNITTEST_CHECK_EQUAL(1, num);
		CHECK_POLAR_POINTS_CLOSE(expected_intersection_line_2_spiral_5_0, intersections[0], 0.001);

		// Line (parallel) - spiral (straight).
		num = cartocrow::ComputeIntersections(spiral_5, line_1, intersections);
		UNITTEST_CHECK_EQUAL(0, num);

		// Line (through pole) - spiral (straight).
		num = cartocrow::ComputeIntersections(spiral_5, line_4, intersections);
		UNITTEST_CHECK_EQUAL(1, num);
		CHECK_POLAR_POINTS_CLOSE(expected_intersection_line_4_spiral_5_0, intersections[0], 0.001);

		// Line (parallel through pole) - spiral (straight).
		num = cartocrow::ComputeIntersections(spiral_5, line_3, intersections);
		UNITTEST_CHECK_EQUAL(2, num);
		CHECK_POLAR_POINTS_CLOSE(expected_intersection_line_3_spiral_5_0, intersections[0], 0.001);
		CHECK_POLAR_POINTS_CLOSE(expected_intersection_line_3_spiral_5_1, intersections[1], 0.001);

		// Line - line segment.
		num = cartocrow::ComputeIntersections(line_2, line_segment_1, intersections);
		UNITTEST_CHECK_EQUAL(1, num);

		num = cartocrow::ComputeIntersections(line_2, line_segment_2, intersections);
		UNITTEST_CHECK_EQUAL(0, num);

		// Line - spiral segment.
		num = cartocrow::ComputeIntersections(line_1, spiral_segment_1, intersections);
		UNITTEST_CHECK_EQUAL(2, num);

		num = cartocrow::ComputeIntersections(line_1, spiral_segment_2, intersections);
		UNITTEST_CHECK_EQUAL(1, num);

		num = cartocrow::ComputeIntersections(line_1, spiral_segment_3, intersections);
		UNITTEST_CHECK_EQUAL(0, num);

		// Line segment - spiral segment.
		num = cartocrow::ComputeIntersections(line_segment_2, spiral_segment_2, intersections);
		UNITTEST_CHECK_EQUAL(0, num);
	}

} // UNITTEST_SUITE(Common)

#endif //CARTOCROW_TEST_CORE_CORE_H
