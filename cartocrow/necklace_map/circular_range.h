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

#ifndef CARTOCROW_CORE_CIRCULAR_RANGE_H
#define CARTOCROW_CORE_CIRCULAR_RANGE_H

#include <memory>

#include "../core/core.h"
#include "range.h"

namespace cartocrow::necklace_map {

/// An angular interval \f$[\alpha, \beta]\f$.
/**
 * A valid circular range maintains the invariant that \f$a \in [0, 2 \pi)\f$
 * and \f$b \in [a, a + 2 \pi)\f$, or \f$a = 0\f$ and \f$b = 2 \pi\f$ (this
 * represents the full interval.
 *
 * The range is considered to be specified counterclockwise, for example, the
 * circular range \f$[0, \pi / 2]\f$ covers one quarter of the circle, while
 * \f$[\pi / 2, 2 \pi]\f$ covers three quarters of the circle.
 */
class CircularRange : public Range {
  public:
	/// Constructs a circular range between the two given angles.
	/**
     * If the given angles are outside the range $[0, 2 \pi)$ they are
	 * normalized to fall within this range.
     */
	CircularRange(const Number<Inexact>& from_angle, const Number<Inexact>& to_angle);

	/// Constructs a circular range from a regular range.
	/**
     * If the given angles are outside the range $[0, 2 \pi)$ they are
	 * normalized to fall within this range.
	 */
	explicit CircularRange(const Range& range);

	bool isValid() const override;

	/// Checks whether this circular range covers the full circle.
	bool isFull() const;

	virtual bool contains(const Number<Inexact>& value) const override;
	virtual bool containsInterior(const Number<Inexact>& value) const override;
	virtual bool intersects(const Range& range) const override;
	virtual bool intersectsInterior(const Range& range) const override;

	/// Computes the midpoint angle of this circular range.
	Number<Inexact> midpoint() const;
};

} // namespace cartocrow::necklace_map

#endif //CARTOCROW_CORE_CIRCULAR_RANGE_H
