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

#include <ostream>

#include <glog/logging.h>

#include "geoviz/common/circular_range.h"
#include "geoviz/common/circulator.h"
#include "geoviz/common/intersections.h"
#include "geoviz/common/spiral_segment.h"
#include "geoviz/common/polar_segment.h"


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

/**@brief Determine the topological type of this node.
 *
 * Each node is either the root, a leaf, a join node, or a subdivision node.
 * * The root has no parent.
 * * Leaves have no children.
 * * Join nodes have multiple children.
 * * Subdivision nodes have exactly one child.
 *
 * Note that this type does not describe whether the node is associated with a place on the map. This can be determined using IsSteiner().
 * @return the node type.
 */
Node::ConnectionType Node::GetType() const
{
  if (parent == nullptr)
    return ConnectionType::kRoot;
  else switch (children.size())
  {
    case 0:
      return ConnectionType::kLeaf;
    case 1:
      return ConnectionType::kSubdivision;
    default:
      return ConnectionType::kJoin;
  }
}

/**@brief Determine whether this node is a Steiner node.
 *
 * Steiner nodes are not part of the input places. They support the tree, either by splitting the flow, or by guiding the flow around obstacles.
 * @return whether this node is a Steiner node.
 */
bool Node::IsSteiner() const
{
  return
    place == nullptr ||
    (place->flow_in <= 0 && parent != nullptr);
}

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
  CHECK_LE(restricting_angle_rad, M_PI_2);
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

/**@brief Add a set of obstacles to the spiral tree.
 *
 * Note that the spiral arcs are not automatically computed after adding the obstacles. This requires manually calling the Compute() method.
 * @param obstacles the set of obstacles to add to the spiral tree.
 */
void SpiralTree::AddObstacles(const std::vector<Region>& obstacles)
{
  Clean();

  for (const Region& obstacle : obstacles)
    for (const Polygon_with_holes& polygon : obstacle.shape)
      AddObstacle(polygon);
}

/**@brief Compute the spiral tree arcs.
 *
 * These arcs are based on the position of the nodes, relative to the root, and the restricting angle of the tree.
 */
