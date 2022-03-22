/*
The Necklace Map library implements the algorithmic
geo-visualization method by the same name, developed by
Bettina Speckmann and Kevin Verbeek at TU Eindhoven
(DOI: 10.1109/TVCG.2010.180 & 10.1142/S021819591550003X).
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
*/

#ifndef CARTOCROW_FLOW_MAP_IO_IPE_READER_H
#define CARTOCROW_FLOW_MAP_IO_IPE_READER_H

#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include <ipeattributes.h>
#include <ipebase.h>
#include <ipelib.h>

#include "cartocrow/core/polygon.h"
#include "cartocrow/core/region.h"

#include "cartocrow/flow_map/place.h"

namespace cartocrow::flow_map {

/// A reader for flow map input in Ipe format.
/**
 * This reads regions and necklaces from the Ipe file as follows:
 * * stroked (non-filled) paths are interpreted as necklaces;
 * * stroked-and-filled paths are interpreted as regions;
 * * each region is supposed to have one single text label in it, which
 *   indicates the region's name;
 * * to use more than one necklace, put each necklace on a separate layer, along
 *   with the regions that should be mapped to that necklace.
 */
class IpeReader {
  public:
	/// Constructs an Ipe reader.
	IpeReader();

	/// Reads data from the Ipe file with the given filename.
	/**
	 * Stores the data into output parameters \c regions, \c obstacles, and \c places.
	 */
	bool readFile(const std::filesystem::path& filename, std::vector<Region>& regions,
	              std::vector<Region>& obstacles, std::vector<Place::Ptr>& places);
};

} // namespace cartocrow::flow_map

#endif //CARTOCROW_FLOW_MAP_IO_IPE_READER_H
