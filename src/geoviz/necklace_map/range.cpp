/*
The Necklace Map library implements the algorithmic geo-visualization
method by the same name, developed by Bettina Speckmann and Kevin Verbeek
at TU Eindhoven (DOI: 10.1109/TVCG.2010.180 & 10.1142/S021819591550003X).
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 09-12-2019
*/

#include "range.h"


namespace geoviz
{
namespace necklace_map
{

/**@class Range
 * @brief A continuous interval on a circle.
 */

/**@brief Construct a range.
 * @param from the start of the range.
 * @param to the end of the range.
 */
Range::Range(const Number& from, const Number& to)
  : from_(from), to_(to)
{}

/**@brief Copy a range.
 * @param range the range to copy.
 */
Range::Range(const Range& range) :
  from_(range.from()), to_(range.to())
{}

/**@brief The start of the range.
 * @return the start of the range.
 */
const Number& Range::from() const { return from_; }

/**@brief The start of the range.
 * @return the start of the range.
 */
Number& Range::from() { return from_; }

/**@brief The end of the range.
 * @return the end of the range.
 */
const Number& Range::to() const { return to_; }

/**@brief The end of the range.
 * @return the end of the range.
 */
Number& Range::to() { return to_; }

/**@brief Check whether the range is in a valid state.
 *
 * The range is in a valid state if @f from() <= @f to().
 * @return whether the interval is valid.
 */
bool Range::IsValid() const
{
  return from() <= to();
}

/**@brief Check whether the range is degenerate.
 * @return true if and only if the range is a single point.
 */
bool Range::IsDegenerate() const
{
  return from() == to();
}

/**@brief Check whether the range contains a value.
 *
 * For this check, the range is considered closed.
 * @param value the value to query.
 * @return whether the range contains the value.
 */
bool Range::Contains(const Number& value) const
{
  return from() <= value && value <= to();
}

/**@brief Check whether the range contains a value.
 *
 * For this check, the range is considered open.
 * @param value the value to query.
 * @return whether the range contains the value.
 */
bool Range::ContainsOpen(const Number& value) const
{
  return from() <= value && value <= to();
}

/**@brief Check whether this range and another range intersect.
 *
 * For this check, the ranges are considered closed.
 * @param range the range for which to check the intersection.
 * @return whether the ranges intersect in their interior.
 */
bool Range::Intersects(const Range::Ptr& range) const
{
  return
    Contains(range->from()) ||
    range->Contains(from());
}

/**@brief Check whether this range and another range intersect.
 *
 * For this check, the ranges are considered open.
 * @param range the range for which to check the intersection.
 * @return whether the ranges intersect in their interior.
 */
bool Range::IntersectsOpen(const Range::Ptr& range) const
{
  return
    (Contains(range->from()) && range->from() != to()) ||
    (range->Contains(from()) && from() != range->to());
}

/**@brief Compute the total length of the range.
 * @return the total length.
 */
Number Range::ComputeLength() const
{
  return to() - from();
}

} // namespace necklace_map
} // namespace geoviz
