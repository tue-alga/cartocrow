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

#include "../core/region_map.h"
#include "node.h"
#include "place.h"

namespace cartocrow::flow_map {

/// A binary tree where each arc is a logarithmic spiral.
///
/// The spiral arcs are constructed based on a root node and a restricted angle.
/// Logarithmic spirals have the property that the directions of the tangent at
/// any point on the spiral and the line between that point and the root differ
/// by a fixed angle. This angle is the restricted angle for all spiral arcs in
/// the spiral tree.
///
/// To make a spiral tree, first construct it (\ref SpiralTree()), then add
/// places for the tree to reach (\ref addPlace()), possibly add some obstacles
/// for the tree to avoid (\ref addObstacle()), and then use
/// \ref SpiralTreeUnobstructedAlgorithm or \ref SpiralTreeObstructedAlgorithm.
///
/// ## Example
///
/// The intended way to compute a spiral tree is like this:
/// ```
/// // construct spiral tree rooted at (100, 100) with restricted angle 0.5
/// flow_map::Spiral Tree tree(Point<Inexact>(100, 100), 0.5);
///
/// // add places
/// tree.addPlace("BE", Point<Inexact>(90, 50), 100);
/// tree.addPlace("DE", Point<Inexact>(160, 20), 200);
/// tree.addPlace("FR", Point<Inexact>(70, -30), 150);
///
/// // add obstacle
/// Region obstacle = ...;
/// tree.addObstacle(obstacle);
///
/// // run computation
/// flow_map::SpiralTreeUnobstructedAlgorithm algorithm(tree);
/// algorithm.run();
/// ```
class SpiralTree {

  public:
	/// An obstacle is made up of a list of polar points in clockwise order.
	using Obstacle = std::list<PolarPoint>;

	/// Constructs a spiral tree.
	///
	/// \param rootPosition The position of the root node. This point is used to
	/// determine the relative position of all the nodes in the tree and the
	/// shape of the spiral arcs.
	/// \param restrictingAngle The restricting angle used to construct all
	/// the spiral arcs of the tree. This number must be strictly positive.
	SpiralTree(const Point<Inexact> rootPosition, const Number<Inexact> restrictingAngle);

	/// Changes the root position.
	///
	/// This removes all existing arcs of the tree. The new tree can be computed
	/// using \ref compute().
	//void setRootPosition(const Point<Inexact>& rootPosition);

	/// Gets the root position of the spiral tree.
	Point<Inexact> rootPosition() const;

	/// Changes the restricting angle.
	///
	/// This removes all existing arcs of the tree. The new tree can be computed
	/// using \ref compute().
	//void setRestrictingAngle(const Number<Inexact> restrictingAngle);

	/// Gets the restricting angle of the spiral tree (in radians).
	Number<Inexact> restrictingAngle() const;

	/// Returns a list of the places in this spiral tree.
	const std::vector<std::shared_ptr<Place>>& places() const;

	/// Returns a list of the nodes in this spiral tree.
	const std::vector<std::shared_ptr<Node>>& nodes() const;

	/// Returns a list of the obstacles in this spiral tree.
	const std::vector<Obstacle>& obstacles() const;

	/// Adds a node to the spiral tree.
	//void addNode(std::shared_ptr<Node> node);

	/// Adds a place with the given name, position, and flow to the spiral tree.
	void addPlace(const std::string& name, const Point<Inexact>& position, Number<Inexact> flow);

	/// Adds an obstacle to the spiral tree.
	void addObstacle(const Polygon<Inexact>& obstacle);

	/// Adds a shield obstacle to each place to make sure that these nodes in
	/// the tree become leaves.
	void addShields();

	/// Removes all the arcs and nodes from the tree, so only the manually
	/// added nodes and obstacles remain.
	void clean();

	bool isReachable(const PolarPoint& parent_point, const PolarPoint& child_point) const;

  private:
	/// The restricting angle of this tree.
	Number<Inexact> m_restrictingAngle;
	/// The position of the root node.
	Point<Inexact> m_rootPosition;

	/// List of places (terminals) in this tree.
	std::vector<std::shared_ptr<Place>> m_places;
	/// List of all nodes in this tree.
	std::vector<std::shared_ptr<Node>> m_nodes;
	/// List of obstacles.
	std::vector<Obstacle> m_obstacles;

	/// The root node of this tree.
	std::shared_ptr<Node> m_root;

	friend class SpiralTreeUnobstructedAlgorithm;
	friend class SpiralTreeObstructedAlgorithm;
};

} // namespace cartocrow::flow_map

#endif //CARTOCROW_SPIRAL_TREE_H
