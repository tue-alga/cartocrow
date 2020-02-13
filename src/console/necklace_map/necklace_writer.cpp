/*
The Necklace Map console application implements the algorithmic
geo-visualization method by the same name, developed by
Bettina Speckmann and Kevin Verbeek at TU Eindhoven
(DOI: 10.1109/TVCG.2010.180 & 10.1142/S021819591550003X).
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 29-01-2020
*/

#include "necklace_writer.h"

#include <glog/logging.h>


namespace geoviz
{

NecklaceWriter::NecklaceWriter()
{}

bool NecklaceWriter::Write
(
  const std::vector<MapElement::Ptr>& elements,
  const std::vector<Necklace::Ptr>& necklaces,
  const Number& scale_factor,
  const WriterOptions::Ptr& options,
  std::ostream& out
) const
{
  detail::NecklaceWriter writer(elements, necklaces, scale_factor, options, out);

  // The order of drawing the features determines their stacking order, i.e. the last one will be on top.
  writer.DrawRegions();
  writer.DrawNecklaces();
  writer.DrawValidIntervals();
  writer.DrawRegionAngles();
  writer.DrawBeadAngles();
  writer.DrawFeasibleIntervals();
  writer.DrawBeads();
}

} // namespace geoviz
