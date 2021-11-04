/*
The Flow Map library implements the algorithmic geo-visualization
method by the same name, developed by Kevin Verbeek, Kevin Buchin,
and Bettina Speckmann at TU Eindhoven
(DOI: 10.1007/s00453-013-9867-z & 10.1109/TVCG.2011.202).
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

#ifndef CARTOCROW_FLOW_MAP_FLOW_MAP_H
#define CARTOCROW_FLOW_MAP_FLOW_MAP_H

#include <string>

#include <cartocrow/core/core_types.h>
#include <cartocrow/core/region.h>

#include "cartocrow/flow_map/flow_tree.h"
#include "cartocrow/flow_map/io/data_reader.h"
#include "cartocrow/flow_map/io/svg_reader.h"
#include "cartocrow/flow_map/io/svg_writer.h"
#include "cartocrow/flow_map/parameters.h"
#include "cartocrow/flow_map/place.h"

namespace cartocrow::flow_map {

/// Compute the flow map.
/**
 * This consists of three major steps: computing the spiral tree, subdividing
 * and thickening the tree, and improving the smoothness and clarity of the
 * tree.
 *
 * @param parameters The parameter settings to apply to the computations.
 * @param places The places (e.g. root and leaf nodes) on the flow map.
 * @param index_root The index of the root node of the flow map.
 * @param obstacles The polygonal obstacles that must be avoided by the flow
 * tree.
 * @param[out] tree The flow tree that shows the flow from the root to the
 * leaf nodes.
 */
void computeFlowMap(const Parameters& parameters, const std::vector<Place::Ptr>& places,
                    const size_t index_root, const std::vector<Region>& obstacles,
                    FlowTree::Ptr& tree);

} // namespace cartocrow::flow_map

#endif //CARTOCROW_FLOW_MAP_FLOW_MAP_H
