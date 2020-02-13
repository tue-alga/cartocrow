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

#ifndef CONSOLE_NECKLACE_MAP_DETAIL_NECKLACE_WRITER_H
#define CONSOLE_NECKLACE_MAP_DETAIL_NECKLACE_WRITER_H

#include <string>
#include <unordered_map>
#include <vector>

#include <tinyxml2.h>

#include "geoviz/common/core_types.h"
#include "geoviz/necklace_map/map_element.h"
#include "geoviz/necklace_map/necklace.h"


namespace geoviz
{

struct WriterOptions
{
  using Ptr = std::shared_ptr<WriterOptions>;

  static Ptr Default();
  static Ptr Debug();

  int pixel_width;

  int region_precision;
  double region_opacity;
  double bead_opacity;

  bool draw_necklace_curve;
  bool draw_bead_ids;

  bool draw_feasible_intervals;
  bool draw_valid_intervals;
  bool draw_region_angles;
  bool draw_bead_angles;
}; // struct WriterOptions


namespace detail
{

class NecklaceWriter
{
 public:
  using Bead = necklace_map::Bead;
  using MapElement = necklace_map::MapElement;
  using Necklace = necklace_map::Necklace;

  NecklaceWriter
  (
    const std::vector<MapElement::Ptr>& elements,
    const std::vector<Necklace::Ptr>& necklaces,
    const Number& scale_factor,
    const WriterOptions::Ptr& options,
    std::ostream& out
  );

  ~NecklaceWriter();

  void DrawRegions();

  void DrawNecklaces();

  void DrawBeads();

  void DrawFeasibleIntervals();

  void DrawValidIntervals();

  void DrawRegionAngles();

  void DrawBeadAngles();

 private:
  using NecklaceShape = necklace_map::NecklaceShape;
  using CircleNecklace = necklace_map::CircleNecklace;
  using CurveNecklace = necklace_map::CurveNecklace;
  using GenericNecklace = necklace_map::GenericNecklace;
  using BeadShapeMap = std::unordered_map<Bead::Ptr, NecklaceShape::Ptr>;

  void OpenSvg();

  void CloseSvg();

  void ComputeBoundingBox();

  void CreateBeadIntervalShapes();

  void AddDropShadowFilter();

  void DrawBeadIds();

  const std::vector<MapElement::Ptr>& elements_;
  const std::vector<Necklace::Ptr>& necklaces_;
  const Number scale_factor_;
  std::ostream& out_;

  WriterOptions::Ptr options_;

  Box bounding_box_;
  double unit_px_;
  std::string transform_matrix_;

  BeadShapeMap bead_shape_map_;

  tinyxml2::XMLPrinter printer_;
}; // class NecklaceWriter

} // namespace detail
} // namespace geoviz

#endif //CONSOLE_NECKLACE_MAP_DETAIL_NECKLACE_WRITER_H
