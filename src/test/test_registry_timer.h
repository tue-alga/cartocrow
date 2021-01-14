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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 16-11-2020
*/

#ifndef GEOVIZ_TEST_TEST_REGISTRY_TIMER_H
#define GEOVIZ_TEST_TEST_REGISTRY_TIMER_H

#include "test/test_registry.h"


template <typename T_, size_t P_, size_t V_>
struct PrintTimes
{
  using Self_ = PrintTimes<T_, P_, V_>;
  using R_ = Registry<T_, P_, V_, Self_>;

  using Memory = typename R_::Memory;
  using Block = typename R_::Block;

  void operator()(const Memory& memory) const
  {
    double geom = 0, data = 0;
    for (const Block& block : memory)
    {
      std::cout << "Read time [" << block.first << "] geom: " << block.second[0] << " seconds; data: " << block.second[1] << " seconds" << std::endl;
      geom += block.second[0];
      data += block.second[1];
    }

    std::cout << "Read time [TOTAL] geom: " << geom << " seconds; data: " << data << " seconds" << std::endl;
  }
}; // struct PrintTimes

#endif //GEOVIZ_TEST_TEST_REGISTRY_TIMER_H
