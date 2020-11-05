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

#include "place.h"

#include <glog/logging.h>


namespace geoviz
{
namespace flow_map
{

/**@struct Place
 * @brief A place on the flow map.
 *
 * This named place has a position and a numeric value indicating flow that enters this place.
 */

/**@fn Place::Ptr
 * @brief The preferred pointer type for storing or sharing a place.
 */

/**@brief Construct a new flow map place.
 * @param id @parblock the ID of the place.
 *
 * See Region::id for details on this ID.
 * @endparblock
 */
Place::Place(const std::string& id, const PolarPoint& position)
  : id(id), position(position), flow_in(0) {}

/**@fn Region Place::id;
 * @brief The ID of this place.
 */

/**@fn Region Place::position;
 * @brief The position of this place in polar coordinates.
 */

/**@fn Number Place::flow_in;
 * @brief The amount of flow that enters this place.
 */

} // namespace flow_map
} // namespace geoviz"
