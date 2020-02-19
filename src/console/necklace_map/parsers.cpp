/*
The Necklace Map console application implements the algorithmic
geo-visualization method by the same name, developed by
Bettina Speckmann and Kevin Verbeek at TU Eindhoven
(DOI: 10.1109/TVCG.2010.180 & 10.1142/S021819591550003X).
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 13-02-2020
*/

#include "parsers.h"


namespace geoviz
{

/**@class IntervalTypeParser
 * @brief A simple parser to convert strings to interval types or vise versa.
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
  }
  if (str == kWedge)
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
  switch (type)
  {
    case IntervalType::kCentroid:
      return kCentroid;
    case IntervalType::kWedge:
      return kWedge;
  }
}

/**@fn IntervalType& IntervalTypeParser::type;
 * @brief The last interval type parsed.
 */


/**@class OrderTypeParser
 * @brief A simple parser to convert strings to order types or vise versa.
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
  }
  if (str == kAny)
  {
    type = OrderType::kAny;
    return true;
  }
  if (str == kHeuristic)
  {
    type = OrderType::kHeuristicAny;
    return true;
  }
  return false;
}

/**@brief Construct a string from the last order type parsed.
 * @return the constructed string.
 */
std::string OrderTypeParser::Serialize() const
{
  switch (type)
  {
    case OrderType::kFixed:
      return kFixed;
    case OrderType::kAny:
      return kAny;
    case OrderType::kHeuristicAny:
      return kHeuristic;
  }
}

/**@fn OrderType& OrderTypeParser::type;
 * @brief The last order type parsed.
 */

} // namespace geoviz
