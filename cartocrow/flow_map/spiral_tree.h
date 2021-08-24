/*
The Flow Map library implements the algorithmic geo-visualization
method by the same name, developed by Kevin Verbeek, Kevin Buchin,
and Bettina Speckmann at TU Eindhoven
(DOI: 10.1007/s00453-013-9867-z & 10.1109/TVCG.2011.202).
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 28-10-2020
*/

#ifndef CARTOCROW_FLOW_MAP_SPIRAL_TREE_H
#define CARTOCROW_FLOW_MAP_SPIRAL_TREE_H

#include <deque>
#include <map>
#include <memory>
#include <queue>
#include <vector>

#include "cartocrow/common/region.h"
#include "cartocrow/flow_map/place.h"


namespace cartocrow
{
namespace flow_map
{

struct Node
{
  using Ptr = std::shared_ptr<Node>;

  enum class ConnectionType
  {
    kRoot,
    kLeaf,
    kJoin,
    kSubdivision
  };

  explicit Node(const Place::Ptr& place = nullptr);

  ConnectionType GetType() const;

  bool IsSteiner() const;

  Place::Ptr place;
  Ptr parent;
  std::vector<Ptr> children;
}; // struct Node


class SpiralTree
{
 private:
  using NodeSet = std::vector<Node::Ptr>;

  using Obstacle = std::list<PolarPoint>;
  using ObstacleSet = std::vector<Obstacle>;

 public:
  using Ptr = std::shared_ptr<SpiralTree>;

  using NodeIterator = NodeSet::iterator;
  using NodeConstIterator = NodeSet::const_iterator;

 private:  // TODO(tvl) made private until computing the tree with obstructions is implemented.
  using ObstacleIterator = ObstacleSet::iterator;
  using ObstacleConstIterator = ObstacleSet::const_iterator;
 public:

  SpiralTree(const Point& root, const Number& restricting_angle_rad);

  inline Point GetRoot() const { return Point(CGAL::ORIGIN) - root_translation_; }

  inline Number GetRestrictingAngle() const { return restricting_angle_rad_; }

  NodeConstIterator nodes_begin() const { return nodes_.begin(); }
  NodeConstIterator nodes_end() const { return nodes_.end(); }

  NodeIterator nodes_begin() { return nodes_.begin(); }
  NodeIterator nodes_end() { return nodes_.end(); }

 private:  // TODO(tvl) made private until computing the tree with obstructions is implemented.
  ObstacleConstIterator obstacles_begin() const { return obstacles_.begin(); }
  ObstacleConstIterator obstacles_end() const { return obstacles_.end(); }

  ObstacleIterator obstacles_begin() { return obstacles_.begin(); }
  ObstacleIterator obstacles_end() { return obstacles_.end(); }
 public:

  void AddPlaces(const std::vector<Place::Ptr>& places);

 private:  // TODO(tvl) made private until computing the tree with obstructions is implemented.
  void AddObstacles(const std::vector<Region>& obstacles);
 public:

  void Compute();

 private:  // TODO(tvl) made private until computing the tree with obstructions is implemented.
  void ComputeUnobstructed();

  void ComputeObstructed();
 public:

  void SetRoot(const Point& root);

  void SetRestrictingAngle(const Number& restricting_angle_rad);

 private:
  void Clean();

  bool IsReachable(const PolarPoint& parent_point, const PolarPoint& child_point) const;

  void AddObstacle(const Polygon_with_holes& polygon);

  Number restricting_angle_rad_;
  Vector root_translation_;

  NodeSet nodes_;  // Note that the positions of these nodes are offset by the position of the root.
  ObstacleSet obstacles_;
}; // class SpiralTree

} // namespace flow_map
} // namespace cartocrow

#endif //CARTOCROW_SPIRAL_TREE_H
