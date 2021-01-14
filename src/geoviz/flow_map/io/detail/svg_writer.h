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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 16-10-2020
*/

#ifndef GEOVIZ_FLOW_MAP_IO_DETAIL_SVG_WRITER_H
#define GEOVIZ_FLOW_MAP_IO_DETAIL_SVG_WRITER_H

#include <string>
#include <unordered_map>
#include <vector>

#include <tinyxml2.h>

#include "geoviz/common/core_types.h"
#include "geoviz/common/region.h"
#include "geoviz/flow_map/flow_tree.h"
#include "geoviz/flow_map/spiral.h"
#include "geoviz/flow_map/io/write_options.h"


namespace geoviz
{
namespace flow_map
{
namespace detail
{

class SvgWriter
{
 public:
  SvgWriter
  (
    const std::vector<Region>& context,
    const FlowTree::Ptr& tree,
    const WriteOptions::Ptr& options,
    std::ostream& out
  );

  ~SvgWriter();

  void DrawContext();

  void DrawFlow();

  void DrawNodes();

 private:
  void OpenSvg();

  void CloseSvg();

  void ComputeBoundingBox();

  void AddDropShadowFilter();

  void DrawSpiral(const Spiral& spiral, const Vector& offset, const PolarPoint& parent);

  void DrawRoots();

  void DrawLeaves();

  void DrawJoinNodes();

  void DrawSubdivisionNodes();

  const std::vector<Region>& context_;
  const FlowTree::Ptr& tree_;
  std::ostream& out_;

  WriteOptions::Ptr options_;

  Box bounding_box_;
  Box bounding_box_spirals_;
  double unit_px_;
  std::string transform_matrix_;

  tinyxml2::XMLPrinter printer_;
}; // class SvgWriter

} // namespace detail
} // namespace flow_map
} // namespace geoviz

#endif //GEOVIZ_FLOW_MAP_IO_DETAIL_SVG_WRITER_H
