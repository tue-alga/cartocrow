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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 10-04-2020
*/

#ifndef GEOVIZ_TEST_TEST_H
#define GEOVIZ_TEST_TEST_H

#include <UnitTest++/UnitTest++.h>

#ifdef GEOVIZ_DISABLE_SUITE_MAIN
#define GEOVIZ_RUN_TESTS(x)
#endif // GEOVIZ_DISABLE_SUITE_MAIN

// Note that calling the extern is a hack for release configuration to force rebuilding the code.
#ifndef GEOVIZ_RUN_TESTS
#define GEOVIZ_RUN_TESTS(x) int main() { extern void x(); x(); return UnitTest::RunAllTests(); }
#endif // GEOVIZ_RUN_TESTS

#endif //GEOVIZ_TEST_TEST_H
