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

#include "compute_scale_factor.h"

#include <glog/logging.h>

#include "geoviz/necklace_map/compute_scale_factor_any_order.h"
#include "geoviz/necklace_map/compute_scale_factor_fixed_order.h"


namespace geoviz
{
namespace necklace_map
{

/**@struct ComputeScaleFactor
 * @brief A functor to compute the optimal scale factor for a collection of necklace map elements.
 *
 * The optimal scale factor is the maximum value such that if all necklace beads have radius scale factor * sqrt(data value), none of these beads are within the minimum separation distance of another bead on the same necklace.
 *
 * Note that this scale factor is the minimum over the scale factors per necklace. These scale factors per necklace can be determined independently.
 *
 * Note that we do not restrict the beads of different necklaces to overlap. In case of overlap between different necklaces, the user can manually adjust the buffer thickness or the positioning forces (see ComputeValidPlacement) to prevent overlapping beads.
 */

/**@fn ComputeScaleFactor::Ptr
 * @brief The preferred pointer type for storing or sharing a computation functor.
 */

/**@brief Construct a new scale factor computation functor.
 * @param parameters the parameters describing the desired type of functor.
 * @return a unique pointer containing a new functor or a nullptr if the functor could not be constructed.
 */
ComputeScaleFactor::Ptr ComputeScaleFactor::New(const Parameters& parameters)
{
  switch (parameters.order_type)
  {
    case OrderType::kFixed:
      return Ptr(new ComputeScaleFactorFixedOrder(parameters));
    case OrderType::kAny:
      return Ptr(new ComputeScaleFactorAnyOrder(parameters));
    default:
      return nullptr;
  }
}

/**@brief Construct a bead scale factor computation functor.
 * @param parameters @parblock the parameter settings to apply to the computations.
 *
 * Specifically parameters.buffer_rad is used to set the minimum distance in radians between necklace beads.
 *
 * This distance must be in the range [0, pi]. Note that values beyond some threshold based on the input regions, the scale factor is forced to 0.
 * @endparblock
 */
ComputeScaleFactor::ComputeScaleFactor(const Parameters& parameters)
  : buffer_rad_(parameters.buffer_rad), max_buffer_rad_(-1)
{
  CHECK_GE(buffer_rad_, 0);
  CHECK_LE(buffer_rad_, M_PI);
}

/**@fn virtual Number ComputeScaleFactor::operator()(Necklace::Ptr& necklace) = 0;
 * @brief Apply the scaler to a necklace.
 *
 * Note that elements with value 0 will not be included in the ordering.
 * @param necklace the necklace for which to determine the optimal scale factor.
 * @return the optimal scale factor.
 */

/**@brief Apply the scaler to a collection of necklaces.
 * @param necklaces the necklaces for which to determine the optimal scale factor.
 * @return the optimal scale factor.
 */
Number ComputeScaleFactor::operator()(std::vector<Necklace::Ptr>& necklaces)
{
  // Determine the optimal scale factor per necklace;
  // the global optimum is the smallest of these.
  Number scale_factor = -1;
  for (Necklace::Ptr& necklace : necklaces)
  {
    // Clean beads without a feasible interval.
    std::vector<Bead::Ptr> keep_beads;
    for (const Bead::Ptr& bead : necklace->beads)
      if (bead->feasible)
        keep_beads.push_back(bead);
    necklace->beads.swap(keep_beads);

    if (necklace->beads.empty())
      continue;

    // Limit the initial bead radii.
    Number rescale = 1;
    for (const Bead::Ptr& bead : necklace->beads)
    {
      CHECK_GT(bead->radius_base, 0);
      const Number distance = necklace->shape->ComputeDistanceToKernel(bead->feasible);
      const Number bead_rescale = bead->radius_base / distance;
      rescale = std::max(rescale, bead_rescale);
    }
    for (const Bead::Ptr& bead : necklace->beads)
      bead->radius_base /= rescale;

    const Number necklace_scale_factor = (*this)(necklace) / rescale;

    for (const Bead::Ptr& bead : necklace->beads)
      bead->radius_base *= rescale;

    if (scale_factor < 0 || necklace_scale_factor < scale_factor)
      scale_factor = necklace_scale_factor;
  }
  return std::max(scale_factor, Number(0));
}

/**@fn const Number& ComputeScaleFactor::max_buffer_rad() const;
 * @brief Get the maximum buffer (in radians) between beads for which there exists a valid bead placement on the processed necklaces.
 *
 * If a buffer larger that this value is used, the functor will never produce an optimal scale factor larger than 0.
 */

} // namespace necklace_map
} // namespace geoviz"
