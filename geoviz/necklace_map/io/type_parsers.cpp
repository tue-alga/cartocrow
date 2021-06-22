/*
The Necklace Map library implements the algorithmic
geo-visualization method by the same name, developed by
Bettina Speckmann and Kevin Verbeek at TU Eindhoven
(DOI: 10.1109/TVCG.2010.180 & 10.1142/S021819591550003X).
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 13-02-2020
*/

#include "type_parsers.h"


namespace geoviz
{
namespace necklace_map
{

/**@class IntervalTypeParser
 * @brief A simple parser to convert strings to interval types or vise versa.
 */

/**@fn IntervalTypeParser::IntervalType
 * @brief The type of intervals in which the necklace beads must be placed.
 */

/**@fn IntervalTypeParser::kCentroid
 * @brief The string representation of the centroid IntervalType.
 */

/**@fn IntervalTypeParser::kWedge
 * @brief The string representation of the wedge IntervalType.
 */

/**@brief Construct an interval type parser.
 * @param type whether to store any parsed interval types.
 */
IntervalTypeParser::IntervalTypeParser(IntervalType& type) :
  type(type) {}

/**@brief Apply the parser to an interval type string.
 * @param str the string to parse.
 * @return whether the string could be parsed.
 */
bool IntervalTypeParser::operator()(const std::string& str) const
{
  if (str == kCentroid)
  {
    type = IntervalType::kCentroid;
    return true;
  } else if (str == kWedge)
  {
    type = IntervalType::kWedge;
    return true;
  }
  return false;
}

/**@brief Construct a string from the last interval type parsed.
 * @return the constructed string.
 */
std::string IntervalTypeParser::Serialize() const
{
  if (type == IntervalType::kCentroid)
  {
    return kCentroid;
  }
  else
  {
    return kWedge;
  }
}

/**@fn IntervalType& IntervalTypeParser::type;
 * @brief The last interval type parsed.
 */


/**@class OrderTypeParser
 * @brief A simple parser to convert strings to order types or vise versa.
 */

/**@fn OrderTypeParser::OrderType
 * @brief The order in which the necklace beads must be placed.
 */

/**@fn OrderTypeParser::kFixed
 * @brief The string representation of the fixed OrderType.
 *
 * For fixed order necklaces, the beads must be placed on the necklace in the same order as the order of the region centroids, projected onto the necklace.
 */

/**@fn OrderTypeParser::kAny
 * @brief The string representation of the any OrderType.
 *
 * For any order necklaces, the beads can be placed in any onto the necklace. This allows the order of beads to be swapped if this results in a larger scale factor.
 */

/**@brief Construct an order type parser.
 * @param type whether to store any parsed order types.
 */
OrderTypeParser::OrderTypeParser(OrderType& type) :
  type(type) {}

/**@brief Apply the parser to an order type string.
 * @param str the string to parse.
 * @return whether the string could be parsed.
 */
bool OrderTypeParser::operator()(const std::string& str) const
{
  if (str == kFixed)
  {
    type = OrderType::kFixed;
    return true;
  } else if (str == kAny)
  {
    type = OrderType::kAny;
    return true;
  }
  return false;
}

/**@brief Construct a string from the last order type parsed.
 * @return the constructed string.
 */
std::string OrderTypeParser::Serialize() const
{
  if (type == OrderType::kFixed)
  {
    return kFixed;
  }
  else
  {
    return kAny;
  }
}

/**@fn OrderType& OrderTypeParser::type;
 * @brief The last order type parsed.
 */

} // namespace necklace_map
} // namespace geoviz
