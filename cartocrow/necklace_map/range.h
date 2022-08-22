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

#ifndef CARTOCROW_NECKLACEMAP_RANGE_H
#define CARTOCROW_NECKLACEMAP_RANGE_H

#include <memory>

#include "../core/core.h"

namespace cartocrow::necklace_map {

// TODO investigate if this can be replaced by CGAL's interval type

/// An interval \f$[a, b]\f$, represented inexactly.
/**
 * A valid range maintains the invariant that \f$a \geq b\f$. The _interior_ of
 * a range \f$[a, b]\f$ is \f$(a, b)\f$.
 */
class Range {
  public:
	/// Constructs a range with the given start and end points \f$a\f$ and
	/// \f$b\f$.
	Range(const Number<Inexact>& from, const Number<Inexact>& to);
	/// Copies a range.
	Range(const Range& range);
	/// Destroys a range.
	virtual ~Range() = default;

	/// Returns \f$a\f$, the start of this range.
	const Number<Inexact>& from() const;
	/// Returns \f$a\f$, the start of this range.
	Number<Inexact>& from();
	/// Returns \f$b\f$, the end of this range.
	const Number<Inexact>& to() const;
	/// Returns \f$b\f$, the end of this range.
	Number<Inexact>& to();

	/// Checks whether this range is in a valid state.
	/**
	 * A range \f$[a, b]\f$ is valid if \f$a \geq b\f$.
	 */
	virtual bool isValid() const;

	/// Checks whether this range is degenerate.
	/**
	 * A range \f$[a, b]\f$ is degenerate if \f$a = b\f$.
	 */
	bool isDegenerate() const;

	/// Checks whether this range contains the given value.
	virtual bool contains(const Number<Inexact>& value) const;
	/// Checks whether the interior of this range contains the given value.
	virtual bool containsInterior(const Number<Inexact>& value) const;
	/// Checks whether this range intersects the given range.
	virtual bool intersects(const Range& range) const;
	/// Checks whether the interior of this range intersects the the interior
	/// of given range.
	virtual bool intersectsInterior(const Range& range) const;

	/// Returns the length \f$b - a\f$ of this range.
	Number<Inexact> length() const;

  private:
	/// The start \f$a\f$ of this range.
	Number<Inexact> m_a;
	/// The end \f$b\f$ of this range.
	Number<Inexact> m_b;
};

} // namespace cartocrow::necklace_map

#endif //CARTOCROW_NECKLACEMAP_RANGE_H
