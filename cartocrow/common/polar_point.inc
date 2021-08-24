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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 04-09-2020
*/

#ifndef CARTOCROW_COMMON_POLAR_POINT_INC
#define CARTOCROW_COMMON_POLAR_POINT_INC

#include "cartocrow/common/core_types.h"

#include <glog/logging.h>


namespace cartocrow
{

/**@brief Construct a polar point.
 * @tparam T1 the number type for the distance to the pole.
 * @tparam T2 the number type for the angle relative to the positive x-axis.
 * @param R the distance to the pole.
 * @param phi the angle from the pole.
 */
template<typename T1, typename T2>
PolarPoint::PolarPoint(const T1& R, const T2& phi) : R_(R), phi_(phi)
{
  CHECK_LE(0, R);
  while (phi_ < -M_PI)
    phi_ += M_2xPI;
  while (M_PI < phi_)
    phi_ -= M_2xPI;
}

} // namespace cartocrow

#endif //CARTOCROW_COMMON_POLAR_POINT_INC
