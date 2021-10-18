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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 02-09-2020
*/

#ifndef CARTOCROW_FLOW_MAP_IO_SVG_WRITER_H
#define CARTOCROW_FLOW_MAP_IO_SVG_WRITER_H

#include <ostream>

#include "cartocrow/core/core_types.h"
#include "cartocrow/core/region.h"
#include "cartocrow/flow_map/flow_tree.h"
#include "cartocrow/flow_map/io/write_options.h"

namespace cartocrow {
namespace flow_map {

class SvgWriter {
  public:
	SvgWriter();

	bool Write(const std::vector<Region>& context, const std::vector<Region>& obstacles,
	           const FlowTree::Ptr& tree, const WriteOptions::Ptr& options, std::ostream& out) const;
}; // class SvgWriter

} // namespace flow_map
} // namespace cartocrow

#endif //CARTOCROW_FLOW_MAP_IO_SVG_WRITER_H
