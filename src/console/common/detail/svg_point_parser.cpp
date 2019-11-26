/*
Parser for SVG point coordinates.
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 26-11-2019
*/

#include "svg_point_parser.h"


namespace geoviz
{
namespace detail
{

/**@class SvgPointParser
 * @brief Functor to parse strings as SVG points and coordinates and to collect this data
 * from a string stream.
 */

/**@brief Convert a string to a number.
 *
 * If the string is a not a valid number, a std::invalid_argument exception is thrown.
 * @param str the number as string.
 * @return the number.
 */
Number SvgPointParser::toNumber(const std::string& str)
{
  return std::stod(str);
}

/**@brief Convert a string to a x coordinate.
 *
 * If the string is a not a valid number, a std::invalid_argument exception is thrown.
 * @param str the x coordinate as string.
 * @return the x coordinate.
 */
Number SvgPointParser::toX(const std::string& str)
{
  return toNumber(str);
}

/**@brief Convert a string to a y-coordinate.
 *
 * Note that SVG uses a y-down coordinate system, while the point uses y-up coordinates.
 *
 * If the string is a not a valid number, a std::invalid_argument exception is thrown.
 * @param str the y coordinate as string.
 * @return the y-coordinate.
 */
Number SvgPointParser::toY(const std::string& str)
{
  return -toNumber(str);
}

/**@brief Convert two strings to a point.
 *
 * Note that SVG uses a y-down coordinate system, while the point uses y-up coordinates.
 *
 * If either string is a not a valid number, a std::invalid_argument exception is thrown.
 * @param str_x the x coordinate as string.
 * @param str_y the y coordinate as string.
 * @return the point.
 */
Point SvgPointParser::toPoint(const std::string& str_x, const std::string& str_y)
{
  return Point(CGAL::ORIGIN) + toVector(str_x, str_y);
}

/**@brief Convert two strings to a vector.
 *
 * Note that SVG uses a y-down coordinate system, while the vector uses y-up coordinates.
 *
 * If either string is a not a valid number, a std::invalid_argument exception is thrown.
 * @param str_x the x coordinate as string.
 * @param str_y the y coordinate as string.
 * @return the vector.
 */
Vector SvgPointParser::toVector(const std::string& str_x, const std::string& str_y)
{
  return Vector(toX(str_x), toY(str_y));
}

/**@brief Convert the next token in the stream to a number.
 *
 * If the next token is a not a valid number, the stream's failbit is enabled.
 * @param ss the string stream.
 * @return the next token as number.
 */
Number SvgPointParser::getNumber(std::stringstream& ss)
{
  Number v;
  ss >> v;
  return v;
}

/**@brief Convert the next token in the stream to a x coordinate.
 *
 * If the next token is a not a valid number, the stream's failbit is enabled.
 * @param ss the string stream.
 * @return the next token as x coordinate.
 */
Number SvgPointParser::getX(std::stringstream& ss)
{
  return getNumber(ss);
}

/**@brief Convert the next token in the stream to a y coordinate.
 *
 * Note that SVG uses a y-down coordinate system, while the point uses y-up coordinates.
 *
 * If the next token is a not a valid number, the stream's failbit is enabled.
 * @param ss the string stream.
 * @return the next token as y coordinate.
 */
Number SvgPointParser::getY(std::stringstream& ss)
{
  return -getNumber(ss);
}

/**@brief Convert the next two tokens in the stream to a point.
 *
 * Note that SVG uses a y-down coordinate system, while the point uses y-up coordinates.
 *
 * If the next token is a not a valid number, the stream's failbit is enabled.
 * @param ss the string stream.
 * @return the next two tokens as point.
 */
Point SvgPointParser::getPoint(std::stringstream& ss)
{
  return Point(CGAL::ORIGIN) + getVector(ss);
}

/**@brief Convert the next two tokens in the stream to a vector.
 *
 * Note that SVG uses a y-down coordinate system, while the vector uses y-up coordinates.
 *
 * If the next token is a not a valid number, the stream's failbit is enabled.
 * @param ss the string stream.
 * @return the next two tokens as vector.
 */
Vector SvgPointParser::getVector(std::stringstream& ss)
{
  // Note that parsing the stream must be performed in the correct order,
  // so this cannot be done as part of the constructor parameters.
  const Number x = getX(ss);
  const Number y = getY(ss);
  return Vector(x, y);
}

} // namespace detail
} // namespace geoviz
