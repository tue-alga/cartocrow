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

#include "compute_valid_placement.h"

#include "../detail/cycle_node.h"
#include "../necklace_interval.h"

namespace cartocrow::necklace_map {
namespace detail {

inline Number<Inexact> DistanceOnCircle(const Number<Inexact>& from_rad,
                                        const Number<Inexact>& to_rad) {
	const Number<Inexact> dist = std::abs(to_rad - from_rad);
	return std::min(dist, M_2xPI - dist);
}

} // namespace detail

/**@class ComputeValidPlacement
 * @brief A functor to compute a valid placement for a collection of necklace beads.
 *
 * A placement for a set of necklace beads is a set of angles that describes the position of each bead on the necklace. A placement is valid if all scaled beads are inside their feasible interval and the distance between any two beads is at least some non-negative buffer distance. Note that this buffer distance guarantees that the beads do not overlap.
 *
 * There is often a range of valid placements. In this case, the placement is guided by an attraction-repulsion force: the beads are attracted to the center of their interval and repelled by the neighboring beads.
 *
 * Note that the placements are computed independently per necklace. This means that if a map contains multiple necklaces, no guarantees can be given about overlap of beads on different necklaces. However, such overlap can often be prevented manually by tuning the attraction-replusion force and the buffer distance.
 */

/**@fn ComputeValidPlacement::Ptr
 * @brief The preferred pointer type for storing or sharing a computation functor.
 */

/**@brief Construct a new valid placement computation functor.
 * @param parameters the parameters describing the desired type of functor.
 * @return a unique pointer containing a new functor or a nullptr if the functor could not be constructed.
 */
ComputeValidPlacement::Ptr ComputeValidPlacement::construct(const Parameters& parameters) {
	switch (parameters.order_type) {
	case OrderType::kFixed:
		return Ptr(new ComputeValidPlacementFixedOrder(
		    parameters.placement_cycles, parameters.aversion_ratio, parameters.buffer_rad));
	case OrderType::kAny:
		return Ptr(new ComputeValidPlacementAnyOrder(
		    parameters.placement_cycles, parameters.aversion_ratio, parameters.buffer_rad));
	default:
		return nullptr;
	}
}

/**@brief Construct a valid placement computation functor.
 * @param cycles the number of positioning cycles.
 * @param aversion_ratio @parblock the ratio between attraction to the interval center (0) and repulsion from the neighboring beads (1).
 *
 * This ratio must be in the range (0, 1].
 * @endparblock
 * @param buffer_rad the minimum distance (in radians on the necklace) between the beads.
 */
ComputeValidPlacement::ComputeValidPlacement(const int cycles, const Number<Inexact>& aversion_ratio,
                                             const Number<Inexact>& buffer_rad /*= 0*/
                                             )
    : cycles(cycles), aversion_ratio(aversion_ratio), buffer_rad(buffer_rad) {}

/**@brief Apply the functor place the beads on a necklace.
 *
 * The beads must start in a valid placement. This valid placement is guaranteed immediately after computing the optimal scale factor of the necklace.
 * @param scale_factor the factor by which to multiply the radius of the beads.
 * @param necklace the necklace to which to apply the functor.
 */
void ComputeValidPlacement::operator()(const Number<Inexact>& scale_factor,
                                       Necklace& necklace) const // TODO(tvl) factorize.
{
	const NecklaceShape::Ptr& necklace_shape = necklace.shape;
	for (const std::shared_ptr<Bead>& bead : necklace.beads) {
		// Compute the scaled covering radius.
		assert(bead != nullptr);
		bead->angle_rad = scale_factor == 0 ? bead->feasible.from() : wrapAngle(bead->angle_rad);
	}

	// Sort the necklace beads by their current angle.
	std::sort(necklace.beads.begin(), necklace.beads.end(),
	          [](const std::shared_ptr<Bead>& a, const std::shared_ptr<Bead>& b) -> bool {
		          return a->angle_rad < b->angle_rad;
	          });

	// Compute the valid intervals.
	const bool adjust_angle = 0 < aversion_ratio;
	detail::ValidateScaleFactor validate(scale_factor, buffer_rad, adjust_angle);
	const bool valid = validate(necklace);

	if (!valid || !adjust_angle) {
		return;
	}

	const double precision = 1e-7;
	const Number<Inexact> centroid_ratio = 1;

	const size_t num_beads = necklace.beads.size();
	for (int cycle = 0; cycle < cycles; ++cycle) {
		for (size_t index_bead = 0; index_bead < num_beads; ++index_bead) {
			std::shared_ptr<Bead>& bead = necklace.beads[index_bead];
			assert(bead != nullptr);

			const size_t index_prev = (index_bead + num_beads - 1) % num_beads;
			const size_t index_next = (index_bead + 1) % num_beads;

			const std::shared_ptr<Bead>& prev = necklace.beads[index_prev];
			const std::shared_ptr<Bead>& next = necklace.beads[index_next];

			const Number<Inexact> offset_from_prev_rad =
			    CircularRange(prev->angle_rad, bead->angle_rad).length();
			const Number<Inexact> offset_from_centroid_rad =
			    CircularRange(bead->feasible.midpoint(), bead->angle_rad).length();

			const Number<Inexact>& radius_bead = scale_factor * bead->radius_base;
			const Number<Inexact>& radius_prev = scale_factor * prev->radius_base;
			const Number<Inexact>& radius_next = scale_factor * next->radius_base;

			const Number<Inexact> distance_from_prev_min =
			    CircularRange(prev->angle_rad, necklace_shape->computeAngleAtDistanceRad(
			                                       prev->angle_rad, radius_prev + radius_bead))
			        .length() +
			    buffer_rad;
			const Number<Inexact> distance_from_prev_max =
			    CircularRange(prev->angle_rad, necklace_shape->computeAngleAtDistanceRad(
			                                       next->angle_rad, -(radius_bead + radius_next)))
			        .length() -
			    buffer_rad;

			// The 'bubble' is the largest range centered on the bead that does not contain the centroid.
			const Number<Inexact> offset_prev_to_bubble =
			    offset_from_centroid_rad < M_PI
			        ? (offset_from_prev_rad - offset_from_centroid_rad)
			        : (offset_from_prev_rad + (M_2xPI - offset_from_centroid_rad));

			const double w_0 = centroid_ratio * offset_prev_to_bubble * distance_from_prev_min *
			                       distance_from_prev_max -
			                   aversion_ratio * (distance_from_prev_min + distance_from_prev_max);
			const double w_1 =
			    aversion_ratio * 2 -
			    centroid_ratio * ((distance_from_prev_min + offset_prev_to_bubble) *
			                          (distance_from_prev_max + offset_prev_to_bubble) -
			                      offset_prev_to_bubble * offset_prev_to_bubble);
			const double w_2 = centroid_ratio * (distance_from_prev_min + distance_from_prev_max +
			                                     offset_prev_to_bubble);
			const double w_3 = -centroid_ratio;

			// Solve w_3 * x^3 + w_2 * x^2 + w_1 * x + w_0 = 0 up to the specified precision.
			if (std::abs(w_3) < precision && std::abs(w_2) < precision) {
				const Number<Inexact> x = wrapAngle(-w_0 / w_1 + prev->angle_rad);

				if (!bead->feasible.contains(x)) {
					if (0 < 2 * offset_from_prev_rad - distance_from_prev_min +
					            distance_from_prev_max) {
						bead->angle_rad = bead->feasible.from();
					} else {
						bead->angle_rad = bead->feasible.to();
					}
				} else {
					bead->angle_rad = x;
				}
			} else {
				const double q = (3 * w_3 * w_1 - w_2 * w_2) / (9 * w_3 * w_3);
				const double r = (9 * w_3 * w_2 * w_1 - 27 * w_3 * w_3 * w_0 - 2 * w_2 * w_2 * w_2) /
				                 (54 * w_3 * w_3 * w_3);

				const double rho = std::max(std::sqrt(-q * q * q), std::abs(r));

				const double theta_3 = std::acos(r / rho) / 3;
				const double rho_3 = std::pow(rho, 1 / 3.0);

				const Number<Inexact> x =
				    wrapAngle(prev->angle_rad - rho_3 * std::cos(theta_3) - w_2 / (3 * w_3) +
				              rho_3 * std::sqrt(3.0) * std::sin(theta_3));

				if (bead->feasible.contains(x)) {
					bead->angle_rad = x;
				} else if (0 < aversion_ratio * (2 * offset_from_prev_rad -
				                                 (distance_from_prev_min + distance_from_prev_max)) +
				                   centroid_ratio * (offset_prev_to_bubble - offset_from_prev_rad) *
				                       (offset_from_prev_rad - distance_from_prev_min) *
				                       (offset_from_prev_rad - distance_from_prev_max)) {
					bead->angle_rad = bead->feasible.from();
				} else {
					bead->angle_rad = bead->feasible.to();
				}
			}
			bead->angle_rad = wrapAngle(bead->angle_rad);
		}

		SwapBeads(necklace);
	}
}

/**@fn Number ComputeValidPlacement::cycles;
 * @brief The number of cycles to apply the positioning forces.
 */

/**@fn Number ComputeValidPlacement::aversion_ratio;
 * @brief The ratio between attraction to the interval center (0) and repulsion from the neighboring beads (1).
 */

/**@fn Number ComputeValidPlacement::buffer_rad;
 * @brief The minimum distance (in radians on the necklace) between the beads.
 */

/**@brief Apply the functor place the beads on a collection of necklaces.
 * @param scale_factor the factor by which to multiply the radius of the beads.
 * @param necklaces the necklaces to which to apply the functor.
 */
void ComputeValidPlacement::operator()(const Number<Inexact>& scale_factor,
                                       std::vector<Necklace>& necklaces) const {
	for (Necklace& necklace : necklaces) {
		(*this)(scale_factor, necklace);
	}
}

/**@class ComputeValidPlacementFixedOrder
 * @brief A functor to compute a valid placement for a collection of unordered necklace beads.
 *
 * A placement for a set of necklace beads is a set of angles that describes the position of each bead on the necklace. A placement is valid if all scaled beads are inside their feasible interval and the distance between any two beads is at least some non-negative buffer distance. Note that this buffer distance guarantees that the beads do not overlap.
 *
 * Beads must be ordered by the clockwise extreme of their feasible interval.
 *
 * There is often a range of valid placements. In this case, the placement is guided by an attraction-repulsion force: the beads are attracted to the center of their interval and repelled by the neighboring beads.
 *
 * Note that the placements are computed independently per necklace. This means that if a map contains multiple necklaces, no guarantees can be given about overlap of beads on different necklaces. However, such overlap can often be prevented manually by tuning the attraction-replusion force and the buffer distance.
 */

/**@brief Construct a valid placement computation functor.
 * @param cycles the number of positioning cycles.
 * @param aversion_ratio @parblock the ratio between attraction to the interval center (0) and repulsion from the neighboring beads (1).
 *
 * This ratio must be in the range (0, 1].
 * @endparblock
 * @param buffer_rad the minimum distance (in radians on the necklace) between the beads.
 */
ComputeValidPlacementFixedOrder::ComputeValidPlacementFixedOrder(
    const int cycles, const Number<Inexact>& aversion_ratio, const Number<Inexact>& buffer_rad /*= 0*/
    )
    : ComputeValidPlacement(cycles, aversion_ratio, buffer_rad) {}

/**@class ComputeValidPlacementAnyOrder
 * @brief A functor to compute a valid placement for a collection of unordered necklace beads.
 *
 * A placement for a set of necklace beads is a set of angles that describes the position of each bead on the necklace. A placement is valid if all scaled beads are inside their feasible interval and the distance between any two beads is at least some non-negative buffer distance. Note that this buffer distance guarantees that the beads do not overlap.
 *
 * Beads may be reordered if this would result in a valid placement where the beads are closer to the centroid of their feasible intervals.
 *
 * There is often a range of valid placements. In this case, the placement is guided by an attraction-repulsion force: the beads are attracted to the center of their interval and repelled by the neighboring beads.
 *
 * Note that the placements are computed independently per necklace. This means that if a map contains multiple necklaces, no guarantees can be given about overlap of beads on different necklaces. However, such overlap can often be prevented manually by tuning the attraction-replusion force and the buffer distance.
 */

/**@brief Construct a valid placement computation functor.
 * @param cycles the number of positioning cycles.
 * @param aversion_ratio @parblock the ratio between attraction to the interval center (0) and repulsion from the neighboring beads (1).
 *
 * This ratio must be in the range (0, 1].
 * @endparblock
 * @param min_separation the minimum distance (in radians as seen fron the necklace kernel) between the beads.
 */
ComputeValidPlacementAnyOrder::ComputeValidPlacementAnyOrder(const int cycles,
                                                             const Number<Inexact>& aversion_ratio,
                                                             const Number<Inexact>& min_separation /*= 0*/
                                                             )
    : ComputeValidPlacement(cycles, aversion_ratio, min_separation) {}

void ComputeValidPlacementAnyOrder::SwapBeads(Necklace& necklace) const {
	const size_t num_beads = necklace.beads.size();
	for (size_t index_bead = 0; index_bead < num_beads; index_bead++) {
		const size_t index_next = (index_bead + 1) % num_beads;

		std::shared_ptr<Bead>& bead = necklace.beads[index_bead];
		std::shared_ptr<Bead>& next = necklace.beads[index_next];

		const Number<Inexact>& radius_bead = bead->covering_radius_rad;
		const Number<Inexact>& radius_next = next->covering_radius_rad;

		// Note that for the swapped angles, the buffers cancel each other out.
		const Number<Inexact> swapped_angle_bead_rad =
		    wrapAngle(next->angle_rad + radius_next - radius_bead);
		const Number<Inexact> swapped_angle_next_rad =
		    wrapAngle(bead->angle_rad - radius_bead + radius_next);

		if (bead->feasible.contains(swapped_angle_bead_rad) &&
		    next->feasible.contains(swapped_angle_next_rad)) {
			const Number<Inexact> centroid_bead_rad = bead->feasible.midpoint();
			const Number<Inexact> centroid_next_rad = next->feasible.midpoint();

			const Number<Inexact> dist_original_bead =
			    detail::DistanceOnCircle(bead->angle_rad, centroid_bead_rad);
			const Number<Inexact> dist_original_next =
			    detail::DistanceOnCircle(next->angle_rad, centroid_next_rad);
			const Number<Inexact> dist_swapped_bead =
			    detail::DistanceOnCircle(swapped_angle_bead_rad, centroid_bead_rad);
			const Number<Inexact> dist_swapped_next =
			    detail::DistanceOnCircle(swapped_angle_next_rad, centroid_next_rad);

			const Number<Inexact> cost_original =
			    dist_original_bead * dist_original_bead + dist_original_next * dist_original_next;
			const Number<Inexact> cost_swapped =
			    dist_swapped_bead * dist_swapped_bead + dist_swapped_next * dist_swapped_next;

			if (cost_swapped < cost_original) {
				// Swap the beads.
				bead->angle_rad = swapped_angle_bead_rad;
				next->angle_rad = swapped_angle_next_rad;

				std::swap(necklace.beads[index_bead], necklace.beads[index_next]);
			}
		}
	}
}

} // namespace cartocrow::necklace_map
