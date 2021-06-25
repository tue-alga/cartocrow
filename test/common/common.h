/*
The GeoViz library implements algorithmic geo-visualization methods,
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

#ifndef GEOVIZ_TEST_COMMON_COMMON_H
#define GEOVIZ_TEST_COMMON_COMMON_H

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>

#include <glog/logging.h>

#include <cmake/geoviz_test_config.h>
#include <geoviz/common/circulator.h>
#include <geoviz/common/polar_line.h>
#include <geoviz/common/polar_segment.h>
#include <geoviz/common/spiral.h>
#include <geoviz/common/spiral_segment.h>
#include <geoviz/common/timer.h>
#include <geoviz/common/detail/polar_intersections.h>
#include <geoviz/common/io/svg_writer.h>

#include "test/test.h"
#include "test/test_registry_timer.h"


void TestCommon() {}  // Linking hack, each new test cpp file has it.

//constexpr const size_t kP = 2;
//constexpr const size_t kV = 2;

//using PT = PrintTimes<double, kP, kV>;
//using Reg = PT::R_;

//static Reg kRegistry;


static const std::filesystem::path kDataDir = filesystem::path(GEOVIZ_TEST_DATA_DIR) / "common";


UNITTEST_SUITE(Common)
{

UNITTEST_TEST(Circulator)
{
  using Container = std::vector<int>;
  Container test = {0, 1, 2};
  const std::vector<int> expected = {0, 2, 0, 1, 1, 2, 1};

  {
    auto iter = geoviz::make_circulator(test.begin(), test);
    const auto fixed_iter = iter;
    UNITTEST_CHECK_EQUAL(*iter, 0);
    UNITTEST_CHECK(fixed_iter == iter);

    Container::iterator ci = (Container::iterator)iter;
    UNITTEST_CHECK_EQUAL(*ci, 0);

    std::vector<int> results;

    results.push_back(*iter++);  // 0
    results.push_back(*++iter);  // 2
    results.push_back(*++iter);  // 0 (cycled)
    results.push_back(*++iter);  // 1
    results.push_back(*iter--);  // 1
    results.push_back(*--iter);  // 2 (cycled)
    results.push_back(*--iter);  // 1

    UNITTEST_CHECK(fixed_iter != iter);
    UNITTEST_CHECK_EQUAL(expected.size(), results.size());
    UNITTEST_CHECK_ARRAY_EQUAL(expected, results, results.size());
  }

  {
    auto iter = geoviz::make_circulator(test);
    const auto fixed_iter = iter;
    UNITTEST_CHECK_EQUAL(*iter, 0);
    UNITTEST_CHECK(fixed_iter == iter);

    Container::iterator ci = (Container::iterator)iter;
    UNITTEST_CHECK_EQUAL(*ci, 0);

    std::vector<int> results;

    results.push_back(*iter++);  // 0
    results.push_back(*++iter);  // 2
    results.push_back(*++iter);  // 0 (cycled)
    results.push_back(*++iter);  // 1
    results.push_back(*iter--);  // 1
    results.push_back(*--iter);  // 2 (cycled)
    results.push_back(*--iter);  // 1

    UNITTEST_CHECK(fixed_iter != iter);
    UNITTEST_CHECK_EQUAL(expected.size(), results.size());
    UNITTEST_CHECK_ARRAY_EQUAL(expected, results, results.size());
  }
}

UNITTEST_TEST(StructCirculator)
{
  struct MyStruct { MyStruct(const int& value) : value(value) {} operator int&() { return value; } int value; };

  using Container = std::vector<MyStruct>;
  Container test = {0, 1, 2};
  const std::vector<int> expected = {0, 2, 0, 1, 1, 2, 1};

  {
    auto iter = geoviz::make_circulator(test.begin(), test);
    UNITTEST_CHECK_EQUAL(iter->value, 0);

    std::vector<int> results;

    results.push_back(*iter++);  // 0
    results.push_back(*++iter);  // 2
    results.push_back(*++iter);  // 0 (cycled)
    results.push_back(*++iter);  // 1
    results.push_back(*iter--);  // 1
    results.push_back(*--iter);  // 2 (cycled)
    results.push_back(*--iter);  // 1

    UNITTEST_CHECK_EQUAL(expected.size(), results.size());
    UNITTEST_CHECK_ARRAY_EQUAL(expected, results, results.size());
  }
}

UNITTEST_TEST(ConstCirculator)
{
  using Container = const std::vector<int>;
  Container test = {0, 1, 2};
  const std::vector<int> expected = {0, 2, 0, 1, 1, 2, 1};

  {
    auto iter = geoviz::make_circulator(test.begin(), test);
    const auto fixed_iter = iter;
    UNITTEST_CHECK_EQUAL(*iter, 0);
    UNITTEST_CHECK(fixed_iter == iter);

    Container::const_iterator ci = (Container::const_iterator)iter;
    UNITTEST_CHECK_EQUAL(*ci, 0);

    std::vector<int> results;

    results.push_back(*iter++);  // 0
    results.push_back(*++iter);  // 2
    results.push_back(*++iter);  // 0 (cycled)
    results.push_back(*++iter);  // 1
    results.push_back(*iter--);  // 1
    results.push_back(*--iter);  // 2 (cycled)
    results.push_back(*--iter);  // 1

    UNITTEST_CHECK(fixed_iter != iter);
    UNITTEST_CHECK_EQUAL(expected.size(), results.size());
    UNITTEST_CHECK_ARRAY_EQUAL(expected, results, results.size());
  }

  {
    auto iter = geoviz::make_circulator(test);
    const auto fixed_iter = iter;
    UNITTEST_CHECK_EQUAL(*iter, 0);
    UNITTEST_CHECK(fixed_iter == iter);

    Container::const_iterator ci = (Container::const_iterator)iter;
    UNITTEST_CHECK_EQUAL(*ci, 0);

    std::vector<int> results;

    results.push_back(*iter++);  // 0
    results.push_back(*++iter);  // 2
    results.push_back(*++iter);  // 0 (cycled)
    results.push_back(*++iter);  // 1
    results.push_back(*iter--);  // 1
    results.push_back(*--iter);  // 2 (cycled)
    results.push_back(*--iter);  // 1

    UNITTEST_CHECK(fixed_iter != iter);
    UNITTEST_CHECK_EQUAL(expected.size(), results.size());
    UNITTEST_CHECK_ARRAY_EQUAL(expected, results, results.size());
  }
}

// Note, these are macros so the test output refers to the line number where the test is called.
#ifndef CHECK_POLAR_POINTS_CLOSE_R
#define CHECK_POLAR_POINTS_CLOSE_R(expected, actual, tolerance) \
UNITTEST_CHECK_CLOSE(expected, actual, tolerance);
#endif

#ifndef CHECK_POLAR_POINTS_CLOSE_PHI
#define CHECK_POLAR_POINTS_CLOSE_PHI(expected, actual, tolerance) \
UNITTEST_CHECK_CLOSE(0, geoviz::Modulo(actual - expected, -M_PI), tolerance);
#endif

#ifndef CHECK_POLAR_POINTS_CLOSE
#define CHECK_POLAR_POINTS_CLOSE(expected, actual, tolerance) \
CHECK_POLAR_POINTS_CLOSE_R(expected.R(), actual.R(), tolerance); \
if (tolerance < actual.R()) \
CHECK_POLAR_POINTS_CLOSE_PHI(expected.phi(), actual.phi(), tolerance);
#endif

UNITTEST_TEST(PolarStraightLines)
{
  // Straight line and line segment that do not and do contain the pole.
  const geoviz::PolarLine line
  (
    geoviz::PolarPoint(geoviz::Point(11, -2)),
    geoviz::PolarPoint(geoviz::Point(-1, 7))
  );
  const geoviz::PolarLine line_pole
  (
    geoviz::PolarPoint(geoviz::Point(8, -6)),
    geoviz::PolarPoint(geoviz::Point(-4, 3))
  );
  const geoviz::PolarSegment segment
  (
    geoviz::PolarPoint(geoviz::Point(11, -2)),
    geoviz::PolarPoint(geoviz::Point(-1, 7))
  );
  const geoviz::PolarSegment segment_pole
  (
    geoviz::PolarPoint(geoviz::Point(8, -6)),
    geoviz::PolarPoint(geoviz::Point(-4, 3))
  );

  // Straight line sub-segment that does not contain the point on the supporting line closest to the pole.
  const geoviz::PolarSegment segment_farther
  (
    geoviz::PolarPoint(geoviz::Point(11, -2)),
    geoviz::PolarPoint(geoviz::Point(7, 1))
  );

  ////////////////////////////
  // Point closest to pole. //
  ////////////////////////////

  const geoviz::PolarPoint expected_close(geoviz::Point(3, 4));
  const geoviz::PolarPoint expected_close_pole(geoviz::Point(0, 0));
  const geoviz::PolarPoint expected_close_2(geoviz::Point(7, 1));

  const geoviz::PolarPoint closest_point_line = line.foot();
  const geoviz::PolarPoint closest_point_line_pole = line_pole.foot();
  const geoviz::PolarPoint closest_point_segment = segment.ComputeClosestToPole();
  const geoviz::PolarPoint closest_point_segment_pole = segment_pole.ComputeClosestToPole();
  const geoviz::PolarPoint closest_point_segment_farther = segment_farther.ComputeClosestToPole();

  CHECK_POLAR_POINTS_CLOSE_R(expected_close.R(), closest_point_line.R(), 0.001);  // Note that the phi of the foot depends on the line's direction.
  CHECK_POLAR_POINTS_CLOSE(expected_close, closest_point_segment, 0.001);
  CHECK_POLAR_POINTS_CLOSE_R(expected_close_pole.R(), closest_point_segment_pole.R(), 0.001);  // Note that the phi of the foot depends on the line's direction.
  CHECK_POLAR_POINTS_CLOSE(expected_close_2, closest_point_segment_farther, 0.001);


  ///////////////////////////////////////////////////
  // Whether a point at given distance is on line. //
  ///////////////////////////////////////////////////

  // Note that the computations contain double precision errors in the order magnitude of ~10e-15.
  // This means that the mathematical closest point is not always on the line.
  const geoviz::Number r_too_small = 1;
  const geoviz::Number r_closest = 5.0000001;
  const geoviz::Number r_2 = 6;
  const geoviz::Number r_3 = 8;
  const geoviz::Number r_4 = 14;
  const geoviz::Number r_closest_pole = 0;
  const geoviz::Number r_2_pole = 4;
  const geoviz::Number r_3_pole = 6;
  const geoviz::Number r_4_pole = 11;

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

  //const geoviz::PolarPoint expected_close(geoviz::Point(3, 4));
  const geoviz::PolarPoint on_line(geoviz::Point(7, 1));
  const geoviz::PolarPoint on_line_far(geoviz::Point(-5, 10));
  const geoviz::PolarPoint parallel(geoviz::Point(4, -3));
  const geoviz::PolarPoint off_line(geoviz::Point(4, -4));

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

  const geoviz::PolarPoint expected_vertical(geoviz::Point(0, 7 - (3 / 4.0)));
  const geoviz::PolarPoint expected_smaller(geoviz::Point(-4, 3));
  const geoviz::PolarPoint expected_larger(geoviz::Point(4, -3));

  int num;
  geoviz::Number phi[2];

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

  const geoviz::PolarPoint sample_both_inside(geoviz::Point(5, 2.5));
  const geoviz::PolarPoint sample_other_inside(geoviz::Point(1, 5.5));
  const geoviz::PolarPoint sample_one_inside(geoviz::Point(9, -0.5));
  const geoviz::PolarPoint sample_both_outside(geoviz::Point(15, -5));

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

  const geoviz::PolarPoint sample_both_inside_pole(geoviz::Point(-2, 1.5));
  const geoviz::PolarPoint sample_other_inside_pole(geoviz::Point(2, -1.5));
  const geoviz::PolarPoint sample_one_inside_pole(geoviz::Point(6, -4.5));
  const geoviz::PolarPoint sample_both_outside_pole(geoviz::Point(12, -9));

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

  const geoviz::Number r_5 = std::sqrt(31.25);
  const geoviz::Number r_6 = std::sqrt(50);
  const geoviz::Number r_7 = std::sqrt(125);

  geoviz::Number angle_rad;
  const geoviz::Number expected_angle_closest = M_PI_2;
  const geoviz::Number expected_angle_5 = std::atan2(5, 2.5);
  const geoviz::Number expected_angle_6 = M_PI_4;
  const geoviz::Number expected_angle_7 = std::atan2(5, 10);

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

UNITTEST_TEST(SpiralIntersections)
{
  geoviz::PolarPoint intersections[2];
  int num;

  const geoviz::PolarLine line_1(geoviz::PolarPoint(geoviz::Point(11, -2)), geoviz::PolarPoint(geoviz::Point(-1, 7)));
  const geoviz::PolarLine line_2(geoviz::PolarPoint(geoviz::Point(-2, -4)), geoviz::PolarPoint(geoviz::Point(1, 0)));
  const geoviz::PolarLine line_3(geoviz::PolarPoint(geoviz::Point(4, -3)), geoviz::PolarPoint(geoviz::Point(0, 0)));
  const geoviz::PolarLine line_4(geoviz::PolarPoint(geoviz::Point(0, 0)), geoviz::PolarPoint(geoviz::Point(4, 0)));
  const geoviz::Spiral spiral_1(geoviz::PolarPoint(geoviz::Point(11, -3)), M_PI * 3.0 / 8);
  const geoviz::Spiral spiral_2(geoviz::PolarPoint(geoviz::Point(11, -3)), -M_PI * 3.0 / 8);
  const geoviz::Spiral spiral_3(geoviz::PolarPoint(geoviz::Point(2, 3)), M_PI_4);
  const geoviz::Spiral spiral_4(geoviz::PolarPoint(geoviz::Point(-11, 3)), M_PI * 3.0 / 8);
  const geoviz::Spiral spiral_5(geoviz::PolarPoint(geoviz::Point(4, -3)), 0);

  const geoviz::PolarSegment line_segment_1(geoviz::PolarPoint(geoviz::Point(11, -2)), geoviz::PolarPoint(geoviz::Point(-1, 7)));
  const geoviz::PolarSegment line_segment_2(geoviz::PolarPoint(geoviz::Point(11, -2)), geoviz::PolarPoint(geoviz::Point(7, 1)));
  const geoviz::SpiralSegment spiral_segment_1(geoviz::PolarPoint(geoviz::Point(5, 5)), M_PI * 3.0 / 8, 0, 15);
  const geoviz::SpiralSegment spiral_segment_2(geoviz::PolarPoint(geoviz::Point(5, 5)), M_PI * 3.0 / 8, 0, 10);
  const geoviz::SpiralSegment spiral_segment_3(geoviz::PolarPoint(geoviz::Point(5, 5)), M_PI * 3.0 / 8, 6, 10);

  geoviz::PolarPoint expected_intersection_line_1_line_2(5.0634, 0.7686);
  geoviz::PolarPoint expected_intersection_line_3_line_4(0, 0);
  geoviz::PolarPoint expected_intersection_spiral_1_spiral_2_0(3.1033, 2.8753);
  geoviz::PolarPoint expected_intersection_spiral_1_spiral_2_1(11.4018, -0.2663);
  geoviz::PolarPoint expected_intersection_spiral_2_spiral_3_0(1.8628, 1.6432);
  geoviz::PolarPoint expected_intersection_spiral_2_spiral_3_1(11.7329, -0.1971);
  geoviz::PolarPoint expected_intersection_line_1_spiral_1_0(51.0082, 2.3999);
  geoviz::PolarPoint expected_intersection_line_1_spiral_1_1(10.9538, -0.1695);
  geoviz::PolarPoint expected_intersection_line_2_spiral_3_0(4.5484, 0.7505);
  geoviz::PolarPoint expected_intersection_line_2_spiral_5_0(0.8000, -0.6435);
  geoviz::PolarPoint expected_intersection_line_3_spiral_1_0(13.3302, -0.6435);
  geoviz::PolarPoint expected_intersection_line_3_spiral_1_1(3.6282, 2.4981);
  geoviz::PolarPoint expected_intersection_line_3_spiral_5_0(0, 0);
  geoviz::PolarPoint expected_intersection_line_3_spiral_5_1(5, -0.6435);
  geoviz::PolarPoint expected_intersection_line_4_spiral_5_0(0, 0);


  // TODO(tvl) TMP DEBUG
  geoviz::SvgWriter writer;
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

  geoviz::WriteOptions::Ptr options = geoviz::WriteOptions::Default();
  std::ofstream out("/storage/GeoViz/wwwroot/data/tmp/common_out.svg");
  writer.Write(options, out);
  out.close();

  // Line - line.
  num = geoviz::ComputeIntersections(line_1, line_2, intersections);
  UNITTEST_CHECK_EQUAL(1, num);
  CHECK_POLAR_POINTS_CLOSE(expected_intersection_line_1_line_2, intersections[0], 0.001);

  // Line - line (parallel).
  num = geoviz::ComputeIntersections(line_1, line_3, intersections);
  UNITTEST_CHECK_EQUAL(0, num);

  // Line - line (pole).
  num = geoviz::ComputeIntersections(line_3, line_4, intersections);
  UNITTEST_CHECK_EQUAL(1, num);
  CHECK_POLAR_POINTS_CLOSE(expected_intersection_line_3_line_4, intersections[0], 0.001);

  // Spiral - spiral (opposite angle).
  num = geoviz::ComputeIntersections(spiral_1, spiral_2, intersections);
  UNITTEST_CHECK_EQUAL(2, num);
  CHECK_POLAR_POINTS_CLOSE(expected_intersection_spiral_1_spiral_2_0, intersections[0], 0.001);
  CHECK_POLAR_POINTS_CLOSE(expected_intersection_spiral_1_spiral_2_1, intersections[1], 0.001);

  // Spiral - spiral.
  num = geoviz::ComputeIntersections(spiral_2, spiral_3, intersections);
  UNITTEST_CHECK_EQUAL(2, num);
  CHECK_POLAR_POINTS_CLOSE(expected_intersection_spiral_2_spiral_3_0, intersections[0], 0.001);
  CHECK_POLAR_POINTS_CLOSE(expected_intersection_spiral_2_spiral_3_1, intersections[1], 0.001);

  // Spiral - spiral (equal angle).
  num = geoviz::ComputeIntersections(spiral_1, spiral_4, intersections);
  UNITTEST_CHECK_EQUAL(0, num);

  // Line - spiral.
  num = geoviz::ComputeIntersections(line_1, spiral_1, intersections);
  UNITTEST_CHECK_EQUAL(2, num);
  CHECK_POLAR_POINTS_CLOSE(expected_intersection_line_1_spiral_1_0, intersections[0], 0.001);
  CHECK_POLAR_POINTS_CLOSE(expected_intersection_line_1_spiral_1_1, intersections[1], 0.001);

  // Line - spiral (one side).
  num = geoviz::ComputeIntersections(line_2, spiral_3, intersections);
  UNITTEST_CHECK_EQUAL(1, num);
  CHECK_POLAR_POINTS_CLOSE(expected_intersection_line_2_spiral_3_0, intersections[0], 0.001);

  // Line (through pole) - spiral.
  num = geoviz::ComputeIntersections(spiral_1, line_3, intersections);
  UNITTEST_CHECK_EQUAL(2, num);
  CHECK_POLAR_POINTS_CLOSE(expected_intersection_line_3_spiral_1_0, intersections[0], 0.001);
  CHECK_POLAR_POINTS_CLOSE(expected_intersection_line_3_spiral_1_1, intersections[1], 0.001);

  // Line - spiral (straight).
  num = geoviz::ComputeIntersections(spiral_5, line_2, intersections);
  UNITTEST_CHECK_EQUAL(1, num);
  CHECK_POLAR_POINTS_CLOSE(expected_intersection_line_2_spiral_5_0, intersections[0], 0.001);

  // Line (parallel) - spiral (straight).
  num = geoviz::ComputeIntersections(spiral_5, line_1, intersections);
  UNITTEST_CHECK_EQUAL(0, num);

  // Line (through pole) - spiral (straight).
  num = geoviz::ComputeIntersections(spiral_5, line_4, intersections);
  UNITTEST_CHECK_EQUAL(1, num);
  CHECK_POLAR_POINTS_CLOSE(expected_intersection_line_4_spiral_5_0, intersections[0], 0.001);

  // Line (parallel through pole) - spiral (straight).
  num = geoviz::ComputeIntersections(spiral_5, line_3, intersections);
  UNITTEST_CHECK_EQUAL(2, num);
  CHECK_POLAR_POINTS_CLOSE(expected_intersection_line_3_spiral_5_0, intersections[0], 0.001);
  CHECK_POLAR_POINTS_CLOSE(expected_intersection_line_3_spiral_5_1, intersections[1], 0.001);


  // Line - line segment.
  num = geoviz::ComputeIntersections(line_2, line_segment_1, intersections);
  UNITTEST_CHECK_EQUAL(1, num);

  num = geoviz::ComputeIntersections(line_2, line_segment_2, intersections);
  UNITTEST_CHECK_EQUAL(0, num);

  // Line - spiral segment.
  num = geoviz::ComputeIntersections(line_1, spiral_segment_1, intersections);
  UNITTEST_CHECK_EQUAL(2, num);

  num = geoviz::ComputeIntersections(line_1, spiral_segment_2, intersections);
  UNITTEST_CHECK_EQUAL(1, num);

  num = geoviz::ComputeIntersections(line_1, spiral_segment_3, intersections);
  UNITTEST_CHECK_EQUAL(0, num);

  // Line segment - spiral segment.
  num = geoviz::ComputeIntersections(line_segment_2, spiral_segment_2, intersections);
  UNITTEST_CHECK_EQUAL(0, num);
}

} // UNITTEST_SUITE(Common)

#endif //GEOVIZ_TEST_COMMON_COMMON_H
