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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 01-04-2020
*/

#include "core_types.h"


namespace cartocrow
{

/**@brief Constrain a value to be beyond some starting value by less than some range.
 * @param value the value to constrain.
 * @param start the minimum value.
 * @param range the maximum beyond the start value.
 * @return the value in the constrained range of [start, start+range].
 */
Number Modulo(const Number& value, const Number& start /*= 0*/, const Number& range /*= M_2xPI*/)
{
  Number constrained = value;
  while (constrained < start)
    constrained += range;
  while (start + range <= constrained)
    constrained -= range;
  return constrained;
}

/**@brief Constrain a value to be strictly beyond some starting value by at most some range.
 * @param value the value to constrain.
 * @param start the minimum value.
 * @param range the maximum beyond the start value.
 * @return the value in the constrained range of [start, start+range].
 */
Number ModuloNonZero(const Number& value, const Number& start /*= 0*/, const Number& range /*= M_2xPI*/)
{
  Number constrained = value;
  while (constrained <= start)
    constrained += range;
  while (start + range < constrained)
    constrained -= range;
  return constrained;
}

} // namespace cartocrow
