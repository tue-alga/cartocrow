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

#include "stopwatch.h"

#include <chrono>
#include <iostream>

namespace cartocrow {

Stopwatch::Stopwatch(std::string name, TimeResolution resolution)
    : name(name), resolution(resolution) {
	count = 0;
	total = 0;
}

Stopwatch& Stopwatch::start() {
	start_time = current_time();
	return *this;
}

long Stopwatch::stop() {
	long duration = current_time() - start_time;
	total += duration;
	count++;
	return duration;
}

long Stopwatch::getTotalTime() {
	return total;
}

long Stopwatch::getCount() {
	return count;
}

TimeResolution Stopwatch::getResolution() {
	return resolution;
}

long Stopwatch::current_time() {
	switch (resolution) {
	case SECONDS:
		return std::chrono::duration_cast<std::chrono::seconds>(
		           std::chrono::system_clock::now().time_since_epoch())
		    .count();
	case MILLISECONDS:
		return std::chrono::duration_cast<std::chrono::milliseconds>(
		           std::chrono::system_clock::now().time_since_epoch())
		    .count();
	case NANOSECONDS:
		return std::chrono::duration_cast<std::chrono::nanoseconds>(
		           std::chrono::system_clock::now().time_since_epoch())
		    .count();
	default:
		std::cout << "WARNING: unexpected resolution in stopwatch " << resolution << std::endl;
		return 0;
	}
}

StopwatchPool::StopwatchPool(std::string name, TimeResolution resolution)
    : name(name), resolution(resolution) {}

Stopwatch& StopwatchPool::get(std::string name) {
	for (Stopwatch& w : watches) {
		if (w.name == name) {
			return w;
		}
	}
	watches.emplace_back(name, resolution);
	return watches.back();
}

void StopwatchPool::clear() {
	watches.clear();
}

void StopwatchPool::printAndClear() {
	printAll();
	clear();
}

int countDigits(long val) {
	// NB: assume val >= 0
	return val < 10 ? 1 : (1 + countDigits(val / 10));
}

inline void repeat(std::string s, int c) {
	while (c > 0) {
		std::cout << s;
		c--;
	}
}

inline void printLeftAlign(std::string s, int w) {
	std::cout << s;
	repeat(" ", w - s.length());
}
inline void printRightAlign(std::string s, int w) {
	repeat(" ", w - s.length());
	std::cout << s;
}

inline std::string to_string(TimeResolution resolution) {
	switch (resolution) {
	case SECONDS:
		return "s";
	case MILLISECONDS:
		return "ms";
	case NANOSECONDS:
		return "ns";
	default:
		return "<?>";
	}
}

void StopwatchPool::printAll() {

	int name_width = 4;
	long maxtime = 0;
	long maxcount = 0;
	for (Stopwatch& w : watches) {
		name_width = std::max(name_width, (int) w.name.length());
		maxtime = std::max(maxtime, w.total);
		maxcount = std::max(maxcount, w.count);
	}
	int time_width = std::max(5, countDigits(maxtime));
	int count_width = std::max(5, countDigits(maxcount));
	int average_width = std::max(time_width, 7);

	std::cout << "------ " << name << " (" << to_string(resolution) << ") ";
	repeat("-", name_width + time_width + count_width + average_width + 6 - name.length() -
	                to_string(resolution).length() - 10);
	std::cout << std::endl;

	printLeftAlign("NAME", name_width);
	std::cout << "  ";
	printLeftAlign("TOTAL", time_width);
	std::cout << "  ";
	printLeftAlign("COUNT", count_width);
	std::cout << "  ";
	printLeftAlign("AVERAGE", average_width);
	std::cout << std::endl;

	for (Stopwatch& w : watches) {
		printLeftAlign(w.name, name_width);
		std::cout << "  ";
		printRightAlign(std::to_string(w.total), time_width);
		std::cout << "  ";
		printRightAlign(std::to_string(w.count), count_width);
		std::cout << "  ";
		if (w.count == 0) {
			printRightAlign("-", average_width);
		} else {
			printRightAlign(std::to_string(w.total / w.count), average_width);
		}
		std::cout << std::endl;
	}

	repeat("-", name_width + time_width + count_width + average_width + 6);
	std::cout << std::endl;
}

static StopwatchPool* global_pool = nullptr;

StopwatchPool& StopwatchPool::global() {
	if (global_pool == nullptr) {
		global_pool = new StopwatchPool("GLOBAL TIMING");
	}
	return *global_pool;
}

} // namespace cartocrow