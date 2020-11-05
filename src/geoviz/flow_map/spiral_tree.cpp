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

#include "spiral_tree.h"

#include <glog/logging.h>


namespace geoviz
{
namespace flow_map
{

std::ostream& operator<<(std::ostream& os, const SpiralTree::Node::Type& type)
{
  switch (type)
  {
    case SpiralTree::Node::Type::kRoot:
      os << "[root node]";
      break;
    case SpiralTree::Node::Type::kJoin:
      os << "[join node]";
      break;
    case SpiralTree::Node::Type::kLeaf:
      os << "[leaf node]";
      break;
  }
}

SpiralTree::Node::Node(const Type type, const PolarPoint& relative_position, const Place::Ptr& place /*= nullptr*/) :
  type(type),
  place(place),
  relative_position(relative_position)
{}

Number SpiralTree::Node::ComputeOrder(const Number& restricting_angle_rad) const
{
  return Spiral(restricting_angle_rad, relative_position).ComputeOrder();
}


SpiralTree::Arc::Arc(const NodePtr& parent, const NodePtr& child, const Side side /*= Side::kStraight*/) :
  parent(parent),
  child(child),
  side(side)
{
  CHECK_NOTNULL(parent);
  CHECK_NOTNULL(child);
}


SpiralTree::SpiralTree(const Point& root, const Number& restricting_angle_rad) :
  restricting_angle_rad_(restricting_angle_rad),
  root_(root)
{}

void SpiralTree::AddPlaces(const std::vector<Place::Ptr>& places)
{
  // The root node will be positioned at the origin.
  const Vector translation = Point(CGAL::ORIGIN) - root_;

  for (const Place::Ptr& place : places)
  {
    PolarPoint relative_position(place->position, translation);
    Node::Type type = relative_position.R() == 0 ? Node::Type::kRoot : Node::Type::kLeaf;

    // Skip leaves without incoming flow.
    if (type != Node::Type::kRoot && place->flow_in <= 0)
      continue;

    /*if
    (
      place->id != "CA" &&
      place->id != "ME" &&
      place->id != "FL" &&
      place->id != "NH" &&
      place->id != "OR" &&
      place->id != "TX" &&
      place->id != "PA" &&
      place->id != "MI" &&
      place->id != "NM" &&
      place->id != "NC" &&
      place->id != "ND"
    )
      continue;  // TODO(tvl) TMP DEBUG.*/

    nodes_.push_back(std::make_shared<Node>(type, relative_position, place));
  }
}

void SpiralTree::Compute()
{
  EventQueue events(nodes_.begin(), nodes_.end());
  Wavefront wavefront;

  while (!events.empty())
  {
    NodePtr event_node = events.top();
    events.pop();

    if (event_node->type == Node::Type::kRoot)
    {
      // Connect the remaining node to the root.
      CHECK_EQ(wavefront.size(), 1);
      event_node->child_left = wavefront.begin()->second;
      wavefront.begin()->second->arc_parent = std::make_shared<Arc>(event_node, event_node->child_left, Arc::Side::kStraight);

      wavefront.clear();
      continue;
    }

    // Compute the position of the event node on the wavefront.
    //const Number order = event_node->ComputeOrder(restricting_angle_rad_);
    const Number order = event_node->relative_position.phi();
    Wavefront::iterator node_iter = wavefront.find(order);

    Wavefront::iterator cw_iter, ccw_iter;
    if (event_node->type == Node::Type::kJoin)
    {
      // Join node.
      // Check whether both children are still active.
      if (event_node->child_left->arc_parent != nullptr || event_node->child_right->arc_parent != nullptr)
        continue;

      // Add the join node to the wavefront and collect its neighbors.
      node_iter = wavefront.emplace(order, event_node).first;

      cw_iter = neighbor_cw(wavefront, node_iter);
      ccw_iter = neighbor_ccw(wavefront, node_iter);

      CHECK_EQ(event_node->child_right, cw_iter->second);
      CHECK_EQ(event_node->child_left, ccw_iter->second);

      // Connect the neighbors to the join node and remove them from the wavefront.
      event_node->child_left->arc_parent = std::make_shared<Arc>(event_node, event_node->child_left, Arc::Side::kLeft);
      event_node->child_right->arc_parent = std::make_shared<Arc>(event_node, event_node->child_right, Arc::Side::kRight);

      wavefront.erase(cw_iter);
      wavefront.erase(ccw_iter);




      nodes_.push_back(event_node);
    }
    else
    {
      // Leaf node.
      // Check whether the event node position overlaps an existing node in the wavefront.
      if (node_iter != wavefront.end())
      {
        // Connect the nodes and remove the existing node from the wavefront.
        // Note that this ignores the implied obstacle behind the event node.
        event_node->child_left = node_iter->second;
        node_iter->second->arc_parent = std::make_shared<Arc>(event_node, event_node->child_left, Arc::Side::kStraight);
        wavefront.erase(node_iter);
      }

      // Add the event node to the wavefront.
      node_iter = wavefront.emplace(order, event_node).first;

      // Check whether the event node is reachable by a neighbor; if so connect them and remove the neighbor.
      if (1 < wavefront.size())
      {
        cw_iter = neighbor_cw(wavefront, node_iter);
        CHECK_LE(event_node->relative_position.R(), cw_iter->second->relative_position.R());
        const Spiral cw_spiral(cw_iter->second->relative_position, event_node->relative_position);
        if (std::abs(cw_spiral.angle_rad()) <= restricting_angle_rad_)
        {
          event_node->child_left = cw_iter->second;
          cw_iter->second->arc_parent = std::make_shared<Arc>
          (
            event_node,
            event_node->child_left,
            Arc::Side::kStraight
          );
          wavefront.erase(cw_iter);
        }
      }

      if (1 < wavefront.size())
      {
        ccw_iter = neighbor_ccw(wavefront, node_iter);
        CHECK_LE(event_node->relative_position.R(), ccw_iter->second->relative_position.R());
        const Spiral ccw_spiral(ccw_iter->second->relative_position, event_node->relative_position);
        if (std::abs(ccw_spiral.angle_rad()) <= restricting_angle_rad_)
        {
          event_node->child_right = ccw_iter->second;
          ccw_iter->second->arc_parent = std::make_shared<Arc>
          (
            event_node,
            event_node->child_right,
            Arc::Side::kStraight
          );
          wavefront.erase(ccw_iter);
        }
      }
    }

    if (wavefront.size() < 2)
      continue;



    const Vector translation = root_ - Point(CGAL::ORIGIN);  // TODO(tvl) TMP DEBUG




    // Add join nodes with the neighbors to the event queue.
    {
      // Clockwise.
      cw_iter = neighbor_cw(wavefront, node_iter);

      const Spiral spiral_left(-restricting_angle_rad_, event_node->relative_position);
      const Spiral spiral_right(restricting_angle_rad_, cw_iter->second->relative_position);
      const PolarPoint intersection = spiral_left.Intersect(spiral_right);
      CHECK_LE(intersection.R(), event_node->relative_position.R());
      CHECK_LE(intersection.R(), cw_iter->second->relative_position.R());

      NodePtr join = std::make_shared<Node>(Node::Type::kJoin, intersection);
      join->child_left = event_node;
      join->child_right = cw_iter->second;

      events.push(join);





      // TODO(tvl) TMP DEBUG.
      const std::string id = "[" + cw_iter->second->place->id + "+" + event_node->place->id + "]";
      PolarPoint absolute_position(join->relative_position, translation);
      join->place = std::make_shared<Place>(id, absolute_position);
    }

    {
      // Counter-clockwise.
      ccw_iter = neighbor_ccw(wavefront, node_iter);

      const Spiral spiral_left(-restricting_angle_rad_, ccw_iter->second->relative_position);
      const Spiral spiral_right(restricting_angle_rad_, event_node->relative_position);
      const PolarPoint intersection = spiral_left.Intersect(spiral_right);
      CHECK_LE(intersection.R(), ccw_iter->second->relative_position.R());
      CHECK_LE(intersection.R(), event_node->relative_position.R());

      NodePtr join = std::make_shared<Node>(Node::Type::kJoin, intersection);
      join->child_left = ccw_iter->second;
      join->child_right = event_node;

      events.push(join);





      // TODO(tvl) TMP DEBUG.
      const std::string id = "[" + event_node->place->id + "+" + ccw_iter->second->place->id + "]";
      PolarPoint absolute_position(join->relative_position, translation);
      join->place = std::make_shared<Place>(id, absolute_position);
    }
  }
}

bool SpiralTree::CompareEvents::operator()(const NodePtr& a, const NodePtr& b) const
{
  return a->relative_position.R() < b->relative_position.R();
}

SpiralTree::Wavefront::iterator SpiralTree::neighbor_cw(Wavefront& wavefront, const Wavefront::iterator& node_iter)
{
  Wavefront::iterator neighbor_iter = node_iter;
  if (neighbor_iter == wavefront.begin()) neighbor_iter = wavefront.end();
  --neighbor_iter;
  return neighbor_iter;
}

SpiralTree::Wavefront::iterator SpiralTree::neighbor_ccw(Wavefront& wavefront, const Wavefront::iterator& node_iter)
{
  Wavefront::iterator neighbor_iter = node_iter;
  ++neighbor_iter;
  if (neighbor_iter == wavefront.end()) neighbor_iter = wavefront.begin();
  return neighbor_iter;
}

} // namespace flow_map
} // namespace geoviz
