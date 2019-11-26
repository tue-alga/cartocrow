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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 10-09-19
*/

#ifndef GEOVIZ_NECKLACE_MAP_NECKLACE_MAP_H
#define GEOVIZ_NECKLACE_MAP_NECKLACE_MAP_H

#include <string>
#include <tuple>
#include <vector>

#include "geoviz/common/core_types.h"
#include "geoviz/necklace_map/necklace.h"
#include "geoviz/necklace_map/region.h"


// TODO(tvl) this should probably end up as forwarding file, to functional parts that may each have their own subdirectory...



namespace geoviz
{
namespace necklace_map
{


struct NecklaceInterval
{
  // If multiple intervals have the same order value, these can be placed in any order relative to each other (in practice: order as encountered).
  virtual Number toOrder() const = 0;

  Number begin_rad, end_rad;
}; // struct NecklaceInterval

struct CentroidInterval : public NecklaceInterval
{
  // Order based on centroid.
  Number toOrder() const;
}; // struct CentroidInterval

struct WedgeInterval : public NecklaceInterval
{
  // Order based on begin.
  Number toOrder() const;
}; // struct WedgeInterval


struct IntervalGenerator
{
  virtual void operator()
  (
    const std::vector<Polygon>& extents,
    const NecklaceType& necklace,
    std::vector<NecklaceInterval>& intervals
  ) const = 0;
}; // struct IntervalGenerator

struct CentroidIntervalGenerator : public  IntervalGenerator
{
  void operator()
  (
    const std::vector<Polygon>& extents,
    const NecklaceType& necklace,
    std::vector<NecklaceInterval>& intervals
  ) const;
}; // struct CentroidIntervalGenerator

// Fallback to centroid interval if:
// * Extent contains necklace center
// *
// TODO(tvl) implement 'toggle' to always use centroid interval for empty-interval regions (e.g. point regions).
struct WedgeIntervalGenerator : public  IntervalGenerator
{
  void operator()
  (
    const std::vector<Polygon>& extents,
    const NecklaceType& necklace,
    std::vector<NecklaceInterval>& intervals
  ) const;
}; // struct WedgeIntervalGenerator


struct GlyphScaler
{
  GlyphScaler(const Number& buffer);

  void operator()
  (
    const std::vector<Number>& values,
    Number& scale_factor
  ) const;

  void toRadius(const Number& value, const Number& scale_factor, Number& radius) const;

  const Number& buffer;  // Minimum distance between two glyphs (while determining scale factor, the buffer is added to s*sqrt(z) for the glyph radius).
  // TODO(tvl) check >= 0, design decent max (should depend on number of glyphs and should at the very least be half the necklace length).
}; // struct GlyphScaler




// Add a note on multiple necklaces that the different necklaces may generate overlapping glyphs;
// these can often be corrected by manually tuning the buffer and attraction-repulsion parameters.
// We don't fix this, or even check for occurrence of overlapping glyphs.

struct GlyphPositioner
{
  GlyphPositioner(const Number& centroid_attraction_ratio);

  virtual void operator()
  (
    const std::vector<NecklaceInterval>& intervals,
    const std::vector<Number>& values,
    const Number& scale_factor,
    const std::vector<Number>& angles_rad
  ) const = 0;

  void toCenter(const Circle& necklace, const Number& angle, Point& center) const;  // TODO(tvl) move to necklace: depends on necklace type...

  const Number& centroid_attraction_ratio;  // Ratio between attraction to interval center (1) and repulsion from neighboring glyphs (0).
}; // struct GlyphPositioner

// TODO(tvl) Note, the fixed order positioner has implemented incorrectly; the paper version was corrected, so use this instead!
struct FixedOrderPositioner : public GlyphPositioner
{
  FixedOrderPositioner(const Number& centroid_attraction_ratio);

  virtual void operator()
  (
    const std::vector<NecklaceInterval>& intervals,
    const std::vector<Number>& values,
    const Number& scale_factor,
    const std::vector<Number>& angles_rad
  ) const;
}; // struct FixedOrderPositioner

struct AnyOrderPositioner : public GlyphPositioner
{
  AnyOrderPositioner(const Number& centroid_attraction_ratio);

  virtual void operator()
    (
      const std::vector<NecklaceInterval>& intervals,
      const std::vector<Number>& values,
      const Number& scale_factor,
      const std::vector<Number>& angles_rad
    ) const;
}; // struct AnyOrderPositioner

// Heuristic any-order positioner.
struct HeuristicPositioner : public GlyphPositioner
{
  HeuristicPositioner(const Number& centroid_attraction_ratio);

  virtual void operator()
    (
      const std::vector<NecklaceInterval>& intervals,
      const std::vector<Number>& values,
      const Number& scale_factor,
      const std::vector<Number>& angles_rad
    ) const;
}; // struct HeuristicPositioner


} // namespace necklace_map

} // namespace geoviz

#endif //GEOVIZ_NECKLACE_MAP_NECKLACE_MAP_H
