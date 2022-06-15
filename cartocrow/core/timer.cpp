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

#include <iostream>

namespace cartocrow {

Timer::Timer() {
	reset();
}

void Timer::reset() {
	m_stamps.clear();
	m_descriptions.clear();
	m_stamps.push_back(clock());
}

/// Returns the <code>i</code>-th step.
/**
 * \return A pair containing the description and the duration.
 */
std::pair<std::string, double> Timer::operator[](const size_t& i) const {
	return std::make_pair(m_descriptions[i], toSeconds(m_stamps[i + 1] - m_stamps[i]));
}

double Timer::stamp(const std::string& description) {
	m_stamps.push_back(clock());
	m_descriptions.push_back(description);
	return toSeconds(m_stamps.back() - m_stamps[m_stamps.size() - 1]);
}

double Timer::peek() const {
	return toSeconds(clock() - m_stamps[m_stamps.size() - 1]);
}

double Timer::span() const {
	return double(m_stamps.back() - m_stamps.front()) / CLOCKS_PER_SEC;
}

size_t Timer::size() const {
	return m_descriptions.size();
}

void Timer::output() const {
	for (size_t i = 0; i < size(); ++i) {
		std::cout << this->operator[](i).first << ": " << this->operator[](i).second << " s\n";
	}
}

double Timer::toSeconds(clock_t time) {
	return double(time) / CLOCKS_PER_SEC;
}

} // namespace cartocrow
