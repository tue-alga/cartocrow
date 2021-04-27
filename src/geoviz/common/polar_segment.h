/*
The GeoViz library implements algorithmic geo-visualization methods,
developed at TU Eindhoven.
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 19-02-2021
*/

#ifndef GEOVIZ_COMMON_POLAR_SEGMENT_H
#define GEOVIZ_COMMON_POLAR_SEGMENT_H

#include <ostream>

#include "geoviz/common/circular_range.h"
#include "geoviz/common/core_types.h"
#include "geoviz/common/polar_line.h"
#include "geoviz/common/polar_point.h"


namespace geoviz
{

class PolarSegment : public PolarLine
{
 public:
  PolarSegment(const PolarPoint& point_1, const PolarPoint& point_2);

  Number FromT() const;

  Number ToT() const;

  Number R_min() const;

  Number R_max() const;

  bool IsLeft() const;
  bool IsRight() const;
  bool IsCollinear() const;

  bool ContainsFoot() const;

  bool ContainsT(const Number& t) const;

  // Note that the following methods shadow the methods of PolarLine, which are not virtual.

  bool ContainsR(const Number& R) const;

  bool ContainsPhi(const Number& phi) const;

  Number EvaluateR(const Number& t) const;

  Number EvaluatePhi(const Number& t) const;

  PolarPoint Evaluate(const Number& t) const;

  Number ComputeT(const Number& phi) const;

  template<class OutputIterator>
  int CollectT(const Number& R, OutputIterator t) const;

  template<class OutputIterator>
  int CollectPhi(const Number& R, OutputIterator phi) const;

  PolarPoint ComputeClosestToPole() const;

  const PolarLine& SupportingLine() const;

 private:
  // We will call the 't' value of the base PolarLine the 'distance'.
  Number ToDistance(const Number& t) const;
  Number ToT(const Number& distance) const;

  Number offset_, multiplier_;
}; // class PolarSegment


std::ostream& operator<<(std::ostream& os, const PolarSegment& line);

} // namespace geoviz

#include "polar_segment.inc"

#endif //GEOVIZ_COMMON_POLAR_SEGMENT_H
