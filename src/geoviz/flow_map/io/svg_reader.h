/*
The Flow Map library implements the algorithmic geo-visualization
method by the same name, developed by Kevin Verbeek, Kevin Buchin,
and Bettina Speckmann at TU Eindhoven
(DOI: 10.1007/s00453-013-9867-z & 10.1109/TVCG.2011.202).
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 04-09-2020
*/

#ifndef GEOVIZ_FLOW_MAP_IO_SVG_READER_H
#define GEOVIZ_FLOW_MAP_IO_SVG_READER_H

#include <iostream>
#include <string>
#include <vector>

#include <gflags/gflags.h>

#include "geoviz/flow_map/place.h"
#include "geoviz/common/region.h"


namespace geoviz
{
namespace flow_map
{

class SvgReader
{
 public:
  SvgReader();

  bool ReadFile
  (
    const std::string& filename,
    std::vector<geoviz::Region>& context,
    std::vector<Place::Ptr>& places,
    int max_retries = 2
  );

  bool Parse
  (
    const std::string& input,
    std::vector<geoviz::Region>& context,
    std::vector<Place::Ptr>& places
  );
}; // class SvgReader

} // namespace flow_map
} // namespace geoviz

#endif //GEOVIZ_FLOW_MAP_IO_SVG_READER_H
