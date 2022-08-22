/*
The Necklace Map library implements the algorithmic geo-visualization
method by the same name, developed by Bettina Speckmann and Kevin Verbeek
at TU Eindhoven (DOI: 10.1109/TVCG.2010.180 & 10.1142/S021819591550003X).
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 10-09-2019
*/

#include "necklace_map.h"

#include <stdexcept>

namespace cartocrow::necklace_map {

NecklaceMap::NecklaceHandle::NecklaceHandle(size_t index) : m_index(index) {}

NecklaceMap::NecklaceMap(const std::shared_ptr<RegionMap> map) : m_map(map) {}

NecklaceMap::NecklaceHandle NecklaceMap::addNecklace(std::unique_ptr<NecklaceShape> shape) {
	m_necklaces.emplace_back(std::move(shape));
	return NecklaceHandle{m_necklaces.size() - 1};
}

void NecklaceMap::addBead(std::string regionName, Number<Inexact> value, NecklaceHandle& handle) {
	if (!m_map->contains(regionName)) {
		throw std::runtime_error("Tried to add bead for non-existing region \"" + regionName + "\"");
	}
	Necklace& necklace = m_necklaces[handle.m_index];
	necklace.beads.push_back(std::make_shared<Bead>(&(m_map->at(regionName)), value, handle.m_index));
}

Parameters& NecklaceMap::parameters() {
	return m_parameters;
}

void NecklaceMap::compute() {
	// compute the feasible region for each bead
	for (auto& necklace : m_necklaces) {
		for (auto& bead : necklace.beads) {
			(*ComputeFeasibleInterval::construct(m_parameters))(bead, necklace);
		}
	}

	// compute the scaling factor
	m_scaleFactor = (*ComputeScaleFactor::construct(m_parameters))(m_necklaces);

	// compute valid placement
	(*ComputeValidPlacement::construct(m_parameters))(m_scaleFactor, m_necklaces);
}

Number<Inexact> NecklaceMap::scaleFactor() {
	return m_scaleFactor;
}

} // namespace cartocrow::necklace_map
