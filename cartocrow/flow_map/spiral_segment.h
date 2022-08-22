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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 26-02-2021
*/

#ifndef CARTOCROW_CORE_SPIRAL_SEGMENT_H
#define CARTOCROW_CORE_SPIRAL_SEGMENT_H

#include <ostream>

#include "../core/core.h"
#include "polar_point.h"
#include "spiral.h"

namespace cartocrow::flow_map {

/// A section of a logarithmic spiral.
class SpiralSegment : private Spiral {
  public:
	/// Constructs the shortest logarithmic spiral segment connecting the two
	/// given points \f$p_1 = (r_1, \phi_1)\f$ and \f$p_2 = (r_2, \phi_2)\f$.
	///
	/// The anchor of the spiral is set to \f$p_1\f$ if \f$r_1 > r_2\f$ and to
	/// \f$p_2\f$ if \f$r_1 < r_2\f$. This constructor throws if \f$r_1 =
	/// r_2\f$ (in which case no spiral connecting them exists).
	///
	/// If \f$p_1\f$ and \f$p_2\f$ are collinear with the root, then the result
	/// is a line segment.
	SpiralSegment(const PolarPoint& p1, const PolarPoint& p2);

	/// Constructs a spiral segment starting at the given point \c far, with the
	/// given angle, and ending at \c rMin.
	SpiralSegment(const PolarPoint& far, const Number<Inexact>& angle, const Number<Inexact>& rMin);

	/// Constructs a spiral segment with the given anchor and angle, starting
	/// at \c rMin and ending at \c rMax.
	SpiralSegment(const PolarPoint& anchor, const Number<Inexact>& angle,
	              const Number<Inexact>& rMin, const Number<Inexact>& rMax);

	/// Returns the anchor of the supporting spiral of this spiral segment.
	const PolarPoint& anchor() const;
	/// Returns the angle of the supporting spiral of this spiral segment.
	const Number<Inexact>& angle() const;

	/// Returns the far endpoint of this spiral segment.
	PolarPoint far() const;
	/// Returns the near endpoint of this spiral segment.
	PolarPoint near() const;
	/// Returns the lower bound for \f$r\f$ in this spiral segment.
	const Number<Inexact>& rMin() const;
	/// Returns the upper bound for \f$r\f$ in this spiral segment.
	const Number<Inexact>& rMax() const;

	/// Checks if the given parameter falls within this spiral segment.
	bool containsParameter(const Number<Inexact>& t) const;
	/// Checks if this spiral segment contains a point with the given \f$r\f$.
	bool containsR(const Number<Inexact>& r) const;

	/// Returns the supporting spiral of this spiral segment.
	const Spiral& supportingSpiral() const;

  private:
	/// The lower bound for \f$r\f$.
	Number<Inexact> m_rMin;
	/// The upper bound for \f$r\f$.
	Number<Inexact> m_rMax;
};

std::ostream& operator<<(std::ostream& os, const SpiralSegment& segment);

} // namespace cartocrow::flow_map

#endif //CARTOCROW_CORE_SPIRAL_SEGMENT_H
