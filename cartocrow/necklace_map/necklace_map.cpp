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

#include "necklace_map.h"

#include <glog/logging.h>

/**@file
 * Global functions for computing necklace maps.
 */

namespace cartocrow {
namespace necklace_map {

/**@anchor necklace_scale_factor
 * @brief Compute a feasible interval per bead, the optimal scale factor for the necklaces, and a valid placement for the scaled beads.
 *
 * A feasible interval describes a continuous part of the necklace where the association between a bead and its region is clear.
 *
 * The optimal scale factor is the largest factor by which the beads can be scaled such that they can all be placed on their necklace without any overlap.
 *
 * A valid placement defines a position for each scaled bead such that it is inside its feasible interval and no two beads overlap in their interior.
 * @param parameters the parameter settings to apply to the computations.
 * @param elements the map elements involved.
 * @param necklaces the necklaces involved.
 * @return the optimal scale factor.
 */
Number ComputeScaleFactor(const Parameters& parameters, std::vector<MapElement::Ptr>& elements,
                          std::vector<Necklace::Ptr>& necklaces) {
	// Create a bead per necklace that an element is part of.
	for (Necklace::Ptr& necklace : necklaces)
		necklace->beads.clear();
	for (MapElement::Ptr& element : elements)
		element->InitializeBead(parameters);

	// Generate intervals based on the regions and necklaces.
	(*ComputeFeasibleInterval::New(parameters))(elements);

	// Compute the scaling factor.
	const Number scale_factor = (*ComputeScaleFactor::New(parameters))(necklaces);

	// Compute valid placement.
	(*ComputeValidPlacement::New(parameters))(scale_factor, necklaces);

	return scale_factor;
}

/**@anchor necklace_placement
 * @brief Compute a valid placement for the scaled beads.
 *
 * A valid placement defines a position for each scaled bead such that it is inside its feasible interval and no two beads overlap in their interior.
 *
 * Note that this placement will be stored in Bead::angle_rad for each bead involved.
 * @param parameters the parameter settings to apply to the computations.
 * @param scale_factor the factor by which to scale the beads.
 * @param elements the map elements involved.
 * @param necklaces the necklaces involved.
 */
void ComputePlacement(const Parameters& parameters, const Number& scale_factor,
                      std::vector<MapElement::Ptr>& elements, std::vector<Necklace::Ptr>& necklaces) {
	// Create a bead per necklace that an element is part of.
	for (Necklace::Ptr& necklace : necklaces)
		necklace->beads.clear();
	for (MapElement::Ptr& element : elements) {
		element->InitializeBead(parameters);

		if (element->bead) {
			CHECK_NOTNULL(element->input_feasible);

			element->bead->angle_rad = element->input_angle_rad;
			element->bead->feasible = element->input_feasible;
		}
	}

	// Compute valid placement.
	(*ComputeValidPlacement::New(parameters))(scale_factor, necklaces);
}

} // namespace necklace_map

} // namespace cartocrow