void SpiralTree::Compute()
{
  if (obstacles_.empty())
    ComputeUnobstructed();
  else
    ComputeObstructed();
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
  CHECK_LT(restricting_angle_rad, M_PI_2);
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

void SpiralTree::AddObstacle(const Polygon_with_holes& polygon)
{
  // Ignore the holes of the obstacle: flow cannot cross the obstacle boundary.
  const Polygon& boundary = polygon.outer_boundary();
  if (boundary.is_empty())
    return;

  CHECK_NE(boundary.oriented_side(GetRoot()), CGAL::ON_BOUNDED_SIDE) << "Root inside an obstacle.";

  obstacles_.emplace_back();
  Obstacle& obstacle = obstacles_.back();
  for (Polygon::Vertex_const_iterator vertex_iter = boundary.vertices_begin(); vertex_iter != boundary.vertices_end(); ++vertex_iter)
    obstacle.emplace_back(*vertex_iter, root_translation_);

  // Enforce counter-clockwise obstacles for a canonical arrangement.
//  if (!boundary.is_counterclockwise_oriented())
//    obstacle.reverse();

  // Add vertices for the points closest to the root as well as spiral points.
  // The wedge with the root as apex and boundaries through a closest point and a spiral point (on the same edge) has a fixed angle.
  const Number phi_offset = M_PI_2 - restricting_angle_rad_;
  CHECK_LT(0, phi_offset);

  Obstacle::iterator vertex_prev = --obstacle.end();
  for (Obstacle::iterator vertex_iter = obstacle.begin(); vertex_iter != obstacle.end(); vertex_prev = vertex_iter++)
  {
    const PolarSegment edge(*vertex_prev, *vertex_iter);
    const PolarPoint closest = edge.SupportingLine().foot();

    // The spiral points have fixed R and their phi is offset from the phi of the closest by +/- phi_offset.
    const Number R_s = closest.R() / std::sin(restricting_angle_rad_);

    const int sign = vertex_prev->phi() < vertex_iter->phi() ? -1 : 1;
    const Number phi_s_prev = closest.phi() - sign * phi_offset;
    const Number phi_s_next = closest.phi() + sign * phi_offset;

    // The closest point and spiral points must be added ordered from p to q and only if they are on the edge.
    if (edge.ContainsPhi(phi_s_prev))
      obstacle.insert(vertex_iter, PolarPoint(R_s, phi_s_prev));
    if (edge.ContainsPhi(closest.phi()))
      obstacle.insert(vertex_iter, closest);
    if (edge.ContainsPhi(phi_s_next))
      obstacle.insert(vertex_iter, PolarPoint(R_s, phi_s_next));
  }
}

void SpiralTree::ComputeUnobstructed()
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
            //event_node->type = Node::ConnectionType::kSubdivision;
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

      const Spiral spiral_left(event.relative_position, -restricting_angle_rad_);
      const Spiral spiral_right(cw_iter->second.relative_position, restricting_angle_rad_);

      PolarPoint intersections[2];
      const int num = ComputeIntersections(spiral_left, spiral_right, intersections);
      CHECK_LT(0, num);

      // Note that the intersections should be the two closest to the anchor of the first spiral.
      const PolarPoint& intersection = intersections[0];
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

      const Spiral spiral_left(ccw_iter->second.relative_position, -restricting_angle_rad_);
      const Spiral spiral_right(event.relative_position, restricting_angle_rad_);

      PolarPoint intersections[2];
      const int num = ComputeIntersections(spiral_left, spiral_right, intersections);
      CHECK_LT(0, num);

      // Note that the intersections should be the two closest to the anchor of the first spiral.
      const PolarPoint& intersection = intersections[0];
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






///////////////////////////
/// Obstructed ST impl. ///
///////////////////////////


namespace obst
{


class EdgeStraight;
class EdgeSpiral;

class Edge
{
 public:
  using Ptr = std::shared_ptr<Edge>;  // TODO(tvl) remove all references to Ptr types, unless they are explicitly used in the method interfaces of a class.

  Edge() {}
  virtual ~Edge() {}

  virtual const EdgeStraight* AsEdgeStraight() const { return nullptr; }
  virtual const EdgeSpiral* AsEdgeSpiral() const { return nullptr; }

  virtual Number R_max() const = 0;

  virtual Number R_min() const = 0;

  virtual Number ComputePhi(const Number& R) const = 0;

  virtual PolarPoint Intersect(const EdgeStraight& edge) const = 0;
  virtual PolarPoint Intersect(const EdgeSpiral& edge) const = 0;

  virtual std::ostream& print(std::ostream& os) const = 0;
}; // class Edge

class EdgeStraight : public Edge
{
 public:
  EdgeStraight(const PolarPoint& a, const PolarPoint& b) :
    Edge(), segment(a, b)
  {}

  const EdgeStraight* AsEdgeStraight() const { return this; }

  Number R_max() const { return 0/*segment.far().R()*/; // TODO(tvl) fix this so the segment is always oriented correctly. Better yet, replace the method(s) by getting the (unoriented) endpoints...
  }

  Number R_min() const { return 0/*segment.near().R()*/; // TODO(tvl) fix this so the segment is always oriented correctly.
  }

  Number ComputePhi(const Number& R) const
  {
    // Note, due to minor round-off errors, there can be two very similar phi values near the point closest to the root.
    Number phi[2];
    CHECK_LT(0, segment.CollectPhi(R, phi));

    return phi[0];
  }

  PolarPoint Intersect(const EdgeStraight& edge) const { return PolarPoint(); }
  PolarPoint Intersect(const EdgeSpiral& edge) const { return PolarPoint(); }

  std::ostream& print(std::ostream& os) const
  {
    os << segment;
    return os;
  }

  // Note that because each edge was split at the point closest to the root, the vertices cannot be equidistant from the root.
  PolarSegment segment;
}; // class EdgeStraight

class EdgeSpiral : public Edge
{
 public:
  EdgeSpiral(const PolarPoint& a, const PolarPoint& b) : Edge(), segment(a, b) {}

  const EdgeSpiral* AsEdgeSpiral() const { return this; }

  Number R_max() const { return segment.far().R(); }

  Number R_min() const { return segment.near().R(); }

  Number ComputePhi(const Number& R) const
  {
    return segment.ComputePhi(R);
  }

  PolarPoint Intersect(const EdgeStraight& edge) const { return PolarPoint(); }
  PolarPoint Intersect(const EdgeSpiral& edge) const { return PolarPoint(); }

  std::ostream& print(std::ostream& os) const
  {
    os << segment;
    return os;
  }

  SpiralSegment segment;
}; // class EdgeSpiral

std::ostream& operator<<(std::ostream& os, const Edge& edge)
{
  edge.print(os);
  return os;
}




class Interval
{
 public:
  using Ptr = std::shared_ptr<Interval>;

  Interval(const Edge::Ptr& edge_cw, const Edge::Ptr& edge_ccw, const bool obstacle = false) : edge_cw(edge_cw), edge_ccw(edge_ccw), obstacle(obstacle), tag(nullptr) {}

  Interval(const Edge::Ptr& edge_cw, const Edge::Ptr& edge_ccw, const Node::Ptr& tag) : edge_cw(edge_cw), edge_ccw(edge_ccw), obstacle(false), tag(tag) {}

  // Note that an open interval referencing a connected node counts as a free interval.
  bool IsFree() const { return tag == nullptr || tag->parent != nullptr; }

  bool obstacle;
  Node::Ptr tag;

  Edge::Ptr edge_cw, edge_ccw;
}; // class Interval

struct CompareIntervals
{
  bool operator()(const Interval::Ptr& a, const Interval::Ptr& b) const
  {
    const Edge::Ptr& edge_ccw_a = a->edge_ccw;
    const Edge::Ptr& edge_ccw_b = b->edge_ccw;

    // Compare the ccw edges somewhere inside (we use the center of their shared range of R.
    //
    //
    // at their 'farthest' shared point (this would be closest to the root for the reverse ordering).
    // Note that there shouldn't be a case where the cw edges of two intervals don't share some part (distance from the root), due to the processing of the events in order.
    // Also note that if two edges are found to intersect, they should be replaced by their subdivided counterparts, because otherwise the 'farthest' point is no longer a good measurement for the order of the intervals on the circle.
    const Number& R_max = std::min(edge_ccw_a->R_max(), edge_ccw_b->R_max());
    const Number& R_min = std::max(edge_ccw_a->R_min(), edge_ccw_b->R_min());
    CHECK_LE(R_min, R_max);

    const Number R = (R_min + R_max) / 2;
    const Number& phi_a = edge_ccw_a->ComputePhi(R);
    const Number& phi_b = edge_ccw_b->ComputePhi(R);
    return phi_a < phi_b;
  }
}; // struct CompareIntervals



class Event
{
 public:
  using Ptr = std::shared_ptr<Event>;

  // Node event.
  Event(const PolarPoint& relative_position, const Node::Ptr& node) :
    relative_position(relative_position), node(node) {}

  // Vertex event.
  Event
  (
    const PolarPoint& relative_position,
    const Node::Ptr& vertex,
    const Edge::Ptr& edge_cw,
    const Edge::Ptr& edge_ccw
  ) : relative_position(relative_position), node(vertex), edge_cw(edge_cw), edge_ccw(edge_ccw) {}

  // Vanishing event.
  Event
  (
    const PolarPoint& relative_position,
    const Edge::Ptr& edge_cw,
    const Edge::Ptr& edge_ccw
  ) : Event(relative_position, nullptr, edge_cw, edge_ccw) {}

  bool IsNode() const { return (bool)node && !HasEdge(); }
  bool IsVertex() const { return (bool)node && HasEdge(); }
  bool IsVanishing() const { return !node; }

  PolarPoint relative_position;

  Node::Ptr node;
  Edge::Ptr edge_cw, edge_ccw;

 private:
  bool HasEdge() const { return (bool)edge_cw || (bool)edge_ccw; }
}; // class Event

std::ostream& operator<<(std::ostream& os, const Event& event)
{
  os << "Event @ " << event.relative_position << std::endl;

  os << "CW: ";
  if (event.edge_cw)
    os << (*event.edge_cw) << std::endl;
  else
    os << "null" << std::endl;

  os << "CCW: ";
  if (event.edge_ccw)
    os << (*event.edge_ccw) << std::endl;
  else
    os << "null" << std::endl;
}


struct CompareEvents
{
  bool operator()(const Event::Ptr& a, const Event::Ptr& b) const
  {
    return a->relative_position.R() < b->relative_position.R();
  }
}; // struct CompareEvents

struct CompareEventsReverse : public CompareEvents
{
  bool operator()(const Event::Ptr& a, const Event::Ptr& b) const
  {
    return CompareEvents::operator()(b, a);
  }
}; // struct CompareEventsReverse


// Strictly speaking, the wavefront should be the collection of unconnected nodes that have been passed by the sweep circle.
// In practice, we keep track of the intervals on this sweep circle instead, ordered by their edges intersecting the sweep circle.
class SweepStatus
{
 public:
  using Boundary = obst::Edge::Ptr;
  using BoundarySet = std::set<Boundary>;



  BoundarySet boundaries;
}; // class SweepStatus


} // namespace obst



void SpiralTree::ComputeObstructed()
{
  // Required data types:
  // Points (events):
  // * Node/terminal
  //   - tree node
  //   - Reachable regions? (or is the node stored in the region?)
  //     probably necessary to clean up all reachable regions of this node, once it is connected to another point. However, this may also be done implicitly by smart definitions.
  // * Vertex
  //   - obstacle vertex (that can become part of the tree).
  //   - 2 obstacle edges connected to the vertex.
  // * Vanishing point (of a region); could be at a vertex, but these should probably be handled by the vertex case.
  //   - should this have the region that vanishes attached?
  //   - probably better: the 2 edges involved.
  //   vanishing points should be checked when encountered: are they still applicable (or should they be 'removed' when their interval changes?), i.e. are their two edges still next to each other (needs sorted list iterators to check)?
  //
  // Edges (endpoints):
  // * Straight
  // * Spiral
  // Each edge has two associated regions/intervals.
  //
  // Intervals (is it necessary to declare these explicitly, or can they be implicitly defined in their edges?):
  // - Vanishing event?
  // * Free/unreachable (e.g. beyond the last node or behind an obstacle)
  // * Obstacle
  // * Reachable:
  //   - 1 parent (tagged) node/vertex: node/vertex n 'inside' interval, farther from the root (to connect to next node touching interval). This is actually the point in the region fartest from the root.
  //   - Could we just mark free intervals as 'reachable' intervals without a parent?
  // Stored in (balanced) search tree by their endpoints, i.e. their edges. The order of the endpoints never changes: when this would be the case, a new region is inserted. Can we store only the left (or right) edge with each interval?
  //
  // Maybe: an interval is marked as either "obstacle" or "open". An open interval can have a tagged event (node/vertex); an open interval that either has no tagged event or that has a tagged event that is already assigned a parent is called a "free interval".
  // Is merging newly freed up intervals really necessary? Isn't this just a case of more clutter without actual problems?
  //
  // Event (per point type).


  // Debug lists of all elements ever created.
  std::vector<obst::Interval::Ptr> intervals_d;
  std::vector<obst::Edge::Ptr> edges_d;

  // Actual functional collections.
  std::priority_queue<obst::Event::Ptr, std::vector<obst::Event::Ptr>, obst::CompareEventsReverse> queue;

  std::vector<Node::Ptr> vertices;


  // Add the obstacles.
  for (const Obstacle& obstacle : obstacles_)
  {
    if (obstacle.empty())
      continue;

    obst::Event::Ptr vertex;
    obst::EdgeStraight::Ptr edge_first = nullptr;

    Obstacle::const_iterator prev_iter = --obstacle.end();
    for (Obstacle::const_iterator vertex_iter = obstacle.begin(); vertex_iter != obstacle.end(); prev_iter = vertex_iter++)
    {
      obst::EdgeStraight::Ptr edge_cw = std::make_shared<obst::EdgeStraight>(*prev_iter, *vertex_iter);
      edges_d.push_back(edge_cw);
      if (edge_first == nullptr)
        edge_first = edge_cw;

      if (vertex)
        vertex->edge_ccw = edge_cw;

      vertices.push_back(std::make_shared<Node>());
      const Node::Ptr& node = vertices.back();

      vertex = std::make_shared<obst::Event>(*vertex_iter, node, edge_cw, nullptr);
      queue.push(vertex);
    }

    CHECK(vertex);
    vertex->edge_ccw = edge_first;
  }


  // Compute the reachable region by handling the events (in reverse order) and keeping track of the boundaries of the reachable intervals.
  obst::SweepStatus intervals;




  bool test0 = true;

  // DEBUG //
  while (!queue.empty())
  {
    obst::Event::Ptr event = queue.top();
    queue.pop();

    std::cerr << (*event) << std::endl << std::endl;
  }


  bool test = true;




  // TODO(tvl) temp placeholder.
  //ComputeUnobstructed();
}

} // namespace flow_map
} // namespace geoviz
