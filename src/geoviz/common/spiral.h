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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 16-10-2020
*/

#ifndef GEOVIZ_COMMON_SPIRAL_H
#define GEOVIZ_COMMON_SPIRAL_H

#include <ostream>

#include "geoviz/common/core_types.h"
#include "geoviz/common/polar_point.h"


namespace geoviz
{

class Spiral
{
 public:
  Spiral(const PolarPoint& anchor, const Number& angle_rad);

  Spiral(const PolarPoint& point_1, const PolarPoint& point_2);

  const PolarPoint& anchor() const;

  const Number& angle_rad() const;

  bool IsLeft() const;
  bool IsRight() const;
  bool IsCollinear() const;

  Number EvaluateR(const Number& t) const;

  Number EvaluatePhi(const Number& t) const;

  PolarPoint Evaluate(const Number& t) const;

  Number ComputeT(const Number& R) const;

  Number ComputePhi(const Number& R) const;

  Number SampleT(const Number& phi) const;

  Number SampleR(const Number& phi) const;

  Number ComputePeriod() const;

 private:
  PolarPoint anchor_;

  Number angle_rad_;
}; // class Spiral


std::ostream& operator<<(std::ostream& os, const Spiral& spiral);

} // namespace geoviz

#endif //GEOVIZ_COMMON_SPIRAL_H
