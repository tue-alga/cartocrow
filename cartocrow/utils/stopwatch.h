/*
Copyright (C) 2026  TU Eindhoven

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
*/

#pragma once

#include <string>
#include <list>

namespace cartocrow {

enum TimeResolution { SECONDS, MILLISECONDS, NANOSECONDS };

class StopwatchPool;

/// A simple Stopwatch class that allows for tracking wallclock time.
class Stopwatch {
  private:
	std::string name;
	TimeResolution resolution;
	long total = 0;
	long count = 0;
	long start_time = 0;

	long current_time();

  public:
	Stopwatch(std::string name, TimeResolution resolution = MILLISECONDS);

	Stopwatch& start();
	long stop();

	long getTotalTime();
	long getCount();

	TimeResolution getResolution();

	friend class StopwatchPool;
};

/// A collection of Stopwatches, all at the same temporal resolution. Stopwatches are identified by their name.
class StopwatchPool {
  private:
	std::string name;
	TimeResolution resolution;
	std::list<Stopwatch> watches;

  public:
	StopwatchPool(std::string name, TimeResolution resolution = MILLISECONDS);

	Stopwatch& get(std::string name);

	void clear();
	void printAll();
	void printAndClear();

	/// A global millisecond stopwatch pool.
	static StopwatchPool& global();
};

} // namespace cartocrow