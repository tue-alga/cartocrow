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

#include "necklace.h"

namespace cartocrow::necklace_map {

Necklace::Necklace(std::shared_ptr<NecklaceShape> shape) : shape(std::move(shape)), beads() {}

void Necklace::sortBeads() {
	// TODO
	//for (const std::shared_ptr<Bead>& bead : beads) {
	//assert(bead != nullptr);
	//}

	std::sort(beads.begin(), beads.end(),
	          [](const std::shared_ptr<Bead>& a, const std::shared_ptr<Bead>& b) {
		          return a->feasible.from() < b->feasible.from();
	          });
}

} // namespace cartocrow::necklace_map
