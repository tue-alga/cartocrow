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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 28-11-2019
*/

#ifndef CARTOCROW_NECKLACE_MAP_COMPUTE_SCALE_FACTOR_H
#define CARTOCROW_NECKLACE_MAP_COMPUTE_SCALE_FACTOR_H

#include <memory>
#include <vector>

#include "../../core/core.h"
#include "../necklace.h"
#include "../parameters.h"

namespace cartocrow::necklace_map {

/// A functor to compute the optimal scale factor for a single necklace.
///
/// The optimal scale factor is the maximum value \f$s\f$ such that if all
/// necklace beads \f$b\f$ have radius \f$s \cdot \sqrt{v_b}\f$ (where \f$v_b\f$
/// is the data value for \f$b\f$), there is a necklace map such that none the
/// beads are closer than the minimum separation distance of another bead on the
/// same necklace.
class ComputeScaleFactor {
  public:
	virtual ~ComputeScaleFactor() = default;

	/// Constructs the functor.
	static std::shared_ptr<ComputeScaleFactor> construct(const Parameters& parameters);

	/// Applies the scaler to the given necklace. Elements with value `0` are
	/// excluded from the ordering.
	/// \return The optimal scale factor computed.
	virtual Number<Inexact> operator()(Necklace& necklace) = 0;

	/// Applies the scaler to a list of necklaces.
	/// \return The optimal scale factor computed.
	Number<Inexact> operator()(std::vector<Necklace>& necklaces);

  protected:
	/// Constructs a new scale factor computation functor.
	explicit ComputeScaleFactor(const Parameters& parameters);

	Number<Inexact> buffer_rad_;
	Number<Inexact> max_buffer_rad_;
};

} // namespace cartocrow::necklace_map

#endif //CARTOCROW_NECKLACE_MAP_COMPUTE_SCALE_FACTOR_H
