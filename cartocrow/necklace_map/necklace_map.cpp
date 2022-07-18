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

NecklaceMap::NecklaceHandle::NecklaceHandle(Necklace* necklace) : m_necklace(necklace) {}

NecklaceMap::NecklaceMap(const std::shared_ptr<RegionMap> map) : m_map(map) {}

NecklaceMap::NecklaceHandle NecklaceMap::addNecklace(std::unique_ptr<NecklaceShape> shape) {
	m_necklaces.push_back(std::make_unique<Necklace>(std::move(shape)));
	return NecklaceHandle{m_necklaces.back().get()};
}

void NecklaceMap::addBead(std::string regionName, Number<Inexact> value, NecklaceHandle& necklace) {
	if (!m_map->contains(regionName)) {
		throw std::runtime_error("Tried to add bead for non-existing region \"" + regionName + "\"");
	}
	necklace.m_necklace->beads.push_back(
	    std::make_shared<Bead>(&(m_map->at(regionName)), value, necklace.m_necklace));
}

Parameters& NecklaceMap::parameters() {
	return m_parameters;
}

void NecklaceMap::compute() {
	for (auto& necklace : m_necklaces) {
		necklace->beads.clear();
		for (auto& bead : necklace->beads) {
			(*ComputeFeasibleInterval::construct(m_parameters))(bead);
		}
	}
	// TODO TODO TODO
}

Number<Inexact> NecklaceMap::scaleFactor() {
	return m_scaleFactor;
}

// TODO remove the following methods, kept for now for reference

/*Number<Inexact> computeScaleFactor(const Parameters& parameters,
                                   std::vector<MapElement::Ptr>& elements,
                                   std::vector<Necklace::Ptr>& necklaces) {
	// create a bead per necklace that an element is part of
	for (Necklace::Ptr& necklace : necklaces) {
		necklace->beads.clear();
	}
	for (MapElement::Ptr& element : elements) {
		element->InitializeBead(parameters);
	}

	// generate intervals based on the regions and necklaces
	(*ComputeFeasibleInterval::New(parameters))(elements);

	// compute the scaling factor
	const Number scale_factor = (*ComputeScaleFactor::New(parameters))(necklaces);

	// compute valid placement
	(*ComputeValidPlacement::New(parameters))(scale_factor, necklaces);

	return scale_factor;
}*/

/*void computePlacement(const Parameters& parameters, const Number<Inexact>& scale_factor,
                      std::vector<MapElement::Ptr>& elements, std::vector<Necklace::Ptr>& necklaces) {
	// create a bead per necklace that an element is part of
	for (Necklace::Ptr& necklace : necklaces) {
		necklace->beads.clear();
	}
	for (MapElement::Ptr& element : elements) {
		element->InitializeBead(parameters);

		if (element->bead) {
			CHECK_NOTNULL(element->input_feasible);

			element->bead->angle_rad = element->input_angle_rad;
			element->bead->feasible = element->input_feasible;
		}
	}

	// compute valid placement
	(*ComputeValidPlacement::New(parameters))(scale_factor, necklaces);
}*/

} // namespace cartocrow::necklace_map
