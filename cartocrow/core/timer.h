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

#ifndef CARTOCROW_CORE_TIMER_H
#define CARTOCROW_CORE_TIMER_H

#include "time.h"

#include <string>
#include <vector>

namespace cartocrow {

/// A simple timer that keeps track of the duration of a number of events.
/**
 * This is meant for reporting running times of steps of an algorithm for
 * logging purposes. The timer starts running on construction, and steps can
 * be added using \ref stamp(). The intended use is like this:
 * ```
 * cartocrow::Timer timer;
 *
 * // ... code to demolish Earth
 * timer.stamp("Demolish Earth");
 *
 * // ... code to build hyperspace bypass
 * timer.stamp("Build hyperspace bypass");
 *
 * timer.output();
 * ```
 * The call to \ref output() then outputs something like:
 * ```
 * Demolish Earth: 120 s
 * Build hyperspace bypass: 70 s
 * ```
 *
 * Information about the steps stored by the timer can also be obtained
 * programmatically by using \ref operator[]:
 * ```
 * std::cout << timer[0].first << "\n";  // "Demolish Earth"
 * std::cout << timer[0].second << "\n";  // "120"
 * ```
 * Any methods returning \c double return times in seconds.
 */
class Timer {
  public:
	/// Constructs a timer and starts it.
	Timer();

	/// Drops all existing steps and restarts the timer.
	void reset();

	/// Returns the <code>i</code>-th step.
	/**
	 * \return A pair containing the description and the duration.
	 */
	std::pair<std::string, double> operator[](const size_t& i) const;

	/// Adds a step ending at the current time with the given description.
	/**
	 * \return The duration of the added step, that is, the time that passed
	 * since the last timestamp.
	 */
	double stamp(const std::string& description);

	/// Returns the time that has passed since the last timestamp (without
	/// making a new timestamp).
	[[nodiscard]] double peek() const;

	/// Returns the total time that has passed since the timer has been started.
	double span() const;

	/// Returns the number of steps stored in this timer.
	size_t size() const;

	/// Outputs the steps to \ref std::cout in a human-readable format.
	void output() const;

  private:
	/// Converts a time in clock ticks to seconds.
	static double toSeconds(clock_t time);
	/// The event descriptions for each step.
	std::vector<std::string> m_descriptions;
	/// The timestamps (one more than contained in \ref m_descriptions).
	std::vector<clock_t> m_stamps;
};

} // namespace cartocrow

#endif //CARTOCROW_CORE_TIMER_H
