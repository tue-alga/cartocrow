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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 06-05-2020
*/

#ifndef CARTOCROW_COMMON_CIRCULAR_RANGE_H
#define CARTOCROW_COMMON_CIRCULAR_RANGE_H

#include <memory>

#include "cartocrow/common/core_types.h"
#include "cartocrow/common/range.h"


namespace cartocrow
{

class CircularRange : public Range
{
 public:
  using Ptr = std::shared_ptr<CircularRange>;

  CircularRange(const Number& from_rad, const Number& to_rad);

  explicit CircularRange(const Range& range);

  inline const Number& from_rad() const { return from(); }
  inline Number& from_rad() { return from(); }
  inline const Number& to_rad() const { return to(); }
  inline Number& to_rad() { return to(); }

  bool IsValid() const override;

  bool IsFull() const;

  virtual bool Contains(const Number& value) const override;

  virtual bool ContainsOpen(const Number& value) const override;

  virtual bool Intersects(const Range::Ptr& range) const override;

  virtual bool IntersectsOpen(const Range::Ptr& range) const override;

  Number ComputeCentroid() const;

  void Reverse();
}; // class CircularRange

} // namespace cartocrow

#endif //CARTOCROW_COMMON_CIRCULAR_RANGE_H
