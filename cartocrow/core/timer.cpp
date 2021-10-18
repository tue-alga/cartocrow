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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 04-02-2020
*/

#include "timer.h"

namespace cartocrow {

/**@class Timer
 * @brief A simple timer that keeps track of a fixed number of timestamps.
 *
 * When a large amount of timestamps are collected, the oldest one(s) get discarded.
 *
 * The last time the timer was constructed or reset is kept track of separately.
 */

/**@brief Construct a new timer and set the memory size limit.
 * @param memory the maximum number of timestamps that the timer remembers.
 */
Timer::Timer(const size_t memory /*= 10*/) : memory_(memory) {
	Reset();
}

/**@brief Reset the timer.
 * This clears the memory and sets the current time as starting time and as the first timestamp.
 */
void Timer::Reset() {
	start_ = clock();
	times_.clear();
	times_.push_front(start_);
}

/**@brief Add a timestamp.
 * @return The time since the previous timestamp.
 */
double Timer::Stamp() {
	const clock_t time = clock();
	const double difference = Compare(time);
	times_.push_front(time);
	if (memory_ < times_.size()) {
		times_.pop_back();
	}
	return difference;
}

/**@brief Compute the time since some earlier timestamp.
 *
 * Note that this does not insert a new timestamp.
 * @param skip @parblock how far to peek back in the list of timestamps.
 *
 * For skip = 0, the most recent timestamp is considered. If skip exceeds the memory size, the starting time is considered.
 * @endparblock
 * @return the time difference between the current time and the considered timestamp.
 */
double Timer::Peek(const size_t skip /*= 0*/) const {
	const clock_t time = clock();
	return Compare(time, skip);
}

/**@brief Compute the total time recorded by the timer.
 *
 * Note that this does not consider the current time, only time stamps in the memory of the timer and the time since its last reset.
 * @return the time difference between the starting time and the most recent timestamp.
 */
double Timer::Span() const {
	return double(times_.front() - start_) / CLOCKS_PER_SEC;
}

double Timer::Compare(const clock_t time, const size_t skip /*= 0*/) const {
	const clock_t& stamp = skip < times_.size() ? times_.at(skip) : start_;
	return double(time - stamp) / CLOCKS_PER_SEC;
}

} // namespace cartocrow
