/*
The CartoCrow library implements algorithmic geo-visualization methods,
developed at TU Eindhoven.
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

#ifndef CARTOCROW_COMMON_POLAR_SEGMENT_INC
#define CARTOCROW_COMMON_POLAR_SEGMENT_INC

#include <glog/logging.h>

namespace cartocrow {

/**@brief Collect the t at a given distance from the pole.
 * @tparam OutputIterator the type of output iterator to which to send the output.
 * @param R the given distance from the pole.
 * @param t the t values of the points on the line segment at the desired distance.
 * @return the number of points on the line segment at the given distance. This can be either 0, 1, or 2.
 */
template <class OutputIterator>
int PolarSegment::CollectT(const Number& R, OutputIterator t) const {
	Number d[2];
	const int num_d = PolarLine::CollectT(R, d);

	int num = 0;
	for (int i = 0; i < num_d; ++i) {
		const Number tt = ToT(d[i]);
		if (ContainsT(tt)) {
			*t++ = tt;
			++num;
		}
	}

	return num;
}

/**@brief Compute the phi at a given distance from the pole.
 * @tparam OutputIterator the type of output iterator to which to send the output.
 * @param R the given distance from the pole.
 * @param phi the phi values of the points on the line segment at the desired distance.
 * @return the number of points on the line segment at the given distance. This can be either 0, 1, or 2.
 */
template <class OutputIterator>
int PolarSegment::CollectPhi(const Number& R, OutputIterator phi) const {
	Number t[2];
	const int num = CollectT(R, t);

	for (int i = 0; i < num; ++i)
		*phi++ = EvaluatePhi(t[i]);

	return num;
}

} // namespace cartocrow

#endif //CARTOCROW_COMMON_POLAR_SEGMENT_INC