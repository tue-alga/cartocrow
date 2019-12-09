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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 09-12-2019
*/

#ifndef GEOVIZ_NECKLACE_MAP_NECKLACE_INTERVAL_H
#define GEOVIZ_NECKLACE_MAP_NECKLACE_INTERVAL_H

#include <memory>

#include "geoviz/common/core_types.h"
#include "geoviz/necklace_map/necklace.h"

namespace geoviz
{
namespace necklace_map
{

class NecklaceInterval
{
 public:
  NecklaceInterval
  (
    const std::shared_ptr<NecklaceType>& necklace,
    const Number& angle_cw_rad,
    const Number& angle_ccw_rad
  );

  const std::shared_ptr<NecklaceType>& necklace() const;
  const Number& angle_cw_rad() const;
  const Number& angle_ccw_rad() const;

  bool IsValid() const;

  bool IntersectsRay(const Number& angle_rad) const;

  virtual Number ComputeOrder() const = 0;

 protected:
  std::shared_ptr<NecklaceType> necklace_;
  Number angle_cw_rad_; // Internally, this angle is adjusted to be in the range [0, 2*pi).
  Number angle_ccw_rad_; // For convenience, this angle is adjusted to be in the range [angle_from_rad, angle_from_rad+2*pi).
}; // class NecklaceInterval


class IntervalCentroid : public NecklaceInterval
{
 public:
  IntervalCentroid
  (
    const std::shared_ptr<NecklaceType>& necklace,
    const Number& angle_cw_rad,
    const Number& angle_ccw_rad
  );

  Number ComputeOrder() const; // Order based on centroid.
}; // class IntervalCentroid


class IntervalWedge : public NecklaceInterval
{
 public:
  IntervalWedge
  (
    const std::shared_ptr<NecklaceType>& necklace,
    const Number& angle_cw_rad,
    const Number& angle_ccw_rad
  );

  // Order based on begin.
  Number ComputeOrder() const;
}; // class IntervalWedge

} // namespace necklace_map
} // namespace geoviz

#endif //GEOVIZ_NECKLACE_MAP_NECKLACE_INTERVAL_H
