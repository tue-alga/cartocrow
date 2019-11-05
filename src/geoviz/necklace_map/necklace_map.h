/*
The Necklace Map library implements the algorithmic geo-visualization
method by the same name, developed by Bettina Speckmann and Kevin Verbeek
at TU Eindhoven (DOI: 10.1109/TVCG.2010.180 & 10.1142/S021819591550003X).
Copyright (C) 2019  Netherlands eScience Center

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

/// @file
/// @brief Interfaces to necklace map algorithm.
#ifndef GEOVIZ_NECKLACE_MAP_NECKLACE_MAP_H
#define GEOVIZ_NECKLACE_MAP_NECKLACE_MAP_H

#include <string>
#include <tuple>
#include <vector>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Polygon_set_2.h>


namespace geoviz
{

// The geometric data types are taken from CGAL where possible.
using Kernel = CGAL::Exact_predicates_inexact_constructions_kernel;
using Number = Kernel::FT;
using Point = CGAL::Point_2<Kernel>;
using Circle = CGAL::Circle_2<Kernel>;
using Polygon = CGAL::Polygon_2<Kernel>;

namespace necklace_map
{

// Most methods take as input a set of geographical regions.
// Each region represents a country or other geographically significant shape.
// Each such shape can be a collection of disconnected polygons (e.g. an island group),
// where each polygon may contain holes (e.g. lakes).
using Shape = CGAL::Polygon_set_2<Kernel>;
using Subshape = Shape::Polygon_with_holes_2;

// Additional notes on input regions.
// Note that the Necklace Map algorithms ignore these holes for all intents and purposes.
// Also note that polygons may intersect within each region with minor effect on the output,
// although polygon self-intersection will produce undefined results.
// Similarly, intersection between different regions affects the output without causing crashes.
// In some cases, a multi-polygon region is simplied to its convex hull,
// but when determining the centroid, the centroid of the polygon set is used.
struct Region
{
  // Check whether the ID is not empty, the shape is a valid polygon, and the value is at least 0.
  // The shape is valid if all its polygons have counter-clockwise outer boundary that is not self-intersecting
  // DISREGARD?: and the shape does not have a pair of polygons that intersect.
  // Note that polygons may be degenerate, i.e. enclosing an empty region such as a single point.
  // Strict validation also requires the value to be strictly larger than 0.
  bool isValid(const bool strict = true) const;

  // For regions with only one polygon, the extent is the outer boundary of that polygon,
  // otherwise, the extent is the convex hull of the polygon set.
  Polygon getExtent() const;

  std::string id;
  Shape shape;
  Number value;
}; // struct Region


// A necklace map has disk-shaped glyphs with their center on a necklace.
// Several types of necklaces are allowed:
// a full circle,
// a connected component of a circle,
// or a quadratic Bezier curve.
struct NecklaceType
{
  virtual Point& getCenter() const = 0;
}; // struct NecklaceType

struct CircleNecklace : public NecklaceType
{
  Point& getCenter() const;

  Circle shape;
}; // struct CircleNecklace

// If begin_rad and end_rad are the same, the full circle is part of the necklace;
// otherwise, the necklace uses the part of the circle that starts at begin_rad and 'moves' counter-clockwise to end_rad.
// Both begin_rad and end_rad are values of radian unit.
// TODO(tvl) should the necklace contain methods for adapting the 1D solution to the 2D solution?
// This would mainly come into play for Bezier necklaces. For impl: check the 30x for loop.
// Another case would be when a Glyph contains the necklace center;
// in this case, the scale factor should be reduced such that the glyph does not contain the necklace center
// (could this cause recursive failure and if so, in which cases? This *may* actually prove scientifically interesting).
struct CurveNecklace : public CircleNecklace
{
  Number begin_rad, end_rad;
}; // struct CurveNecklace

struct BezierNecklace : public NecklaceType
{
  Point& getCenter() const;

  Point center;
  std::vector<Point> points;
}; // struct BezierNecklace


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
    const std::vector<Shape>& extents,
    const NecklaceType& necklace,
    std::vector<NecklaceInterval>& intervals
  ) const = 0;
}; // struct IntervalGenerator

struct CentroidIntervalGenerator : public  IntervalGenerator
{
  void operator()
  (
    const std::vector<Shape>& extents,
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
    const std::vector<Shape>& extents,
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

  void toCenter(const Circle& necklace, const Number& angle, Point& center) const;

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



// Add a note on multiple necklaces that the different necklaces may generate overlapping glyphs;
// these can often be corrected by manually tuning the buffer and attraction-repulsion parameters.
// We don't fix this, or even check for occurrence of overlapping glyphs.




} // namespace necklace_map








/**
 * Dummy method for running the necklace map algorithm.
 * @return a dummy return string.
 */
std::string proc_necklace_map();

/// Test for generating doxygen documentation.
class TestDoc
{
 public:
  /// Default constructor
  /**
   * More documentation than the brief summary above...
   */
  TestDoc() {}
};

} // namespace geoviz

#endif //GEOVIZ_NECKLACE_MAP_NECKLACE_MAP_H
