/*
The GeoViz library implements algorithmic geo-visualization methods,
developed at TU Eindhoven.
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 16-11-2020
*/

#ifndef GEOVIZ_TEST_COMMON_COMMON_H
#define GEOVIZ_TEST_COMMON_COMMON_H

#include <iostream>

#include <glog/logging.h>

#include <cmake/geoviz_test_config.h>
#include <console/common/utils_filesystem.h>
#include <geoviz/common/timer.h>
#include <geoviz/common/circulator.h>

#include "test/test.h"
#include "test/test_registry_timer.h"


void TestCommon() {}  // Linking hack, each new test cpp file has it.

//constexpr const size_t kP = 2;
//constexpr const size_t kV = 2;

//using PT = PrintTimes<double, kP, kV>;
//using Reg = PT::R_;

//static Reg kRegistry;


static const filesystem::path kDataDir = filesystem::path(GEOVIZ_TEST_DATA_DIR) / "common";


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

} // UNITTEST_SUITE(Common)

#endif //GEOVIZ_TEST_COMMON_COMMON_H
