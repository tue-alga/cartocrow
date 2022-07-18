/*
The Necklace Map library implements the algorithmic geo-visualization
method by the same name, developed by Bettina Speckmann and Kevin Verbeek
at TU Eindhoven (DOI: 10.1109/TVCG.2010.180 & 10.1142/S021819591550003X).
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 10-09-2019
*/

#ifndef CARTOCROW_NECKLACE_MAP_BEAD_H
#define CARTOCROW_NECKLACE_MAP_BEAD_H

#include "../core/core.h"
#include "../core/region_map.h"
#include "circular_range.h"

namespace cartocrow::necklace_map {

class Necklace;

/// A bead in a necklace map.
///
/// A bead \f$b\f$ stores its region and the value \f$v_b\f$ (corresponding to
/// that region) that it displays. The bead is shown with radius
/// \f$s \cdot \sqrt{v_b}\f$ where \f$s\f$ is the scaling factor of the drawing.
class Bead {
  public:
	/// Constructs a bead for the given region, displaying the given value
	/// \f$v_b\f$.
	Bead(const Region* region, const Number<Inexact>& value, size_t necklace_index);

	/// The region this bead displays the data value of.
	const Region* region;
	/// The base radius of this bead, that is, \f$\sqrt{v_b}\f$.
	Number<Inexact> radius_base;
	/// The feasible interval for this bead.
	CircularRange feasible{0, 0}; // TODO see valid below

	/// The covering radius of the scaled bead in radians.
	///
	/// This covering radius is the inner angle of the wedge that has the
	/// necklace kernel as apex and for which one leg intersects the bead
	/// center and the other leg is tangent to the boundary of the bead.
	///
	/// This is used during scaling.
	Number<Inexact> covering_radius_rad;

	/// The valid interval.
	CircularRange valid{
	    0, 0}; // TODO initializer temporary to make it compile, should check if we should make this std::optional or something
	// TODO(tvl) replace CycleNode::valid by this one and see if this can mean
	// that the positioner needs not compute it (may have to add computing valid
	// in fixed order scaler)?

	/// The angle in radians of the final position of the bead.
	Number<Inexact> angle_rad;

	/// Index of the necklace in \ref NecklaceMap::m_necklaces that this bead
	/// is on.
	size_t necklace_index;
};

} // namespace cartocrow::necklace_map

#endif //CARTOCROW_NECKLACE_MAP_BEAD_H
