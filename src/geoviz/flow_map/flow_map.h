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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 10-09-2019
*/

#ifndef GEOVIZ_FLOW_MAP_FLOW_MAP_H
#define GEOVIZ_FLOW_MAP_FLOW_MAP_H

#include <string>

#include <geoviz/common/core_types.h>
#include <geoviz/common/region.h>

#include "geoviz/flow_map/flow_tree.h"
#include "geoviz/flow_map/parameters.h"
#include "geoviz/flow_map/place.h"
#include "geoviz/flow_map/io/data_reader.h"
#include "geoviz/flow_map/io/svg_reader.h"
#include "geoviz/flow_map/io/svg_writer.h"


namespace geoviz
{
namespace flow_map
{

void ComputeFlowMap
(
  const Parameters& parameters,
  const std::vector<Place::Ptr>& places,
  const size_t index_root,
  FlowTree::Ptr& tree
);

} // namespace flow_map
} // namespace geoviz

#endif //GEOVIZ_FLOW_MAP_FLOW_MAP_H
