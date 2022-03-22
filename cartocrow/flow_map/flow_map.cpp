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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 10-09-2019
*/

#include "flow_map.h"

#include "cartocrow/flow_map/spiral_tree.h"

namespace cartocrow::flow_map {

void computeFlowMap(const Parameters& parameters, const std::vector<Place::Ptr>& places,
                    const size_t index_root, const std::vector<Region>& obstacles,
                    FlowTree::Ptr& tree) {
	// Computing Spiral tree: 2015 paper / Journal article.
	// Computing subdivision, thickening, and improvements: 2011 paper.

	// Notes Spiral journal (and 2015 paper):
	// Optimal flux tree consists of straight line segments and alpha-spiral segments. This requires the computation of tangents to alpha-spirals.. (Note the difference with the Spiral tree, where edges are either one or the other).
	// The spirals bounding spiral regions are evaluated in the range [0, pi*cot(beta)]. This implies that the first intersection on S+ and S- with 0<t occurs at t = pi*cot(beta).
	// Concern: how to efficiently (and correctly) compute the intersections of alpha-spirals. Note that this is always the intersection of an S+ and S- spiral.
	// It seems that after computing the spiral tree (in the Jave implementation), the arcs are replaced by straight-line segments (connecting terminal, root, and join nodes). This may lead to intersections in the tree.
	// Steiner points are the intersection of alpha-spirals (through 2 terminals) that as closer to the root. Recall that these must be either {Sp+ union Sq-} or {SP- union Sq+}.
	// Is explicitly computing the minimum spanning tree of the root and nodes a required component? It seems that this will follow implicitly from the algorithm.
	// NP-hard algorithm: MST = C1 -> replace edges by inward-going sequence of spirals -> join pairs for every even/odd sequence (and replace them by their join node) C2 -> repeat replacement step until 1 left.
	// Wavefront W of active nodes farther away than R from root; "keep tract of" join nodes for all of these? => NO, only in radial order (neighbors). Note that this radial order is based on the direction at which the spiral leaves the root, not the direction it has when at the node.
	// Algorithm (without obstacles): sort terminals by distance from r in priority queue; keep track of wavefront as balanced binary tree -> when encountering terminal in the queue: check whether it is reachable by a neighbor (connect and replace); when encountering join nodes (check whether both children are still active): connect children to join node and replace them in the wavefront by the join node. Then add join nodes with the neighbors to the queue.
	// Naive interpretation of algorithm with obstacles: 1. sweep a circle outward to convert domain P into reachable domain P' (in practice, this changes straight-line obstacles into spiral-line obstacles). 2. sweep the wavefront inward to connect terminals and join nodes to their parent while staying inside the reachable intervals.
	// Goal of computing the spiral trees is to draw flow trees; to this effect, is it necessary to draw the actual spirals? => No, but some steiner points may be necessary to prevent tree self-intersections.
	// When connecting reachable nodes (inside the spiral region of the other node), should I use a straight line or a beta-spiral with beta < alpha? => straight line.

	// Computing single spiral through two given points, unknown beta:
	// Note that the two points must have different distance to the root.
	// p = (R_p, phi_p) @ t=0 -> R_p(t) = R_p*e^{-t}; phi_p(t) = phi_p + tan(beta)*t
	// q = (R_q, phi_q) @ t=0 -> R_q(t) = R_q*e^{-t}; phi_q(t) = phi_q + tan(beta)*t; note that q must also be (R_p(t), phi_p(t)) @ t=?.
	// Note that the spiral moves towards the root as t goes to infinity. This also means that and S+ and S- will intersect an infinite number of times at R < R(0), ever closer to the root.
	// R_p(0) = R_p*e^0 = R_p
	// phi_p(0) = phi_p + tan(beta)*0 = phi_p
	// Assuming R_q < R_p and 0 < t:
	//    R_p(t) = R_p*e^{-t} = R_q
	//    phi_p(t) = phi_p + tan(beta)*t = phi_q  =>  tan(beta) = (phi_q - phi_p) / t  =>  beta = tan^-1((phi_q - phi_p) / t)
	//    e^{-t} = R_q / R_p  =>  t = -ln(R_q / R_p)
	//    =>  beta = tan^-1((phi_q - phi_p) / -ln(R_q / R_p))
	//
	// Computation reminder for changing base of logarithm: log_b(x) = log_k(x) / log_k(b)

	// Computing spiral intersections (two given points and given beta, unknown intersection):
	// Note that two such points will always have two intersections: {Sp+ union Sq-} and {SP- union Sq+}. Here we compute the intersection v = {Sp+ union Sq-}; the other can be computing by replacing beta by -beta.
	// Also note that if either point is in the spiral region of the other, the intersection will be on the edge of that spiral region and farther away from the root than the other point; in this case, we generally want to connect the point using a straight line instead of two spirals to the intersection.
	// v = (R_v, phi_v) @ t_p=?, t_q=?
	//    R_v = R_p*e^{-t_p}; phi_v = phi_p + tan(beta)*t_p
	//    R_v = R_q*e^{-t_q}; phi_v = phi_q + tan(-beta)*t_q
	//    R_p*e^{-t_p} = R_q*e^{-t_q}; phi_p + tan(beta)*t_p = phi_q + tan(-beta)*t_q
	//    e^{-t_p} = (R_q / R_p)*e^{-t_q}
	//    e^{-t_p} = e^ln(R_q / R_p)*e^{-t_q}
	//    e^{-t_p} = e^{ln(R_q / R_p) - t_q}
	//    -t_p = ln(R_q / R_p) - t_q  =>  t_q = ln(R_q / R_p) + t_p
	//    phi_p - phi_q + tan(beta)*t_p = tan(-beta)*t_q
	//    (phi_p - phi_q + tan(beta)*t_p) / tan(-beta) = t_q
	//    (phi_p - phi_q + tan(beta)*t_p) / tan(-beta) = ln(R_q / R_p) + t_p
	//    phi_p - phi_q + tan(beta)*t_p = tan(-beta)*ln(R_q / R_p) + tan(-beta)*t_p
	//    tan(beta)*t_p - tan(-beta)*t_p= tan(-beta)*ln(R_q / R_p) - phi_p + phi_q
	//    t_p = (tan(-beta)*ln(R_q / R_p) - phi_p + phi_q) / (tan(beta) - tan(-beta))
	//    =>  R_v = R_p*e^{-t_p}; phi_v = phi_p + tan(beta)*t_p  [fill in t_p]
	//
	// Note that according to the Java implementation, R_v can also be based on the dot product of the Cartesian points:
	//    R_v = sqrt( R_p * R_q * e^{-acos(p*q / R_p*R_q) / tan(beta)} )

	// Notes 2011 paper:
	// Spiral tree: every edge is either a straight line, or a beta-spiral  with beta = alpha (not beta <= alpha).
	// user defined buffer around obstacles
	// subdivision nodes should also have a dummy node per child such that these dummies are on a line touching each other. The starting direction at each dummy node should be the same as the ending direction of the edge from the parent node.
	// obstacle cost (F_obs) goes to infinity as the node gets closer to the obstacle to 'maintain the topology'. Note that this only works as long as the optimization steps are small enough.
	// Special case: subdivision nodes between a leaf node and join node (or root) have F_obs = 0 with regards to _that_ leaf node. Requires special edge tag?
	// How do we determine which obstacles to take into account for F_obs? Checking all obstacles for all nodes may be very costly. Specifically, try not to calculate costs for far away obstacles, where the result would be 0 cost.
	// In a few places (e.g. sec 4.2), the paper requires the "angle of an edge". Is this the angle relative to the positive x-axis, or the angle relative to the line through the root? According to Kevin: angle relative to root.
	// F_ar, "angle of the edges": what is meant here? Angle relative to following the parent edge further? The goal of F_ar seems to be to keep both angles as far away from pi/2 as possible, i.e. close to either 0 or pi.
	// Note that a Spiral tree is always a binary tree, but the subdivision step may also merge nodes to create a non-binary tree.
	// While minimizing the cost function, subdivision nodes must maintain their distance to the root. They may only change their position 'on the circle'.
	// Cost optimization uses steepest decent, so this moves the cost straight into a local minimum.
	// Section 4.3: adaptive epsilon by using a binary search on _all_ edges e and points p (obstacles and nodes). This sounds like an efficiency nightmare.
	// Note, adaptive subdivision during optimization. Adaptive merging (join nodes only) during optimization.
	// F_ar, F_b defined on outermost children; F_s defined on all children.
	// Section 4.4: Hermite splines are functionally identical to Bezier splines, except for their description. => Hermite splines are described by their endpoints and the first derivative at those points, as opposed to the same endpoints and several control points.
	// "Tangent at every point" should be interpreted as "tangent at the endpoints".
	// What is the "child of p"? Is this the child node? Becase taking the difference of the norms of the child and parent node does not really makes sense to me...
	// Taking the direction towards its parent at the leaf nodes would result in somewhat ugly curves (i.e. thay 'curve back' near the leaf node). I imagine there may be nicer final directions.
	// Implementing multiple flow trees, clustering nodes, and waypoints as extensions?

	// Computing the flow map composes three major steps:
	// 1. computing the spiral tree,
	// 2. subdividing and thickening the tree,
	// 3. improving the smoothness and clarity of the tree (and evade obstacles).

	CHECK_LE(0, index_root);
	CHECK_LT(index_root, places.size());
	const Point root = places[index_root]->position.to_cartesian();

	SpiralTree spiral_tree(root, parameters.restricting_angle);
	spiral_tree.addPlaces(places);
	//  spiral_tree.addObstacles(obstacles);  // TODO(tvl) disabled until computing the tree with obstructions is implemented.
	spiral_tree.compute();

	tree = std::make_shared<FlowTree>(spiral_tree);
}

} // namespace cartocrow::flow_map
