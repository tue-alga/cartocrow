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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 04-02-2020
*/

#ifndef GEOVIZ_COMMON_TIMER_H
#define GEOVIZ_COMMON_TIMER_H

#include <deque>
#include "time.h"


namespace geoviz
{

class Timer
{
 public:
  Timer(const size_t memory = 10);

  void Reset();

  double Stamp();

  double Peek(const size_t skip = 0) const;

  double Span() const;

 private:
  // If skip is too large, the starting time is used.
  double Compare(const clock_t time, const size_t skip = 0) const;

  clock_t start_;
  std::deque<clock_t> times_;
  size_t memory_;
}; // class Timer

} // namespace geoviz

#endif //GEOVIZ_COMMON_TIMER_H
