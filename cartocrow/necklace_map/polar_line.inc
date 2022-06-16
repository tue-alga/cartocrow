/*
The Flow Map library implements the algorithmic geo-visualization
method by the same name, developed by Kevin Verbeek, Kevin Buchin,
and Bettina Speckmann at TU Eindhoven
(DOI: 10.1007/s00453-013-9867-z & 10.1109/TVCG.2011.202).
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 19-02-2021
*/

#ifndef CARTOCROW_CORE_POLAR_LINE_INC
#define CARTOCROW_CORE_POLAR_LINE_INC

#include <glog/logging.h>

namespace cartocrow {

/**@brief Collect the t at a given distance from the pole.
 * @tparam OutputIterator the type of output iterator to which to send the output.
 * @param R the given distance from the pole.
 * @param t the t values of the points on the line at the desired distance.
 * @return the number of points on the line at the given distance. This can be either 0, 1, or 2.
 */
template <class OutputIterator> int PolarLine::CollectT(const Number& R, OutputIterator t) const {
	CHECK_LE(0, R);

	// The point at a given distance from the pole could be computed using the sine law.
	// However, we instead base it on the point on the segment closest to the pole for two reasons.
	// Firstly, the segment is not guaranteed to come close enough to the pole.
	// Secondly, the sine law computation has to consider four different configurations for the point lying:
	// * clockwise or counter-clockwise of the closest point (sin(a) = sin(pi-a) identity of the angle near the point at R)
	// * inside or outside the segment (sin(a) versus sin(pi-a) of the angle at the segment endpoint).
	// Both issues are easier to resolve when basing the point at distance R on the point closest to the pole.

	if (R < foot().R())
		return 0;

	if (R == foot().R()) {
		*t++ = 0;
		return 1;
	}

	// Note that R = 0 would have been caught by the previous conditions.
	const Number offset = std::sqrt((R * R) - (foot().R() * foot().R()));
	*t++ = -offset;
	*t++ = offset;
	return 2;
}

/**@brief Compute the phi at a given distance from the pole.
 * @tparam OutputIterator the type of output iterator to which to send the output.
 * @param R the given distance from the pole.
 * @param phi the phi values of the points on the line at the desired distance.
 * @return the number of points on the line at the given distance. This can be either 0, 1, or 2.
 */
template <class OutputIterator>
int PolarLine::CollectPhi(const Number& R, OutputIterator phi) const {
	Number t[2];
	const int num = CollectT(R, t);

	for (int i = 0; i < num; ++i)
		*phi++ = EvaluatePhi(t[i]);

	return num;
}

} // namespace cartocrow

#endif //CARTOCROW_CORE_POLAR_LINE_INC