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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 07-01-2020
*/

#ifndef CARTOCROW_NECKLACE_MAP_COMPUTE_VALID_PLACEMENT_H
#define CARTOCROW_NECKLACE_MAP_COMPUTE_VALID_PLACEMENT_H

#include <vector>

#include "../../core/core.h"
#include "../detail/validate_scale_factor.h"
#include "../necklace.h"
#include "../parameters.h"

namespace cartocrow::necklace_map {

class ComputeValidPlacement {
  public:
	virtual ~ComputeValidPlacement() = default;

	using Ptr = std::unique_ptr<ComputeValidPlacement>;

	static Ptr construct(const Parameters& parameters);

	ComputeValidPlacement(const int cycles, const Number<Inexact>& aversion_ratio,
	                      const Number<Inexact>& buffer_rad = 0);

	void operator()(const Number<Inexact>& scale_factor, Necklace& necklace) const;

	void operator()(const Number<Inexact>& scale_factor, std::vector<Necklace>& necklaces) const;

	int cycles;

	Number<Inexact> aversion_ratio;

	Number<Inexact> buffer_rad;

  protected:
	virtual void SwapBeads(Necklace& necklace) const = 0;
};

class ComputeValidPlacementFixedOrder : public ComputeValidPlacement {
  public:
	ComputeValidPlacementFixedOrder(const int cycles, const Number<Inexact>& aversion_ratio,
	                                const Number<Inexact>& min_separation = 0);

  protected:
	void SwapBeads(Necklace& necklace) const override {}
};

class ComputeValidPlacementAnyOrder : public ComputeValidPlacement {
  public:
	ComputeValidPlacementAnyOrder(const int cycles, const Number<Inexact>& aversion_ratio,
	                              const Number<Inexact>& min_separation = 0);

  protected:
	void SwapBeads(Necklace& necklace) const override;
};

} // namespace cartocrow::necklace_map

#endif //CARTOCROW_NECKLACE_MAP_COMPUTE_VALID_PLACEMENT_H
