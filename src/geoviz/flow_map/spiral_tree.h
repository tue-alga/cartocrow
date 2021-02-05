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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 28-10-2020
*/

#ifndef GEOVIZ_FLOW_MAP_SPIRAL_TREE_H
#define GEOVIZ_FLOW_MAP_SPIRAL_TREE_H

#include <deque>
#include <map>
#include <memory>
#include <queue>
#include <vector>

#include "geoviz/common/region.h"
#include "geoviz/flow_map/place.h"
#include "geoviz/flow_map/spiral.h"


namespace geoviz
{
namespace flow_map
{

struct Node
{
  using Ptr = std::shared_ptr<Node>;

  enum class Type
  {
    kRoot,
    kLeaf,
    kJoin,
    kSubdivision
  };

  explicit Node(const Place::Ptr& place = nullptr);

  Type GetType() const;

  bool IsSteiner() const;

  Place::Ptr place;
  Ptr parent;
  std::vector<Ptr> children;
}; // struct Node


class SpiralTree
{
 private:
  using NodeSet = std::vector<Node::Ptr>;

 public:
  using Ptr = std::shared_ptr<SpiralTree>;

  using NodeIterator = NodeSet::iterator;
  using NodeConstIterator = NodeSet::const_iterator;

  SpiralTree(const Point& root, const Number& restricting_angle_rad);

  inline Point GetRoot() const { return Point(CGAL::ORIGIN) - root_translation_; }

  inline Number GetRestrictingAngle() const { return restricting_angle_rad_; }

  NodeConstIterator nodes_begin() const { return nodes_.begin(); }
  NodeConstIterator nodes_end() const { return nodes_.end(); }

  NodeIterator nodes_begin() { return nodes_.begin(); }
  NodeIterator nodes_end() { return nodes_.end(); }

  void AddPlaces(const std::vector<Place::Ptr>& places);

  void AddObstacles(const std::vector<Region>& obstacles);

  void Compute();

  void SetRoot(const Point& root);

  void SetRestrictingAngle(const Number& restricting_angle_rad);

 private:
  void Clean();

  bool IsReachable(const PolarPoint& parent_point, const PolarPoint& child_point) const;

  Number restricting_angle_rad_;
  Vector root_translation_;

  std::vector<Node::Ptr> nodes_;  // Note that the positions of these nodes are offset by the position of the root.
  std::vector<Region> obstacles_;
}; // class SpiralTree

} // namespace flow_map
} // namespace geoviz

#endif //GEOVIZ_SPIRAL_TREE_H
