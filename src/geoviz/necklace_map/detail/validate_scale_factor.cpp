/*
The Necklace Map library implements the algorithmic geo-visualization
method by the same name, developed by Bettina Speckmann and Kevin Verbeek
at TU Eindhoven (DOI: 10.1109/TVCG.2010.180 & 10.1142/S021819591550003X).
Copyright (C) 2019  Netherlands eScience Center and TU Eindhoven

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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 20-01-2020
*/

#include "validate_scale_factor.h"

#include <vector>

#include <glog/logging.h>

#include "geoviz/necklace_map/detail/cycle_node.h"


namespace geoviz
{
namespace necklace_map
{
namespace detail
{

/**@class ValidateScaleFactor
 * @brief Functor to validate whether for a given scale factor and buffer angle there exists a valid necklace map.
 */

/**@brief Construct a necklace map validator functor.
 * @param scale_factor the scale factor for which to validate the necklace map.
 * @param buffer_rad the buffer angle in radians for which to validate the necklace map.
 */
ValidateScaleFactor::ValidateScaleFactor(const Number& scale_factor, const Number& buffer_rad /*= 0*/)
  : scale_factor(scale_factor), buffer_rad(buffer_rad) {}

/**@brief Validate a necklace.
 * @param necklace the necklace to validate.
 * @return whether there exists a valid placement of the necklace beads, given the scale factor and buffer angle of the validator.
 */
bool ValidateScaleFactor::operator()(Necklace::Ptr& necklace) const
{
  const Number num_beads = necklace->beads.size();
  if (num_beads < 2)
  {
    // Place the elements in a valid position.
    for (Bead::Ptr& bead : necklace->beads)
    {
      bead->valid = std::make_shared<CircleRange>(*bead->feasible);
      bead->angle_rad = bead->valid->angle_cw_rad();
    }

    return true;
  }

  bool valid = true;

  // The validator expects the necklace sorted by the feasible intervals of its beads.
  //necklace->SortBeads();

  using NodeSet = std::vector<detail::BeadCycleNode>;
  NodeSet nodes;
  nodes.reserve(2 * num_beads);

  // Create a sorted cycle based on the feasible intervals of the necklace beads and compute the scaled covering radii.
  const Number necklace_radius = necklace->shape->ComputeRadius();
  for (const Bead::Ptr& bead : necklace->beads)
  {
    // Compute the scaled covering radius.
    CHECK_NOTNULL(bead);
    nodes.emplace_back(bead);
  }

  // Each node is duplicated with an offset to its interval to force cyclic validity.
  const NodeSet::iterator end = nodes.end();
  for (NodeSet::iterator node_iter = nodes.begin(); node_iter != end; ++node_iter)
  {
    CHECK_NOTNULL(node_iter->bead);
    nodes.emplace_back(node_iter->bead);

    nodes.back().interval_cw_rad += M_2xPI;
    nodes.back().interval_ccw_rad += M_2xPI;
  }

  // Compute the valid intervals at the specified scale factor, where beads can be placed without pairwise overlap.

  // Adjust the clockwise extremes.
  for (size_t n = 1; n < nodes.size(); ++n)
  {
    detail::BeadCycleNode& previous = nodes[n - 1];
    detail::BeadCycleNode& current = nodes[n];

    // The bead must not overlap the previous one.
    const Number distance_rad = current.interval_cw_rad - previous.interval_cw_rad;
    const Number min_distance_rad =
      2 * std::asin(scale_factor * (current.bead->radius_base + previous.bead->radius_base) / (2 * necklace_radius)) +
      buffer_rad;

    if (distance_rad < min_distance_rad)
    {
      current.interval_cw_rad = previous.interval_cw_rad + min_distance_rad;
      if (current.interval_ccw_rad < current.interval_cw_rad)
      {
        valid = false;
        current.interval_cw_rad = current.interval_ccw_rad;
      }
    }
  }

  if (!valid)
    return false;

  // Adjust the counterclockwise extremes.
  for (ptrdiff_t n = nodes.size() - 2; 0 <= n; --n)
  {
    detail::BeadCycleNode& next = nodes[n + 1];
    detail::BeadCycleNode& current = nodes[n];

    // The bead must not overlap the next one.
    const Number distance_rad = next.interval_ccw_rad - current.interval_ccw_rad;
    const Number min_distance_rad =
      2 * std::asin(scale_factor * (next.bead->radius_base + current.bead->radius_base) / (2 * necklace_radius));

    if (distance_rad < min_distance_rad)
    {
      current.interval_ccw_rad = next.interval_ccw_rad - min_distance_rad;
      if (current.interval_ccw_rad < current.interval_cw_rad)
        current.interval_ccw_rad = current.interval_cw_rad;
    }
  }

  // Store the valid intervals and place each bead inside its valid interval.
  for (size_t n = 0; n < num_beads; ++n)
  {
    Bead::Ptr& bead = necklace->beads[n];

    // The second half of the nodes have the correct clockwise extreme (offset by 2pi).
    const Number& angle_cw_rad = nodes[num_beads + n].interval_cw_rad;

    // The first half of the nodes have the correct counterclockwise extreme.
    const Number& angle_ccw_rad = nodes[n].interval_ccw_rad;

    bead->valid = std::make_shared<CircleRange>(angle_cw_rad, angle_ccw_rad);
    bead->angle_rad = angle_cw_rad;
  }

  return valid;
}

/**@brief Validate a collection of necklaces.
 * @param necklace the necklaces to validate.
 * @return whether for each necklace there exists a valid placement of the necklace beads, given the scale factor and buffer angle of the validator.
 */
bool ValidateScaleFactor::operator()(std::vector<Necklace::Ptr>& necklaces) const
{
  bool valid = true;
  for (Necklace::Ptr& necklace : necklaces)
    valid &= (*this)(necklace);
  return valid;
}

/**@fn Number ValidateScaleFactor::scale_factor;
 * @brief The scale factor at which to validate the necklace maps.
 */


/**@fn Number ValidateScaleFactor::buffer_rad;
 * @brief The buffer angle at which to validate the necklace maps.
 */

} // namespace detail
} // namespace necklace_map
} // namespace geoviz
