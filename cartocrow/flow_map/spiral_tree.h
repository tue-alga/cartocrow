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
#include "node.h"
#include "place.h"

namespace cartocrow::flow_map {

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

  public:
	/// An obstacle is made up of a list of polar points in clockwise order.
	using Obstacle = std::list<PolarPoint>;

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

	/// Changes the root position.
	/**
	 * This removes all existing arcs of the tree. The new tree can be computed
	 * using \ref compute().
	 */
	void setRoot(const Point& root);

	/// Gets the root position of the spiral tree.
	Point root() const;

	/// Changes the restricting angle.
	/**
	 * This removes all existing arcs of the tree. The new tree can be computed
	 * using \ref compute().
	 */
	void setRestrictingAngle(const Number& restricting_angle);

	/// Gets the restricting angle of the spiral tree (in radians).
	Number restrictingAngle() const;

	/// Returns a list of the places in this spiral tree.
	const std::vector<std::shared_ptr<Place>>& places() const;

	/// Returns a list of the nodes in this spiral tree.
	const std::vector<std::shared_ptr<Node>>& nodes() const;

	/// Returns a list of the obstacles in this spiral tree.
	const std::vector<Obstacle>& obstacles() const;

	/// Adds a place to the spiral tree.
	void addPlace(const Place& place);

	/// Adds an obstacle to the spiral tree.
	void addObstacle(const Polygon& obstacle);

	/// Adds a shield obstacle to each place to make sure that these nodes in
	/// the tree become leaves.
	void addShields();

	/// Compute the spiral tree arcs, ignoring any obstacles.
	/**
	 * These arcs are based on the position of the nodes, the restricting angle
	 * of the tree, and any obstacles that are present.
	 *
	 * To compute the tree while taking obstacles into account, use
	 * \ref computeObstructed() instead.
	 */
	void computeUnobstructed();

	/// Compute the spiral tree arcs, taking obstacles into account.
	/**
	 * \warning Currently not implemented.
	 */
	void computeObstructed();

	/// Removes all the arcs and nodes from the tree, so only the manually
	/// added nodes and obstacles remain.
	void clean();

  private:
	struct Event {
		Event(std::shared_ptr<Node> node, const PolarPoint& relative_position)
		    : m_node(node), m_relative_position(relative_position) {}

		Event(const Event& event)
		    : m_node(event.m_node), m_relative_position(event.m_relative_position) {}

		std::shared_ptr<Node> m_node;
		PolarPoint m_relative_position;
	};
	struct CompareEvents {
		bool operator()(const Event& a, const Event& b) const {
			// join nodes are conceptually farther from the root than other nodes
			if (a.m_relative_position.R() == b.m_relative_position.R()) {
				return 1 < b.m_node->m_children.size();
			}

			return a.m_relative_position.R() < b.m_relative_position.R();
		}
	};
	using EventQueue = std::priority_queue<Event, std::deque<Event>, CompareEvents>;

	using Wavefront = std::map<Number, Event>;

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

	void insertJoinEvent(const Event& first, const Event& second, EventQueue& events);

	bool isReachable(const PolarPoint& parent_point, const PolarPoint& child_point) const;

	Number m_restricting_angle;
	Vector m_root_translation;

	std::vector<std::shared_ptr<Place>> m_places;
	std::vector<std::shared_ptr<Node>> m_nodes;
	std::vector<Obstacle> m_obstacles;
};

} // namespace cartocrow::flow_map

#endif //CARTOCROW_SPIRAL_TREE_H
