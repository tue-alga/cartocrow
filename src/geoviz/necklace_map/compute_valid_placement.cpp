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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 07-01-2020
*/

#include "compute_valid_placement.h"

#include <glog/logging.h>

#include "geoviz/necklace_map/detail/cycle_node.h"


namespace geoviz
{
namespace necklace_map
{

ComputeValidPlacement::ComputeValidPlacement
(
  const Number& scale_factor,
  const Number& aversion_ratio,
  const Number& buffer_rad /*= 0*/
) :
  aversion_ratio(aversion_ratio),
  scale_factor(scale_factor),
  buffer_rad(buffer_rad)
{}

void ComputeValidPlacement::operator()(Necklace::Ptr& necklace) const
{
  const Number necklace_radius = necklace->shape->ComputeRadius();

  // Compute the valid intervals.
  detail::ValidateScaleFactor validate(scale_factor, buffer_rad);
  const bool valid = validate(necklace);

  if (!valid)
    return;

  const Number EPSILON = 1e-7;
  const Number center_attraction_factor = 1;  // Note that while this factor is constant, it gives insight into the forces that pull the bead towards the center of the interval.

  const size_t num_beads = necklace->beads.size();
  for (int epoch = 0; epoch < 30; ++epoch)
  {
    for (size_t n = 0; n < num_beads; ++n)
    {
      Bead::Ptr& bead = necklace->beads[n];
      CHECK_NOTNULL(bead);

      int j1 = (n + num_beads - 1)%num_beads;
      int j2 = (n+1)%num_beads;

      // TODO(tvl) fix naming!

      Bead::Ptr& prev = necklace->beads[j1];
      Bead::Ptr& next = necklace->beads[j2];

      CircleRange r1(prev->angle_rad, bead->angle_rad);
      CircleRange r2(prev->angle_rad, next->angle_rad);
      CircleRange r3(bead->feasible->ComputeCentroid(), bead->angle_rad);

      const Number length_r1 = r1.ComputeLength();
      const Number length_r2 = r2.ComputeLength();
      const Number length_r3 = r3.ComputeLength();

      // determ. cov. radii (can be stored in vector, but then make sure these are also swapped if the elements are swapped...)
      const Number elem_radius = bead->covering_radius_scaled_rad;
      const Number prev_radius = prev->covering_radius_scaled_rad;
      const Number next_radius = next->covering_radius_scaled_rad;


      double d1 = prev_radius + elem_radius + buffer_rad;
      double d2 = length_r2 - next_radius - elem_radius - buffer_rad;
      if (j1 == j2)
        d2 = M_2xPI - next_radius - elem_radius - buffer_rad;
      double m =
        ((length_r3 < M_PI)
         ? (length_r1 - length_r3)
         : (length_r1 + (M_2xPI - length_r3)));
      double e = center_attraction_factor * m * d1 * d2 - aversion_ratio * (d1 + d2);
      double f = 2.0 * aversion_ratio - center_attraction_factor * ((d1 + m) * (d2 + m) - m * m);
      double g = center_attraction_factor * (d1 + d2 + m);
      double h = -center_attraction_factor;



      // solve h x^3 + g x^2 + f x + e == 0
      if (std::abs(h) < EPSILON && std::abs(g) < EPSILON)
      {
        double x = -e / f + prev->angle_rad;
        while (x > M_2xPI) x -= M_2xPI;
        while (x < 0) x += M_2xPI;
        if (!bead->feasible->IntersectsRay(x))
        {
          if (2.0 * length_r1 - (d1 + d2) > 0.0)
            x = bead->feasible->angle_cw_rad();
          else
            x = bead->feasible->angle_ccw_rad();
        }
        bead->angle_rad = x;
      }
      else
      {
        double q = (3.0 * h * f - g * g) / (9.0 * h * h);
        double r = (9.0 * h * g * f - 27.0 * h * h * e - 2.0 * g * g * g) / (54.0 * h * h * h);
        //double z = q * q * q + r * r;
        double rho = CGAL::sqrt(-q * q * q);
        if (std::abs(r) > rho) rho = std::abs(r);
        double theta = std::acos(r / rho);
        rho = std::pow(rho, 1.0 / 3.0);
        double x = -rho * std::cos(theta / 3.0) - g / (3.0 * h) + rho * CGAL::sqrt(3.0) * std::sin(theta/3.0);
        x += prev->angle_rad;
        while (x > M_2xPI) x -= M_2xPI;
        while (x < 0) x += M_2xPI;
        if (!bead->feasible->IntersectsRay(x)) {
          if (aversion_ratio * (2.0 * length_r1 - (d1 + d2)) + center_attraction_factor * (m - length_r1) * (length_r1 - d1) * (length_r1 - d2) > 0.0)
            x = bead->feasible->angle_cw_rad();
          else x = bead->feasible->angle_ccw_rad();
        }
        bead->angle_rad = x;
      }
    }

    SwapBeads(necklace);
  }

}

void ComputeValidPlacement::operator()(std::vector<Necklace::Ptr>& necklaces) const
{
  for (Necklace::Ptr& necklace : necklaces)
    (*this)(necklace);
}


ComputeValidPlacementFixedOrder::ComputeValidPlacementFixedOrder
(
  const Number& scale_factor,
  const Number& aversion_ratio,
  const Number& buffer_rad /*= 0*/
) :
  ComputeValidPlacement(scale_factor, aversion_ratio, buffer_rad)
{}


ComputeValidPlacementAnyOrder::ComputeValidPlacementAnyOrder
(
  const Number& scale_factor,
  const Number& aversion_ratio,
  const Number& min_separation /*= 0*/
) :
  ComputeValidPlacement(scale_factor, aversion_ratio, min_separation) {}

void ComputeValidPlacementAnyOrder::SwapBeads(Necklace::Ptr& necklace) const
{

  const Number necklace_radius = necklace->shape->ComputeRadius();

  // TODO(tvl) fix naming!

  // swapping action
  const size_t num_beads = necklace->beads.size();
  for (int n = 0; n < num_beads; n++)
  {
    int j = (n+1)%num_beads;

    Bead::Ptr& bead = necklace->beads[n];
    Bead::Ptr& next = necklace->beads[j];

    const Number elem_radius = bead->covering_radius_scaled_rad;
    const Number next_radius = next->covering_radius_scaled_rad;



    // Note that for the new angles, the buffers cancel each other out.
    double newAngle1 = next->angle_rad + next_radius - elem_radius;
    double newAngle2 = bead->angle_rad - elem_radius + next_radius;
    while (M_2xPI < newAngle1) newAngle1 -= M_2xPI;
    while (newAngle1 < 0) newAngle1 += M_2xPI;
    while (M_2xPI < newAngle2) newAngle2 -= M_2xPI;
    while (newAngle2 < 0) newAngle2 += M_2xPI;



    if (bead->feasible->IntersectsRay(newAngle1) && next->feasible->IntersectsRay(newAngle2))
    {
      double mid1 = bead->feasible->ComputeCentroid();
      double mid2 = next->feasible->ComputeCentroid();

      CircleRange r1(bead->angle_rad, mid1);
      if (r1.ComputeLength() > M_PI) r1 = CircleRange(mid1, bead->angle_rad);
      CircleRange r2(next->angle_rad, mid2);
      if (r2.ComputeLength() > M_PI) r2 = CircleRange(mid2, next->angle_rad);
      double dif1 = r1.ComputeLength() * r1.ComputeLength() + r2.ComputeLength() * r2.ComputeLength();

      r1 = CircleRange(newAngle1, mid1);
      if (r1.ComputeLength() > M_PI) r1 = CircleRange(mid1, newAngle1);
      r2 = CircleRange(newAngle2, mid2);
      if (r2.ComputeLength() > M_PI) r2 = CircleRange(mid2, newAngle2);
      double dif2 = r1.ComputeLength() * r1.ComputeLength() + r2.ComputeLength() * r2.ComputeLength();

      if (dif2 < dif1) {
        bead->angle_rad = newAngle1;
        next->angle_rad = newAngle2;

        std::swap(necklace->beads[n], necklace->beads[j]);
      }
    }
  }
}

} // namespace necklace_map
} // namespace geoviz
