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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 09-12-2019
*/

#ifndef CARTOCROW_COMMON_RANGE_H
#define CARTOCROW_COMMON_RANGE_H

#include <memory>

#include "cartocrow/common/core_types.h"

namespace cartocrow {

class Range {
  public:
	using Ptr = std::shared_ptr<Range>;

	Range(const Number& from, const Number& to);
	Range(const Range& range);

	const Number& from() const;
	Number& from();
	const Number& to() const;
	Number& to();

	virtual bool IsValid() const;

	bool IsDegenerate() const;

	virtual bool Contains(const Number& value) const;

	virtual bool ContainsOpen(const Number& value) const;

	virtual bool Intersects(const Range::Ptr& range) const;

	virtual bool IntersectsOpen(const Range::Ptr& range) const;

	Number ComputeLength() const;

  private:
	Number from_;
	Number to_;
}; // class Range

} // namespace cartocrow

#endif //CARTOCROW_COMMON_RANGE_H
