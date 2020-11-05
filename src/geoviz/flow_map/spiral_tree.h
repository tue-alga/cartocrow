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

#include "geoviz/flow_map/place.h"
#include "geoviz/flow_map/spiral.h"


namespace geoviz
{
namespace flow_map
{

class SpiralTree
{
 public:
  using Ptr = std::shared_ptr<SpiralTree>;

  struct Node;
  struct Arc;

  using NodePtr = std::shared_ptr<Node>;
  using ArcPtr = std::shared_ptr<Arc>;

  struct Node
  {
    enum class Type { kRoot, kJoin, kLeaf, kSubdivision };  // TODO(tvl) move out of this specific node struct to share with the flow tree node struct.

    Node(const Type type, const PolarPoint& relative_position, const Place::Ptr& place = nullptr);

    Number ComputeOrder(const Number& restricting_angle_rad) const;

    Type type;

    Place::Ptr place;
    PolarPoint relative_position;

    ArcPtr arc_parent;
    NodePtr child_left, child_right;  // Note that either or both of these may be null.
  }; // struct Node

  struct Arc
  {
    enum class Side { kLeft, kStraight, kRight };

    // The parent is closer to the root, the child closer to a leaf.
    Arc(const NodePtr& parent, const NodePtr& child, const Side side = Side::kStraight);

    NodePtr parent, child;
    Side side;
  }; // struct Arc

  SpiralTree(const Point& root, const Number& restricting_angle_rad);

  void AddPlaces(const std::vector<Place::Ptr>& places);

  void Compute();

  //private:
  Number restricting_angle_rad_;
  Point root_;

  std::vector<NodePtr> nodes_;  // Note that the positions of these nodes are offset by the position of the root.

 private:
  struct CompareEvents { bool operator()(const NodePtr& a, const NodePtr& b) const; }; // struct CompareEvents

  using EventQueue = std::priority_queue<NodePtr, std::deque<NodePtr>, CompareEvents>;
  using Wavefront = std::map<Number, NodePtr>;

  Wavefront::iterator neighbor_cw(Wavefront& wavefront, const Wavefront::iterator& node_iter);

  Wavefront::iterator neighbor_ccw(Wavefront& wavefront, const Wavefront::iterator& node_iter);
}; // class SpiralTree

} // namespace flow_map
} // namespace geoviz

#endif //GEOVIZ_SPIRAL_TREE_H
