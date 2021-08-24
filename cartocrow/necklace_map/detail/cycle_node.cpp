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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 23-01-2020
*/

#include "cycle_node.h"


namespace cartocrow
{
namespace necklace_map
{
namespace detail
{

/**@struct CycleNode
 * @brief A node to cycle through the beads.
 *
 * As opposed to beads, these nodes may have a feasible interval completely outside [0, 2pi).
 * This means that they can be used to cycle through the nodes multiple times in order.
 */

/**@brief Clone a node.
 * @param node the cycle node to clone.
 */
CycleNode::CycleNode(const CycleNode& node) :
  bead(node.bead), valid(std::make_shared<Range>(*node.valid))
{}

/**@brief Construct a node for a particular bead.
 *
 * The valid interval is set to a copy of the feasible interval of the bead.
 * @param bead the bead to add to the cycle.
 */
CycleNode::CycleNode(const Bead::Ptr& bead) :
  bead(bead),
  valid(std::make_shared<Range>(*bead->feasible))
{}

/**@brief Construct a node for a particular bead.
 * @param bead the bead to add to the cycle.
 * @param valid the valid interval of the bead.
 */
CycleNode::CycleNode(const Bead::Ptr& bead, const Range::Ptr& valid) :
  bead(bead),
  valid(valid)
{}

/**@fn Bead::Ptr CycleNode::bead;
 * @brief The bead.
 */

/**@fn Number CycleNode::interval_cw_rad;
 * @brief The clockwise extreme of the nodes interval on the cycle.
 *
 * Note that unlike the bead's feasible interval, this can be larger than 2*PI.
 */

/**@fn Number CycleNode::interval_ccw_rad;
 * @brief The counterclockwise extreme of the nodes interval on the cycle.
 *
 * Note that unlike the bead's feasible interval, this can be larger than 2*PI.
 */

CycleNode::CycleNode() :
  bead(),
  valid()
{}

} // namespace detail
} // namespace necklace_map
} // namespace cartocrow
