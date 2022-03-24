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

#include "cartocrow/core/region.h"
#include "cartocrow/flow_map/place.h"

namespace cartocrow::flow_map {

/// A node in a spiral or flow tree.
struct Node {
	/// The preferred pointer type for storing or sharing a node.
	using Ptr = std::shared_ptr<Node>;

	/// The type of node, as defined by its connections.
	enum class ConnectionType {
		/// The root node, the only node without a parent.
		kRoot,
		/// A leaf node, a node without any children.
		kLeaf,
		/// A join node, a node with at least two children.
		kJoin,
		/// A subdivision node, a node with exactly one child.
		kSubdivision
	};

	/// Construct a new node.
	/**
	 * A node may be associated with a place (\c place) on the map that either
	 * sends or receives flow. These nodes are the root and leaf nodes. Other
	 * nodes will have the same amount of incoming flow as the sum of the
	 * outgoing flow.
	 */
	explicit Node(const Place::Ptr& place = nullptr);

	/// Determine the type of this node, based on its number of children.
	/**
	 * Each node is either the root, a leaf, a join node, or a subdivision node
	 * (see \ref ConnectionType).
	 */
	ConnectionType getType() const;

	/// Determine whether this node is a Steiner node.
	/**
	 * Steiner nodes are not part of the input places. They support the tree,
	 * either by splitting the flow, or by guiding the flow around obstacles.
	 */
	bool isSteiner() const;

	/// The place associated with this node, or \c nullptr if no place is
	/// associated with this node.
	Place::Ptr place;
	/// The parent of this node.
	Ptr parent;
	/// The children of this node.
	/**
	 * While generally the nodes of a tree without children are refered to as
	 * leaf nodes, a node with the leaf type may have children if it is located
	 * inside the spiral region of another node.
	 */
	std::vector<Ptr> children;
};

/// A binary tree where each arc is a logarithmic spiral.
/**
 * The spiral arcs are constructed based on a root node and a restricted angle.
 * Logarithmic spirals have the property that the directions of the tangent at
 * any point on the spiral and the line between that point and the root differ
 * by a fixed angle. This angle is the restricted angle for all spiral arcs in
 * the spiral tree.
 *
 * To make a spiral tree, first construct it (\ref SpiralTree()), then add
 * places for the tree to reach (\ref addPlaces()), possibly add some obstacles
 * for the tree to avoid (\ref addObstacles()), and then call \ref compute().
 *
 * \warning Obstacle avoidance is not currently implemented.
 */
class SpiralTree {
  private:
	using NodeSet = std::vector<Node::Ptr>;

	using Obstacle = std::list<PolarPoint>;
	using ObstacleSet = std::vector<Obstacle>;

	struct Event {
		Event(const Node::Ptr& node, const PolarPoint& relative_position)
		    : node(node), relative_position(relative_position) {}

		Event(const Event& event) : node(event.node), relative_position(event.relative_position) {}

		Node::Ptr node;
		PolarPoint relative_position;
	};
	struct CompareEvents {
		bool operator()(const Event& a, const Event& b) const {
			// Join nodes are conceptually farther from the root than other nodes.
			if (a.relative_position.R() == b.relative_position.R()) {
				return 1 < b.node->children.size();
			}

			return a.relative_position.R() < b.relative_position.R();
		}
	};
	using EventQueue = std::priority_queue<Event, std::deque<Event>, CompareEvents>;

	using Wavefront = std::map<Number, Event>;

  public:
	/// The preferred pointer type for storing or sharing a spiral tree.
	using Ptr = std::shared_ptr<SpiralTree>;
	/// The iterator type that is used to iterate over all the nodes of the spiral tree.
	using NodeIterator = NodeSet::iterator;
	/// The constant-value iterator type that is used to iterate over all the nodes of the spiral tree.
	using NodeConstIterator = NodeSet::const_iterator;

  private: // TODO(tvl) made private until computing the tree with obstructions is implemented.
	using ObstacleIterator = ObstacleSet::iterator;
	using ObstacleConstIterator = ObstacleSet::const_iterator;

  public:
	/// Constructs a spiral tree.
	/**
	 * A spiral tree must always have a root point and a positive restricting
	 * angle.
	 * @param root The position of the root node. This point is used to
	 * determine the relative position of all the nodes in the tree and the
	 * shape of the spiral arcs.
	 * @param restricting_angle The restricting angle used to construct all
	 * the spiral arcs of the tree. This number must be strictly positive.
	 */
	SpiralTree(const Point& root, const Number& restricting_angle);

	/// Gets the root position of the spiral tree.
	inline Point getRoot() const {
		return Point(CGAL::ORIGIN) - m_root_translation;
	}

	/// Gets the restricting angle of the spiral tree (in radians).
	inline Number getRestrictingAngle() const {
		return m_restricting_angle;
	}

