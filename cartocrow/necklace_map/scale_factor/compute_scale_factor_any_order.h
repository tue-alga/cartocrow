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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 03-03-2020
*/

#ifndef CARTOCROW_NECKLACE_MAP_COMPUTE_SCALE_FACTOR_ANY_ORDER_H
#define CARTOCROW_NECKLACE_MAP_COMPUTE_SCALE_FACTOR_ANY_ORDER_H

#include "../../core/core.h"
#include "../necklace.h"
#include "compute_scale_factor.h"

namespace cartocrow::necklace_map {

class ComputeScaleFactorAnyOrder : public ComputeScaleFactor {
  public:
	explicit ComputeScaleFactorAnyOrder(const Parameters& parameters);

	Number<Inexact> operator()(Necklace& necklace) override;

  private:
	int binary_search_depth_;
	int heuristic_cycles_;
}; // class ComputeScaleFactorAnyOrder

} // namespace cartocrow::necklace_map

#endif //CARTOCROW_NECKLACE_MAP_COMPUTE_SCALE_FACTOR_ANY_ORDER_H
