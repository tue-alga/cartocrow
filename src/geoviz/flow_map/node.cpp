/*
The Flow Map library implements the algorithmic geo-visualization
method by the same name, developed by Kevin Verbeek, Kevin Buchin,
and Bettina Speckmann at TU Eindhoven
(DOI: 10.1007/s00453-013-9867-z & 10.1109/TVCG.2011.202).
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 04-09-2020
*/

#include "node.h"

#include <glog/logging.h>


namespace geoviz
{
namespace flow_map
{

/**@struct Node
 * @brief A node in the flow map spiral tree.
 *
 * This node has a position and a numeric value indicating flow that enters the node.
 */

/**@fn Node::Ptr
 * @brief The preferred pointer type for storing or sharing a node.
 */

/**@brief Construct a new spiral tree node.
 * @param id @parblock the ID of the node.
 *
 * See Region::id for details on this ID.
 * @endparblock
 */
Node::Node(const std::string& id, const PolarPoint& position)
  : id(id), position(position), flow_in(0) {}

/**@fn Region Node::id;
 * @brief The ID of this node.
 */

/**@fn Region Node::position;
 * @brief The position of this node in polar coordinates.
 */

/**@fn Number Node::flow_in;
 * @brief The amount of flow that enters this node.
 */

} // namespace flow_map
} // namespace geoviz"