	/// Gets a constant-value iterator to the first node of the tree.
	NodeConstIterator nodes_begin() const {
		return m_nodes.begin();
	}
	/// Gets a constant-value iterator to the past-the-end node of the tree.
	NodeConstIterator nodes_end() const {
		return m_nodes.end();
	}
	/// Gets a iterator to the first node of the tree.
	NodeIterator nodes_begin() {
		return m_nodes.begin();
	}
	/// Gets a iterator to the past-the-end node of the tree.
	NodeIterator nodes_end() {
		return m_nodes.end();
	}

  private: // TODO(tvl) made private until computing the tree with obstructions is implemented.
	ObstacleConstIterator obstacles_begin() const {
		return m_obstacles.begin();
	}
	ObstacleConstIterator obstacles_end() const {
		return m_obstacles.end();
	}

	ObstacleIterator obstacles_begin() {
		return m_obstacles.begin();
	}
	ObstacleIterator obstacles_end() {
		return m_obstacles.end();
	}

  public:
	/// Adds a set of places to the spiral tree.
	/**
	 * The spiral arcs are not automatically computed after adding the places;
	 * this requires manually calling \ref compute().
	 * @param places The set of places to add to the spiral tree. This must
	 * contain the root of the tree. Non-root places with a non-positive
	 * incoming flow will be ignored.
	 */
	void addPlaces(const std::vector<Place::Ptr>& places);

  private: // TODO(tvl) made private until computing the tree with obstructions is implemented.
	/// Adds a set of obstacles to the spiral tree.
	/**
	 * The spiral arcs are not automatically computed after adding the places;
	 * this requires manually calling \ref compute().
	 * @param places The set of places to add to the spiral tree. This must
	 * contain the root of the tree. Non-root places with a non-positive
	 * incoming flow will be ignored.
	 */
	void addObstacles(const std::vector<Region>& obstacles);

  public:
	/// Computes the spiral tree arcs.
	/**
	 * These arcs are based on the position of the nodes, the restricting angle
	 * of the tree, and any obstacles that are present.
	 *
	 * If no specific obstacles have been added, input nodes are not forced to
	 * be leaf nodes in the final tree. If this is desired, use \ref
	 * computeObstructed() instead.
	 */
	void compute();

  private: // TODO(tvl) made private until computing the tree with obstructions is implemented.
	/// Compute the spiral tree arcs, ignoring any obstacles.
	void computeUnobstructed();

	/// Handles a root event in the spiral tree computation algorithm.
	/**
	 * This finalizes the algorithm: it connects the remaining wavefront node to
	 * the root and empties the wavefront.
	 */
	void handleRootEvent(const Event& event, Wavefront& wavefront);

	/// Handles a join event in the spiral tree computation algorithm.
	/**
	 * This first checks if the event is invalid (which happens if the children
	 * of the join node are not both active anymore). If the event is valid, we
	 * remove the children from the wavefront, connect them to the join node,
	 * and add the join node to the wavefront.
	 *
	 * Returns the position of the newly inserted join node in the wavefront,
	 * or \c std::nullopt if the event was invalid.
	 */
	std::optional<Wavefront::iterator> handleJoinEvent(const Event& event, Wavefront& wavefront);

	/// Handles a leaf event in the spiral tree computation algorithm.
	/**
	 * This checks if the new leaf node is reachable from one of its neighbors
	 * in the wavefront. (It cannot be reachable from both neighbors, because
	 * then the reachable regions from these neighbors would overlap, resulting
	 * in a join event that should have been handled before this leaf event,
	 * which removes the neighbors from the wavefront.)
	 *
	 * If indeed the leaf node is reachable from a neighbor \f$ v \f$, then
	 * \f$ v \f$ becomes the child of the new node and hence gets removed from
	 * the wavefront. Else, the leaf node is simply inserted into the wavefront
	 * without children.
	 *
	 * Returns the position of the newly inserted leaf node in the wavefront.
	 */
	Wavefront::iterator handleLeafEvent(Event& event, Wavefront& wavefront);

	/// Compute the spiral tree arcs, taking obstacles into account.
	/**
	 * \warning Currently not implemented.
	 */
	void computeObstructed();

  public:
	/// Changes the root position.
	/**
	 * This removes all existing arcs of the tree. The new tree can be computed
	 * using \ref compute().
	 */
	void setRoot(const Point& root);

	/// Changes the restricting angle.
	/**
	 * This removes all existing arcs of the tree. The new tree can be computed
	 * using \ref compute().
	 */
	void setRestrictingAngle(const Number& restricting_angle);

  private:
	void clean();

	bool isReachable(const PolarPoint& parent_point, const PolarPoint& child_point) const;

	void addObstacle(const Polygon_with_holes& polygon);

	Number m_restricting_angle;
	Vector m_root_translation;

	NodeSet m_nodes; // Note that the positions of these nodes are offset by the position of the root.
	ObstacleSet m_obstacles;
};

} // namespace cartocrow::flow_map

#endif //CARTOCROW_SPIRAL_TREE_H
