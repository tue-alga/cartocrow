/*
The Necklace Map library implements the algorithmic geo-visualization
method by the same name, developed by Bettina Speckmann and Kevin Verbeek
at TU Eindhoven (DOI: 10.1109/TVCG.2010.180 & 10.1142/S021819591550003X).
Copyright (C) 2019  Netherlands eScience Center and TU Eindhoven

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


namespace geoviz
{
namespace necklace_map
{

Number ComputeScaleFactor
(
  const Parameters& parameters,
  std::vector<MapElement::Ptr>& elements,
  std::vector<Necklace::Ptr>& necklaces
)
{
  // Create a bead per necklace that an element is part of.
  for (Necklace::Ptr& necklace : necklaces)
    necklace->beads.clear();
  for(MapElement::Ptr& element : elements)
    element->InitializeBeads(parameters);

  // Generate intervals based on the regions and necklaces.
  (*ComputeFeasibleInterval::New(parameters))(elements);

  // Compute the scaling factor.
  const Number scale_factor = (*ComputeScaleFactor::New(parameters))(necklaces);

  ComputePlacement(parameters, scale_factor, necklaces);

  return scale_factor;
}

void ComputePlacement
(
  const Parameters& parameters,
  const Number& scale_factor,
  std::vector<Necklace::Ptr>& necklaces
)
{
  // Compute valid placement.
  (*ComputeValidPlacement::New(parameters))(scale_factor, necklaces);
}

} // namespace necklace_map

} // namespace geoviz