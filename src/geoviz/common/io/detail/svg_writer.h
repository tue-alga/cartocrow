/*
The GeoViz library implements algorithmic geo-visualization methods,
developed at TU Eindhoven.
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 09-03-2021
*/

#ifndef GEOVIZ_COMMON_IO_DETAIL_SVG_WRITER_H
#define GEOVIZ_COMMON_IO_DETAIL_SVG_WRITER_H

#include <iostream>
#include <string>
#include <vector>

#include <tinyxml2.h>

#include "geoviz/common/core_types.h"
#include "geoviz/common/polar_line.h"
#include "geoviz/common/polar_segment.h"
#include "geoviz/common/spiral.h"
#include "geoviz/common/spiral_segment.h"
#include "geoviz/common/io/write_options.h"


namespace geoviz
{
namespace detail
{

class SvgWriter
{
 public:
  SvgWriter
  (
    const std::vector<PolarPoint>& points,
    const std::vector<Spiral>& spirals,
    const std::vector<SpiralSegment>& spiral_segments,
    const std::vector<PolarLine>& lines,
    const std::vector<PolarSegment>& line_segments,
    const WriteOptions::Ptr& options,
    std::ostream& out
  );

  ~SvgWriter();

  void DrawSpirals();

  void DrawLines();

  void DrawPoints();

 private:
  void OpenSvg();

  void CloseSvg();

  void ComputeBoundingBox();

  void DrawSpiral(const Spiral& spiral, const Number& R_max, const Number& R_min = 0);

  void DrawLine(const PolarLine& line, const Number& t_from, const Number& t_to);

  const std::vector<PolarPoint>& points_;
  const std::vector<Spiral>& spirals_;
  const std::vector<SpiralSegment>& spiral_segments_;
  const std::vector<PolarLine>& lines_;
  const std::vector<PolarSegment>& line_segments_;

  std::ostream& out_;

  WriteOptions::Ptr options_;

  Box bounding_box_;
  double unit_px_;
  std::string transform_matrix_;

  tinyxml2::XMLPrinter printer_;
}; // class SvgWriter

} // namespace detail
} // namespace geoviz

#endif //GEOVIZ_COMMON_IO_DETAIL_SVG_WRITER_H
