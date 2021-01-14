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

#include "geoviz/common/circulator.h"


namespace geoviz
{
namespace flow_map
{
namespace
{

struct Event
{
  Event(const Node::Ptr& node, const PolarPoint& relative_position) :
    node(node),
    relative_position(relative_position)
  {}

  Event(const Event& event) :
    node(event.node),
    relative_position(event.relative_position)
  {}

  Node::Ptr node;
  PolarPoint relative_position;
}; // struct Event


struct CompareEvents
{
  bool operator()(const Event& a, const Event& b) const
  {
    // Join nodes are conceptually farther from the root than other nodes.
    if (a.relative_position.R() == b.relative_position.R())
      return 1 < b.node->children.size();

    return a.relative_position.R() < b.relative_position.R();
  }
}; // struct CompareEvents

using EventQueue = std::priority_queue<Event, std::deque<Event>, CompareEvents>;

}  // anonymous namespace


/**@struct Node
 * @brief A node in a tree.
 *
 * This node type is used in both spiral tree and flow tree.
 */

/**@fn Node::Ptr
 * @brief The preferred pointer type for storing or sharing a node.
 */

/**@brief Construct a new node.
 *
 * A node may be associated with a place on the map that either sends or receives flow. These nodes are the root and leaf nodes. Other nodes will have the same amount of incoming flow as the sum of the outgoing flow.
 * @param place the place associated with this node.
 */
Node::Node(const Place::Ptr& place /*= nullptr*/) :
  place(place)
{}

/**@fn Place::Ptr Node::place
 * @brief The place associated with this node.
 *
 * This may be null if no place is associated with this node.
 */

/**@fn Ptr Node::parent
 * @brief The parent of the node.
 *
 * The tree path from this node to the root node must visit the parent node.
 */

/**@fn std::vector<Ptr> Node::children
 * @brief The children of this node.
 *
 * Note that while generally the nodes of a tree without children are refered to as leaf nodes, a node with the leaf type may have children if it is located inside the spiral region of another node.
 */


/**@class SpiralTree
 * @brief A binary tree where each arc is a logarithmic spiral.
 *
 * The spiral arcs are constructed based on a root node and a restricted angle. Logarithmic spirals have the property that the directions of the tangent at any point on the spiral and the line between that point and the root differ by a fixed angle. This angle is the restricted angle for all spiral arcs in the spiral tree.
 */

/**@fn SpiralTree::Ptr
 * @brief The preferred pointer type for storing or sharing a spiral tree.
 */

/**@fn SpiralTree::NodeIterator
 * @brief The iterator type that is used to iterate over all the nodes of the spiral tree.
 */

/**@fn SpiralTree::NodeConstIterator
 * @brief The constant-value iterator type that is used to iterate over all the nodes of the spiral tree.
 */

/**@brief Construct a spiral tree.
 *
 * A spiral tree must always have a root point and a positive restricting angle.
 * @param root the position of the root node. This point is used to determine the relative position of all the nodes in the tree and the shape of the spiral arcs.
 * @param restricting_angle_rad the restricting angle used to construct all the spiral arcs of the tree. This number must be strictly positive.
 */
SpiralTree::SpiralTree(const Point& root, const Number& restricting_angle_rad) :
  restricting_angle_rad_(restricting_angle_rad),
  root_translation_(Point(CGAL::ORIGIN) - root)
{
  CHECK_GT(restricting_angle_rad, 0);
}

/**@fn Point SpiralTree::GetRoot() const
 * @brief Get the root position of the spiral tree.
 * @return the root position of the spiral tree (in cartesian coordinates).
 */

/**@fn Number SpiralTree::GetRestrictingAngle() const
 * @brief Get the restricting angle of the spiral tree (in radians).
 * @param places the restricting angle of the spiral tree (in radians).
 */

/**@fn NodeConstIterator SpiralTree::nodes_begin() const
 * @brief Get a constant-value iterator to the first node of the tree.
 * @return a constant-value iterator to the first node of the tree.
 */

/**@fn NodeConstIterator SpiralTree::nodes_end() const
 * @brief Get a constant-value iterator to the past-the-end node of the tree.
 * @return a constant-value iterator to the past-the-end node of the tree.
 */

/**@fn NodeIterator SpiralTree::nodes_begin()
 * @brief Get a iterator to the first node of the tree.
 * @return a iterator to the first node of the tree.
 */

/**@fn NodeIterator SpiralTree::nodes_end()
 * @brief Get a iterator to the past-the-end node of the tree.
 * @return a iterator to the past-the-end node of the tree.
 */

/**@brief Add a set of places to the spiral tree.
 *
 * Note that the spiral arcs are not automatically computed after adding the places. This requires manually calling the Compute() method.
 * @param places @parblock the set of places to add to the spiral tree.
 *
 * The root of the tree must be part of these places.
 *
 * Non-root places with a non-positive incoming flow will be ignored.
 * @endparblock
 */
void SpiralTree::AddPlaces(const std::vector<Place::Ptr>& places)
{
  Clean();

  for (const Place::Ptr& place : places)
  {
    if (0 < place->flow_in)
      nodes_.push_back(std::make_shared<Node>(place));
  }
}

/**@brief Compute the spiral tree arcs.
 *
 * These arcs are based on the position of the nodes, relative to the root, and the restricting angle of the tree.
 */
void SpiralTree::Compute()
{
  using Wavefront = std::map<Number, Event>;
  Wavefront wavefront;

  EventQueue events;
  for (const Node::Ptr& node : nodes_)
  {
    CHECK_NOTNULL(node->place);

    const PolarPoint relative_position(node->place->position, root_translation_);
    events.push(Event(node, relative_position));
  }

  while (!events.empty())
  {
    Event event = events.top();
    events.pop();

    if (event.relative_position.R() == 0)
    {
      // Connect the remaining node to the root.
      CHECK_EQ(wavefront.size(), 1);
      CHECK_NOTNULL(wavefront.begin()->second.node);

      event.node->children.push_back(wavefront.begin()->second.node);
      wavefront.begin()->second.node->parent = event.node;

      VLOG(2) << "Added root node: " << event.node->place->id;

      wavefront.clear();
      continue;
    }

    // Compute the position of the event node on the wavefront.
    const Number order = event.relative_position.phi();
    Wavefront::iterator node_iter;

    if (1 < event.node->children.size())
    {
      // Join node.

      // Check whether both children are still active.
      CHECK_EQ(event.node->children.size(), 2);
      if (event.node->children[0]->parent != nullptr || event.node->children[1]->parent != nullptr)
        continue;

      // Add the join node to the wavefront and the collection of nodes.
      node_iter = wavefront.emplace(order, event).first;
      nodes_.push_back(event.node);

      VLOG(2) << "Added join node to wavefront: " << node_iter->second.node->place->id;

      // Connect the neighbors to the join node and remove them from the wavefront.
      event.node->children[0]->parent = event.node;
      event.node->children[1]->parent = event.node;

      CHECK_GE(wavefront.size(), 3);
      wavefront.erase(--make_circulator(node_iter, wavefront));
      wavefront.erase(++make_circulator(node_iter, wavefront));
    }
    else
    {
      // Leaf node.

      // If the event node is reachable by another node, they should be connected and the other node should be removed from the wavefront.
      // If the event node is reachable by two nodes u and v, either the event node overlaps the join node of u and v, or that join node must have been handled before (removing u and v from the wavefront).
      // If the event node is reachable by three or more nodes, it is also reachable by two nodes and the above applies.
      // Note that these cases ignore the implied obstacle behind the event node.
      if (!wavefront.empty())
      {
        Circulator<Wavefront> node_circ = make_circulator(wavefront.lower_bound(order), wavefront);
        if
        (
          IsReachable(event.relative_position, node_circ->second.relative_position) ||
          IsReachable(event.relative_position, (--node_circ)->second.relative_position)
        )
        {
          // A neighbor is reachable.
          // Check whether the nodes overlap.
          if (event.relative_position == node_circ->second.relative_position)
          {
            // Replace the event node by the join node.
            // Note that this ignores the implied obstacle behind the event node.

            node_circ->second.node->place = event.node->place;
            event.node = node_circ->second.node;
          }
          else
          {
            // Connect the event node to the neighbor.
            // Note that this ignores the implied obstacle behind the event node.
            //event_node->type = Node::Type::kSubdivision;
            event.node->children.push_back(node_circ->second.node);
            node_circ->second.node->parent = event.node;
          }

          // Remove the neighbor from the wavefront.
          wavefront.erase(node_circ);
        }
      }

      // Add the event node to the wavefront.
      node_iter = wavefront.emplace(order, event).first;

      VLOG(2) << "Added leaf node to wavefront: " << node_iter->second.node->place->id;
    }

    if (wavefront.size() < 2)
      continue;

    // Add join nodes with the neighbors to the event queue.
    {
      // Clockwise.
      Circulator<Wavefront> cw_iter = --make_circulator(node_iter, wavefront);

      const Spiral spiral_left(-restricting_angle_rad_, event.relative_position);
      const Spiral spiral_right(restricting_angle_rad_, cw_iter->second.relative_position);
      const PolarPoint intersection = spiral_left.Intersect(spiral_right);
      CHECK_LE(intersection.R(), event.relative_position.R());
      CHECK_LE(intersection.R(), cw_iter->second.relative_position.R());

      Node::Ptr join = std::make_shared<Node>();
      join->children = {event.node, cw_iter->second.node};

      events.push(Event(join, intersection));

#ifndef NDEBUG
      const std::string id = "[" + cw_iter->second.node->place->id + "+" + event.node->place->id + "]";
      PolarPoint absolute_position(intersection, -root_translation_);
      join->place = std::make_shared<Place>(id, absolute_position);
#endif // NDEBUG
    }

    {
      // Counter-clockwise.
      Circulator<Wavefront> ccw_iter = ++make_circulator(node_iter, wavefront);

      const Spiral spiral_left(-restricting_angle_rad_, ccw_iter->second.relative_position);
      const Spiral spiral_right(restricting_angle_rad_, event.relative_position);
      const PolarPoint intersection = spiral_left.Intersect(spiral_right);
      CHECK_LE(intersection.R(), ccw_iter->second.relative_position.R());
      CHECK_LE(intersection.R(), event.relative_position.R());

      Node::Ptr join = std::make_shared<Node>();
      join->children = {ccw_iter->second.node, event.node};

      events.push(Event(join, intersection));

#ifndef NDEBUG
      const std::string id = "[" + event.node->place->id + "+" + ccw_iter->second.node->place->id + "]";
      PolarPoint absolute_position(intersection, -root_translation_);
      join->place = std::make_shared<Place>(id, absolute_position);
#endif // NDEBUG
    }
  }
}

/**@brief Change the root position.
 *
 * This removes all existing arcs of the tree. The new tree can be computed using Compute().
 * @param root the new root position.
 */
void SpiralTree::SetRoot(const Point& root)
{
  root_translation_ = Point(CGAL::ORIGIN) - root;

  Clean();
}

/**@brief Change the restricting angle.
 *
 * This removes all existing arcs of the tree. The new tree can be computed using Compute().
 * @param restricting_angle_rad the new restricting angle.
 */
void SpiralTree::SetRestrictingAngle(const Number& restricting_angle_rad)
{
  CHECK_GT(restricting_angle_rad, 0);
  restricting_angle_rad_ = restricting_angle_rad;

  Clean();
}

void SpiralTree::Clean()
{
  size_t num_places = 0;

  // Clean the node connections.
  for (Node::Ptr& node : nodes_)
  {
    if (node->place == nullptr)
      break;
    ++num_places;

    node->parent = nullptr;
    node->children.clear();
  }

  // Remove support nodes, e.g. join nodes.
  nodes_.resize(num_places);
}

bool SpiralTree::IsReachable(const PolarPoint& parent_point, const PolarPoint& child_point) const
{
  if (parent_point == child_point)
    return true;

  const Spiral spiral(child_point, parent_point);
  return std::abs(spiral.angle_rad()) <= restricting_angle_rad_;
}

} // namespace flow_map
} // namespace geoviz
