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

#include "spiral_tree.h"

#include <optional>
#include <ostream>

#include <glog/logging.h>

#include "cartocrow/core/circular_range.h"
#include "cartocrow/core/circulator.h"
#include "cartocrow/core/intersections.h"
#include "cartocrow/core/polar_segment.h"
#include "cartocrow/core/spiral_segment.h"

namespace cartocrow::flow_map {

Node::Node(const Place::Ptr& place) : place(place) {}

Node::ConnectionType Node::getType() const {
	if (parent == nullptr) {
		return ConnectionType::kRoot;
	} else {
		switch (children.size()) {
		case 0:
			return ConnectionType::kLeaf;
		case 1:
			return ConnectionType::kSubdivision;
		default:
			return ConnectionType::kJoin;
		}
	}
}

bool Node::isSteiner() const {
	return place == nullptr || (place->flow_in <= 0 && parent != nullptr);
}

SpiralTree::SpiralTree(const Point& root, const Number& restricting_angle)
    : m_restricting_angle(restricting_angle), m_root_translation(Point(CGAL::ORIGIN) - root) {
	CHECK_GT(restricting_angle, 0);
	CHECK_LE(restricting_angle, M_PI_2);
}

void SpiralTree::addPlaces(const std::vector<Place::Ptr>& places) {
	clean();

	for (const Place::Ptr& place : places) {
		if (place->flow_in > 0) {
			m_nodes.push_back(std::make_shared<Node>(place));
		}
	}
}

void SpiralTree::addObstacles(const std::vector<Region>& obstacles) {
	clean();

	for (const Region& obstacle : obstacles) {
		for (const Polygon_with_holes& polygon : obstacle.shape) {
			addObstacle(polygon);
		}
	}
}

void SpiralTree::compute() {
	if (true) { //obstacles_.empty())  // TODO(tvl) commented out while code is not complete.
		computeUnobstructed();
	} else {
		computeObstructed();
	}
}

void SpiralTree::handleRootEvent(const Event& event, Wavefront& wavefront) {
	Node::Ptr root = event.node;

	// if we reached the root, then the wavefront should have only one
	// node left
	CHECK_EQ(wavefront.size(), 1);
	Node::Ptr node = wavefront.begin()->second.node;
	CHECK_NOTNULL(node);

	// connect the remaining node to the root
	root->children.push_back(node);
	node->parent = root;

	std::cout << "root node " << root->place->id << ": connected to " << node->place->id << std::endl;

	wavefront.clear();
}

std::optional<SpiralTree::Wavefront::iterator> SpiralTree::handleJoinEvent(const Event& event,
                                                                           Wavefront& wavefront) {
	CHECK_EQ(event.node->children.size(), 2);

	// if a child is not active anymore, the event is invalid
	if (event.node->children[0]->parent != nullptr || event.node->children[1]->parent != nullptr) {
		return std::nullopt;
	}

	// add the join node to the wavefront and the collection of nodes
	const Number angle = event.relative_position.phi();
	Wavefront::iterator node_iter = wavefront.emplace(angle, event).first;
	m_nodes.push_back(event.node);

	std::cout << "join node " << event.node->place->id << ": connected to "
	          << event.node->children[0]->place->id << " and " << event.node->children[1]->place->id
	          << std::endl;

	// connect the children to the join node
	event.node->children[0]->parent = event.node;
	event.node->children[1]->parent = event.node;

	// remove the children from the wavefront
	CHECK_GE(wavefront.size(), 3);
	wavefront.erase(--make_circulator(node_iter, wavefront));
	wavefront.erase(++make_circulator(node_iter, wavefront));

	return node_iter;
}

SpiralTree::Wavefront::iterator SpiralTree::handleLeafEvent(Event& event, Wavefront& wavefront) {

	const Number angle = event.relative_position.phi();

	if (!wavefront.empty()) {

		// check the neighbors of the new leaf in the wavefront for reachability
		Circulator<Wavefront> node_circ = make_circulator(wavefront.lower_bound(angle), wavefront);
		if (isReachable(event.relative_position, node_circ->second.relative_position) ||
		    isReachable(event.relative_position, (--node_circ)->second.relative_position)) {
			// due to || being short-circuiting, node_circ now points to the
			// reachable one
			Event reachable_neighbor = node_circ->second;

			// check whether the nodes coincide
			if (event.relative_position == reachable_neighbor.relative_position) {
				// replace the event node by the join node
				reachable_neighbor.node->place = event.node->place;
				event.node = reachable_neighbor.node;
			} else {
				// connect the event node to the neighbor
				event.node->children.push_back(reachable_neighbor.node);
				reachable_neighbor.node->parent = event.node;
			}

			// remove the neighbor from the wavefront
			wavefront.erase(node_circ);
		}
	}

	std::cout << "leaf node " << event.node->place->id << ": added" << std::endl;

	return wavefront.emplace(angle, event).first;
}

void SpiralTree::computeUnobstructed() {

	// we maintain a wavefront as a BST of events, with their angle around the
	// root as the key
	Wavefront wavefront;

	// insert all terminals into the event queue
	EventQueue events;
	for (const Node::Ptr& node : m_nodes) {
		CHECK_NOTNULL(node->place);
		const PolarPoint relative_position(node->place->position, m_root_translation);
		events.push(Event(node, relative_position));
	}

	// main loop, handle all events
	while (!events.empty()) {
		Event event = events.top();
		events.pop();

		// are we looking at the root?
		if (event.relative_position.R() == 0) {
			handleRootEvent(event, wavefront);
			break;
		}

		std::optional<Wavefront::iterator> node_iter;

		// are we looking at a join node?
		if (event.node->children.size() > 1) {
			node_iter = handleJoinEvent(event, wavefront);
		} else {
			node_iter = handleLeafEvent(event, wavefront);
		}

		if (!node_iter || wavefront.size() < 2) {
			continue;
		}

		// Add join nodes with the neighbors to the event queue.
		{
			// Clockwise.
			Circulator<Wavefront> cw_iter = --make_circulator(*node_iter, wavefront);

			const Spiral spiral_left(event.relative_position, -m_restricting_angle);
			const Spiral spiral_right(cw_iter->second.relative_position, m_restricting_angle);

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
			const std::string id =
			    "[" + cw_iter->second.node->place->id + "+" + event.node->place->id + "]";
			PolarPoint absolute_position(intersection, -m_root_translation);
			join->place = std::make_shared<Place>(id, absolute_position);
#endif // NDEBUG
		}

		{
			// Counter-clockwise.
			Circulator<Wavefront> ccw_iter = ++make_circulator(*node_iter, wavefront);

			const Spiral spiral_left(ccw_iter->second.relative_position, -m_restricting_angle);
			const Spiral spiral_right(event.relative_position, m_restricting_angle);

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
			const std::string id =
			    "[" + event.node->place->id + "+" + ccw_iter->second.node->place->id + "]";
			PolarPoint absolute_position(intersection, -m_root_translation);
			join->place = std::make_shared<Place>(id, absolute_position);
#endif // NDEBUG
		}
	}
}

void SpiralTree::setRoot(const Point& root) {
	m_root_translation = Point(CGAL::ORIGIN) - root;
	clean();
}

void SpiralTree::setRestrictingAngle(const Number& restricting_angle) {
	CHECK_GT(restricting_angle, 0);
	CHECK_LT(restricting_angle, M_PI_2);
	m_restricting_angle = restricting_angle;
	clean();
}

void SpiralTree::clean() {
	size_t num_places = 0;

	// Clean the node connections.
	for (Node::Ptr& node : m_nodes) {
		if (node->place == nullptr) {
			break;
		}
		++num_places;

		node->parent = nullptr;
		node->children.clear();
	}

	// Remove support nodes, e.g. join nodes.
	m_nodes.resize(num_places);
}

bool SpiralTree::isReachable(const PolarPoint& parent_point, const PolarPoint& child_point) const {
	if (parent_point == child_point) {
		return true;
	}

	const Spiral spiral(child_point, parent_point);
	return std::abs(spiral.angle_rad()) <= m_restricting_angle;
}

void SpiralTree::addObstacle(const Polygon_with_holes& polygon) {
	// Ignore the holes of the obstacle: flow cannot cross the obstacle boundary.
	const Polygon& boundary = polygon.outer_boundary();
	if (boundary.is_empty()) {
		return;
	}

	CHECK_NE(boundary.oriented_side(getRoot()), CGAL::ON_BOUNDED_SIDE)
	    << "Root inside an obstacle.";

	Obstacle& obstacle = m_obstacles.emplace_back();
	for (Polygon::Vertex_const_iterator vertex_iter = boundary.vertices_begin();
	     vertex_iter != boundary.vertices_end(); ++vertex_iter) {
		obstacle.emplace_back(*vertex_iter, m_root_translation);
	}

	// Enforce counter-clockwise obstacles for a canonical arrangement.
	// Note that this is necessary to be able check on which side of the vertices the interior of the polygon lies.
	if (!boundary.is_counterclockwise_oriented()) {
		obstacle.reverse();
	}

	// Add vertices for the points closest to the root as well as spiral points.
	// The wedge with the root as apex and boundaries through a closest point and a spiral point (on the same edge) has a fixed angle.
	const Number phi_offset = M_PI_2 - m_restricting_angle;
	CHECK_LT(0, phi_offset);

	Obstacle::iterator vertex_prev = --obstacle.end();
	for (Obstacle::iterator vertex_iter = obstacle.begin(); vertex_iter != obstacle.end();
	     vertex_prev = vertex_iter++) {
		const PolarSegment edge(*vertex_prev, *vertex_iter);
		const PolarPoint closest = edge.SupportingLine().foot();

		// The spiral points have fixed R and their phi is offset from the phi of the closest by +/- phi_offset.
		const Number R_s = closest.R() / std::sin(m_restricting_angle);

		const int sign = vertex_prev->phi() < vertex_iter->phi() ? -1 : 1;
		const Number phi_s_prev = closest.phi() - sign * phi_offset;
		const Number phi_s_next = closest.phi() + sign * phi_offset;

		// The closest point and spiral points must be added ordered from p to q and only if they are on the edge.
		if (edge.ContainsPhi(phi_s_prev)) {
			obstacle.insert(vertex_iter, PolarPoint(R_s, phi_s_prev));
		}
		if (edge.ContainsPhi(closest.phi())) {
			obstacle.insert(vertex_iter, closest);
		}
		if (edge.ContainsPhi(phi_s_next)) {
			obstacle.insert(vertex_iter, PolarPoint(R_s, phi_s_next));
		}
	}
}

// Note, the following code has been commented out when the GeoViz implementation project had to be cut short.
// This may be used as a starting point to continue implementation at some later date.

/*

///////////////////////////
/// Obstructed ST impl. ///
///////////////////////////


namespace obst
{

// This following classes are here to provide a topological embedding for the sweep circle, that is: faces (intervals) incident to edges (spiral or straight line) and nodes (node, vertex, and vanishing events).
// This embedding should enable easy traversal of these incident elements. For example, a vertex event can exactly determine which edges and faces are incident without breaking due to point location roundoff errors.
// While this embedding is very similar to the CGAL halfedge data structure, we cannot use that structure because:
// * it requires the halfedges to specify the next edge that shares the same face (to enable traversing the boundary of the face). In our case, we only know the edges intersected by the sweep circle (and possibly those intersected earlier).
// * each face stores one halfedge, while we want the face to store both edges intersected by the sweep circle.


// Types for the halfedge embedding (connectivity between the different pieces)
struct EmbeddingNode;
struct EmbeddingHalfedge;
struct EmbeddingFace;

// Types used for functional processing (event order, segments that can be intersected, interval types).
struct NodeElement;  // Event.
struct EdgeElement;  // Spiral segment, polar segment, or halfline (note that the edge element is shared by both haldedges).
struct FaceElement;  // Interval.


struct EmbeddingNode
{
	using NodePtr = std::shared_ptr<EmbeddingNode>;
	using HalfedgePtr = std::shared_ptr<EmbeddingHalfedge>;

	using NodeElementPtr = std::shared_ptr<NodeElement>;

	static void SetEvent(NodePtr& node, NodeElementPtr& event);

	bool IsValid() const;

	HalfedgePtr edge;

	NodeElementPtr event;
}; // struct EmbeddingNode

struct EmbeddingHalfedge
{
	using NodePtr = std::shared_ptr<EmbeddingNode>;
	using HalfedgePtr = std::shared_ptr<EmbeddingHalfedge>;
	using FacePtr = std::shared_ptr<EmbeddingFace>;

	using EdgeElementPtr = std::shared_ptr<EdgeElement>;

	void SetSegment(const EdgeElementPtr& segment);
	static void SetNode(HalfedgePtr& edge, NodePtr& node);

	bool IsValid() const;

	static HalfedgePtr ConstructWithOpposite();

	NodePtr node;
	HalfedgePtr opposite;
	FacePtr face;

	EdgeElementPtr segment;
}; // struct EmbeddingHalfedge


struct EmbeddingFace
{
	using HalfedgePtr = std::shared_ptr<EmbeddingHalfedge>;
	using FaceElementPtr = std::shared_ptr<FaceElement>;
	using FacePtr = std::shared_ptr<EmbeddingFace>;

	static FacePtr ConstructBetweenEdges(const HalfedgePtr& edge_cw, const HalfedgePtr& edge_ccw);

	static void SetEdgeCw(const HalfedgePtr& edge, FacePtr& face);
	static void SetEdgeCcw(const HalfedgePtr& edge, FacePtr& face);

	static void LinkInterval(FacePtr& face, FaceElementPtr& interval);

	bool IsValid() const;

	HalfedgePtr edge_cw, edge_ccw;

	FaceElementPtr interval;
}; // struct EmbeddingFace

void EmbeddingHalfedge::SetSegment(const EdgeElementPtr& segment)
{
	this->segment = segment;
	if (opposite)
		opposite->segment = segment;
}

void EmbeddingHalfedge::SetNode(HalfedgePtr& edge, NodePtr& node)
{
	edge->node = node;
	node->edge = edge;
}

bool EmbeddingHalfedge::IsValid() const
{
	// Note that an edge is not forced to have a node, so their correct linkage is not part of the validity check.
	return
		(bool)opposite && this == opposite->opposite.get() &&
		(bool)face && (this == face->edge_cw.get() || this == face->edge_ccw.get());
}

// Construct a two halfedges of a segment.
EmbeddingHalfedge::HalfedgePtr EmbeddingHalfedge::ConstructWithOpposite()
{
	HalfedgePtr x_axis_halfedge_1 = std::make_shared<EmbeddingHalfedge>();
	HalfedgePtr x_axis_halfedge_2 = std::make_shared<EmbeddingHalfedge>();
	x_axis_halfedge_1->opposite = x_axis_halfedge_2;
	x_axis_halfedge_2->opposite = x_axis_halfedge_1;
}

EmbeddingFace::FacePtr EmbeddingFace::ConstructBetweenEdges(const HalfedgePtr& edge_cw, const HalfedgePtr& edge_ccw)
{
	FacePtr face = std::make_shared<EmbeddingFace>();
	EmbeddingFace::SetEdgeCw(edge_cw, face);
	EmbeddingFace::SetEdgeCcw(edge_ccw, face);
}

void EmbeddingFace::SetEdgeCw(const HalfedgePtr& edge, FacePtr& face)
{
	face->edge_cw = edge;
	edge->face = face;
}

void EmbeddingFace::SetEdgeCcw(const HalfedgePtr& edge, FacePtr& face)
{
	face->edge_ccw = edge;
	edge->face = face;
}

bool EmbeddingFace::IsValid() const
{
	return
		(bool)edge_cw && this == edge_cw->face.get() &&
		(bool)edge_ccw && this == edge_ccw->face.get();
}






// TODO(tvl) rename Event?
struct NodeElement
{
	using EmbeddingNodePtr = std::shared_ptr<EmbeddingNode>;

	NodeElement(const PolarPoint& relative_position) : relative_position(relative_position) {}

	bool IsValid() const
	{
		if (embedding)
			return embedding->IsValid();
	}

	PolarPoint relative_position;

	EmbeddingNodePtr embedding;
}; // struct NodeElement


struct NodeEvent : public NodeElement
{
	NodeEvent(const PolarPoint& relative_position, const Node::Ptr& node) :
		NodeElement(relative_position), node(node) {}

		// Node events have a spiral tree node.
	Node::Ptr node;
}; // struct NodeEvent

struct VertexEvent : public NodeElement
{
	using EdgeElementPtr = std::shared_ptr<EdgeElement>;

	VertexEvent
	(
		const PolarPoint& relative_position,
		const EdgeElementPtr& edge_prev,
		const EdgeElementPtr& edge_next
	) : NodeElement(relative_position), edge_prev(edge_prev), edge_next(edge_next) {}

	// Obstacle vertex events need to know their polygon edges.
	EdgeElementPtr edge_prev, edge_next;
}; // struct VertexEvent

struct VanishingEvent : public NodeElement
{
	VanishingEvent(const PolarPoint& relative_position, FaceElement* interval) :
		NodeElement(relative_position), interval(interval) {}

	// Instead of active/deactive, check whether the interval still has this event as vanishing event.

	FaceElement* interval;
}; // struct VanishingEvent


//std::ostream& operator<<(std::ostream& os, const Event& event)
//{
//  os << "Event @ " << event.relative_position << std::endl;
//
//  os << (event.node == nullptr ? "vanishing" : "node/vertex");
//
//  if (event.edges[0])
//    os << "Edge 0: " << (*event.edges[0]) << std::endl;
//  if (event.edges[1])
//    os << "Edge 1: " << (*event.edges[1]) << std::endl;
//}

using EventPtr = std::shared_ptr<NodeElement>;

struct CompareEvents
{
	bool operator()(const EventPtr& a, const EventPtr& b) const
	{
		return a->relative_position.R() < b->relative_position.R();
	}
}; // struct CompareEvents

struct CompareEventsReverse : public CompareEvents
{
	bool operator()(const EventPtr& a, const EventPtr& b) const
	{
		return CompareEvents::operator()(b, a);
	}
}; // struct CompareEventsReverse

void EmbeddingNode::SetEvent(NodePtr& node, NodeElementPtr& event)
{
	node->event = event;
	event->embedding = node;
}

bool EmbeddingNode::IsValid() const
{
	return
		(bool)edge && this == edge->node.get() &&
		(bool)event && this == event->embedding.get();
}


struct EdgeElement
{
	virtual ~EdgeElement() {}

	virtual bool ContainsR(const Number& R) const = 0;

	virtual Number ComputePhi(const Number& R) const = 0;

	virtual std::ostream& print(std::ostream& os) const = 0;
}; // struct EdgeElement

std::ostream& operator<<(std::ostream& os, const EdgeElement& edge)
{
	edge.print(os);
	return os;
}

template<class T_>
struct EdgeElement_ : public EdgeElement
{
	EdgeElement_(const T_& segment) : segment(segment) {}

	std::ostream& print(std::ostream& os) const
	{
		os << segment;
		return os;
	}

	T_ segment;
}; // class EdgeElement_

template<class Edge1_, class Edge2_>
PolarPoint Intersect(const Edge1_& edge_1, const Edge2_& edge_2);

template<class T1_, class T2_>
PolarPoint Intersect(const EdgeElement_<T1_>& edge_1, const EdgeElement_<T2_>& edge_2)
{
	PolarPoint intersections[2];
	const int num = ComputeIntersections(edge_1.segment, edge_2.segment, intersections);
	CHECK_LT(0, num);

	if (1 < num && intersections[1].R() < intersections[0].R())
		return intersections[1];

	return intersections[0];
}

class EdgeStraight : public EdgeElement_<PolarSegment>
{
	using Base = EdgeElement_<PolarSegment>;

 public:
	EdgeStraight(const PolarPoint& a, const PolarPoint& b) : Base(PolarSegment(a, b)) {}

	bool ContainsR(const Number& R) const { return segment.ContainsR(R); }

	Number ComputePhi(const Number& R) const
	{
		// Note, due to minor round-off errors, there can be two very similar phi values near the point closest to the root.
		Number phi[2];
		CHECK_LT(0, segment.CollectPhi(R, phi));

		return phi[0];
	}
}; // class EdgeStraight


class EdgeSpiral : public EdgeElement_<SpiralSegment>
{
	using Base = EdgeElement_<SpiralSegment>;

 public:
	EdgeSpiral(const PolarPoint& a, const PolarPoint& b) : Base(SpiralSegment(a, b)) {}

	bool ContainsR(const Number& R) const { return segment.ContainsR(R); }

	Number ComputePhi(const Number& R) const
	{
		return segment.ComputePhi(R);
	}
}; // class EdgeSpiral


class EdgeHalfline : public EdgeElement_<Spiral>
{
	using Base = EdgeElement_<Spiral>;

 public:
	EdgeHalfline(const PolarPoint& point) : Base(Spiral(point, 0)) {}

	bool ContainsR(const Number& R) const { return true; }

	Number ComputePhi(const Number& R) const
	{
		return segment.ComputePhi(R);
	}
}; // class EdgeHalfline


// TODO(tvl) rename Interval?
struct FaceElement
{
	using FacePtr = std::shared_ptr<EmbeddingFace>;
	using HalfedgePtr = std::shared_ptr<EmbeddingHalfedge>;

	using NodeElementPtr = std::shared_ptr<NodeElement>;
	using FaceElementPtr = std::shared_ptr<FaceElement>;

	FaceElement() { UpdateVanishingEvent(); }

	bool IsValid() const
	{
		if (face)
			return face->edge_cw->IsValid() && face->edge_ccw->IsValid(); //&& face->IsValid()
	}

	const NodeElementPtr& Vanishing() const { return vanishing_; }

	void SetEdges(const HalfedgePtr& edge_cw, const HalfedgePtr& edge_ccw)
	{
		face->edge_cw = edge_cw;
		face->edge_ccw = edge_ccw;
		UpdateVanishingEvent();
	}
	void SetEdgeCw(const HalfedgePtr& edge_cw)
	{
		face->edge_cw = edge_cw;
		UpdateVanishingEvent();
	}
	void SetEdgeCcw(const HalfedgePtr& edge_ccw)
	{
		face->edge_ccw = edge_ccw;
		UpdateVanishingEvent();
	}

	FacePtr face;

 private:

	void UpdateVanishingEvent()
	{
//    if (vanishing_)
//      vanishing_->Deactivate();
//
//    if (!edge_cw_ || !edge_ccw_)
//    {
//      vanishing_ = nullptr;
//      return;
//    }

		const PolarPoint position = Intersect(*face->edge_cw->segment, *face->edge_ccw->segment);
		vanishing_ = std::make_shared<VanishingEvent>(position, this);
	}

	NodeElementPtr vanishing_;
}; // struct FaceElement

struct ObstacleInterval : public FaceElement
{
	using FacePtr = std::shared_ptr<EmbeddingFace>;

	ObstacleInterval() : FaceElement() {}
}; // struct ObstacleInterval

struct OpenInterval : public FaceElement
{
	using FacePtr = std::shared_ptr<EmbeddingFace>;

	OpenInterval(const Node::Ptr& tag = nullptr) : FaceElement(), tag(tag) {}

	// Note that an open interval referencing a connected node counts as a free interval.
	bool IsFree() const { return tag == nullptr || tag->parent != nullptr; }

	Node::Ptr tag;
}; // struct OpenInterval

using IntervalPtr = std::shared_ptr<FaceElement>;

// TODO(tvl) make part of sweep circle?
class CompareIntervals
{
 public:
	CompareIntervals(const Number& R) : R(R) {}

	bool operator()(const IntervalPtr& a, const IntervalPtr& b) const
	{
		// Compare the clockwise edges at the current R.
		// Note that these edges must always have a point at the specified R,
		// because the events are processed in order based on R.
		// Also note that if two edges are found to intersect,
		// they should be replaced by their subdivided counterparts to prevent overlapping intervals.
		CHECK_LT(0, R);

		using EdgePtr = std::shared_ptr<EdgeElement>;

		const EdgePtr& edge_a = a->face->edge_cw->segment;
		const EdgePtr& edge_b = b->face->edge_cw->segment;
		CHECK(edge_a->ContainsR(R));
		CHECK(edge_b->ContainsR(R));

		const Number phi_a = edge_a->ComputePhi(R);
		const Number phi_b = edge_b->ComputePhi(R);

		// Note that edges may cross the x-axis, which should change the order of the intervals.
		// i.e. the interval should move from the front of the set to the end or vice versa.
		return phi_a < phi_b;
	}

	const Number& R;
}; // struct CompareIntervals

void EmbeddingFace::LinkInterval(FacePtr& face, FaceElementPtr& interval)
{
	face->interval = interval;
	interval->face = face;
}



// Note, the following implements a structure similar to a half-edge data-structure.
// We should probably explicitly implement to incidence relations between these classes as well.

enum SweepDirection
{
	kInward,
	kOutward
};

template<SweepDirection D_>
struct SweepCompareEvents;

template<>
struct SweepCompareEvents<SweepDirection::kInward>
{
	using Type = CompareEvents;
};

template<>
struct SweepCompareEvents<SweepDirection::kOutward>
{
	using Type = CompareEventsReverse;
};

template<SweepDirection D_>
class SweepCircle
{
 public:
	using CompareEvents = typename SweepCompareEvents<D_>::Type;

	// Note that the intervals order may be invalidated while changing R; specifically, the first and last elements may have to be moved to the other side of the collection.
	// For this reason, the intervals are not stored in a collection that assumes correct order.
	// Instead, the collection is regularly resorted.
	using IntervalPtr = std::shared_ptr<FaceElement>;
	using IntervalSet = std::vector<IntervalPtr>;

	using EventPtr = std::shared_ptr<NodeElement>;
	using Queue = std::priority_queue<EventPtr, std::vector<EventPtr>, CompareEvents>;

	SweepCircle(const Node::Ptr& root);

	SweepDirection Direction() const { return D_; }

	bool IsValid() const
	{
		bool valid = true;
		for (const IntervalPtr& interval : intervals)
			valid &= interval->IsValid();
		for (const EventPtr& event : queue)
			valid &= event->IsValid();
		return valid;
	}

	Number R;
	IntervalSet intervals;
	Queue queue;
}; // class SweepCircle


template<>
SweepCircle<SweepDirection::kInward>::SweepCircle(const Node::Ptr& root) : R(-1)
{
	EmbeddingHalfedge::HalfedgePtr halfedge = EmbeddingHalfedge::ConstructWithOpposite();

	EmbeddingHalfedge::EdgeElementPtr x_axis = std::make_shared<EdgeHalfline>(PolarPoint(1, 0));  // Relevant: edge geometry.
	halfedge->SetSegment(x_axis);

	EmbeddingFace::FacePtr face = EmbeddingFace::ConstructBetweenEdges(halfedge, halfedge->opposite);

	EmbeddingFace::FaceElementPtr interval = std::make_shared<OpenInterval>();  // Relevant: interval is open.
	EmbeddingFace::LinkInterval(face, interval);

	intervals.push_back(interval);

	CHECK(halfedge->IsValid());
	CHECK(halfedge->opposite->IsValid());
	CHECK(face->IsValid());
}

template<>
SweepCircle<SweepDirection::kOutward>::SweepCircle(const Node::Ptr& root) : R(0)
{
	EmbeddingHalfedge::HalfedgePtr halfedge = EmbeddingHalfedge::ConstructWithOpposite();

	EmbeddingHalfedge::EdgeElementPtr x_axis = std::make_shared<EdgeHalfline>(PolarPoint(1, 0));  // Relevant: edge geometry.
	halfedge->SetSegment(x_axis);

	EmbeddingFace::FacePtr face = EmbeddingFace::ConstructBetweenEdges(halfedge, halfedge->opposite);

	EmbeddingFace::FaceElementPtr interval = std::make_shared<OpenInterval>();  // Relevant: interval is open.
	EmbeddingFace::LinkInterval(face, interval);

	intervals.push_back(interval);

	// Note that the root event specifically is not inserted into the event queue.
	EmbeddingNode::NodeElementPtr root_event = std::make_shared<NodeEvent>(PolarPoint(0, 0), root);  // Relevant: node position.

	EmbeddingNode::NodePtr root_node = std::make_shared<EmbeddingNode>();
	EmbeddingNode::SetEvent(root_node, root_event);
	EmbeddingHalfedge::SetNode(halfedge, root_node);

	CHECK(halfedge->IsValid());
	CHECK(halfedge->opposite->IsValid());
	CHECK(face->IsValid());
	CHECK(root_node->IsValid());
}




// Strictly speaking, the wavefront should be the collection of unconnected nodes that have been passed by the sweep circle.
// In practice, we keep track of the intervals on this sweep circle instead, ordered by their edges intersecting the sweep circle.
class SweepCircleOld
{
 public:

	SweepCircleOld(): R(-1) {}


	// TODO(tvl) replace add_events by output iterator? Or insert directly into the event queue?
	void Handle(const Event::Ptr& event, std::vector<Event::Ptr>& add_events)
	{
		if (event->IsNode())
			HandleTerminal(event, add_events);
		else if (event->IsVertex())
			HandleVertex(event, add_events);
		else if (event->IsVanishing())
			HandleVanishing(event, add_events);
		else
			CHECK(false);
	}

	void HandleTerminal(const Event::Ptr& event, std::vector<Event::Ptr>& add_events)
	{
		//TODO(tvl) implement.
		CHECK(false);
	}

	void HandleVertex(const Event::Ptr& event, std::vector<Event::Ptr>& add_events)
	{
		IntervalSet::iterator interval_iter = FindInterval(event);

		const OpenInterval* open_interval = (*interval_iter)->AsOpenInterval();
		if (open_interval == nullptr)
		{
			// If v is in an obstacle interval, then we add a free interval, where the endpoints of the interval trace the edges of P′ connected to v.
			// Note that these descriptions are imprecise, as there can be overlap in obstacles.
			//
			// Instead, first check whether the edges associated with the event are on the "same side of the event" (i.e. both moving towards or away from the root).
			// If they are, then this vertex indicates either the start or end of an obstacle: which one determines whether to start and obstacle interval, or to start a free/reachable interval.
			// If they are not, then the vertex should separate an obstacle interval from a non-obstacle interval, where the obstacle interval should start following the next edge connected to the vertex.
			// Whenever a reachable interval continues after this event, it may either start following the next edge connected to the vertex, or it may start following a spiral edge containing the vertex; in the last case, a new free interval should fill the space between the new spiral and the next edge of the vertex.



		}
		else if (open_interval->IsFree())
		{
			//If v is in a free interval, then we add an obstacle interval, where the endpoints of the interval trace the edges of P′ connected to v.



		}
		else
		{
			// Otherwise, v is in a reachable interval or at the endpoint between a reachable interval and an obstacle interval.



		}


	}

	void MergeSimilarNeighbors(IntervalSet::iterator interval_iter, IntervalSet::iterator neighbor_iter_cw, IntervalSet::iterator neighbor_iter_ccw, std::vector<Event::Ptr>& add_events)
	{
		// Merge the neighbors into the clockwise one and remove the other two intervals.
		EdgePtr edge_ccw = (*neighbor_iter_ccw)->edge_ccw();
		(*neighbor_iter_cw)->SetEdgeCcw(edge_ccw);
		add_events.push_back((*neighbor_iter_cw)->Vanishing());

		intervals.erase(neighbor_iter_ccw);
		intervals.erase(interval_iter);
	}

	void RemoveIntervalFollowEdgeCw(IntervalSet::iterator interval_iter, IntervalSet::iterator neighbor_iter_cw, IntervalSet::iterator neighbor_iter_ccw, std::vector<Event::Ptr>& add_events)
	{
		// Adjust the counter-clockwise neighbor to follow the edge of the clockwise neighbor and remove the middle interval.
		EdgePtr edge = (*neighbor_iter_cw)->edge_ccw();
		(*neighbor_iter_ccw)->SetEdgeCw(edge);
		add_events.push_back((*neighbor_iter_ccw)->Vanishing());

		intervals.erase(interval_iter);
	}

	void RemoveIntervalFollowEdgeCcw(IntervalSet::iterator interval_iter, IntervalSet::iterator neighbor_iter_cw, IntervalSet::iterator neighbor_iter_ccw, std::vector<Event::Ptr>& add_events)
	{
		// Adjust the clockwise neighbor to follow the edge of the counter-clockwise neighbor and remove the middle interval.
		EdgePtr edge = (*neighbor_iter_ccw)->edge_cw();
		(*neighbor_iter_cw)->SetEdgeCw(edge);
		add_events.push_back((*neighbor_iter_cw)->Vanishing());

		intervals.erase(interval_iter);
	}

	void HandleVanishing(const EventPtr& event, std::vector<EventPtr>& add_events)
	{
		if (!event->IsActive())
			return;

		// Find the interval to vanish and its neighbors.
		IntervalSet::iterator interval_iter = FindInterval(event);
		IntervalSet::iterator neighbor_iter_cw = --make_circulator(interval_iter, intervals);
		IntervalSet::iterator neighbor_iter_ccw = ++make_circulator(interval_iter, intervals);

		const OpenInterval* open_cw = (*neighbor_iter_cw)->AsOpenInterval();
		const OpenInterval* open_ccw = (*neighbor_iter_ccw)->AsOpenInterval();

		// Obstacle neighbors take precedence.
		if (open_cw == nullptr && open_ccw == nullptr)
		{
			MergeSimilarNeighbors(interval_iter, neighbor_iter_cw, neighbor_iter_ccw, add_events);
		}
		else if (open_cw == nullptr)
		{
			RemoveIntervalFollowEdgeCw(interval_iter, neighbor_iter_cw, neighbor_iter_ccw, add_events);
		}
		else if (open_ccw == nullptr)
		{
			RemoveIntervalFollowEdgeCcw(interval_iter, neighbor_iter_cw, neighbor_iter_ccw, add_events);
		}
		else if (open_cw->IsFree() && open_ccw->IsFree())
		{
			MergeSimilarNeighbors(interval_iter, neighbor_iter_cw, neighbor_iter_ccw, add_events);
		}
		else if (open_cw->IsFree())
		{
			// The reachable neighbor takes precedence.
			RemoveIntervalFollowEdgeCcw(interval_iter, neighbor_iter_cw, neighbor_iter_ccw, add_events);
		}
		else if (open_ccw->IsFree())
		{
			// The reachable neighbor takes precedence.
			RemoveIntervalFollowEdgeCw(interval_iter, neighbor_iter_cw, neighbor_iter_ccw, add_events);
		}
		else if (open_cw->tag == open_ccw->tag)
		{
			// TODO(tvl) check:
			// According to the paper, we should keep both neighbors, separated by the positive spiral from this vanishing event.
			// However, this seems strange when we could also just merge the two neighbors.
			MergeSimilarNeighbors(interval_iter, neighbor_iter_cw, neighbor_iter_ccw, add_events);
		}
		else
		{
			// Join the two tags.
			// TODO(tvl) implement.
			CHECK(false);
		}
	}

	IntervalSet::iterator FindInterval(const PolarPoint& position)
	{
		CHECK(!intervals.empty());

		// Before we return the interval containing the event, we may have to update the collection.
		// Specifically, the intervals near the end of the collection may have to be moved to the other side of the collection.
		R = position.R();
		if (intervals.size() == 1)
			return intervals.begin();

		CompareIntervals compare;
		if (!compare(*intervals.begin(), *--intervals.end()))
			std::sort(intervals.begin(), intervals.end(), compare);

		// We search for the interval using a dummy interval.
		EdgePtr edge = std::make_shared<EdgeHalfline>(position);
		IntervalPtr dummy = std::make_shared<Interval>(R, edge, edge);

		// Compute the interval that is counter-clockwise of the event.
		IntervalSet::iterator ccw_iter = std::upper_bound(intervals.begin(), intervals.end(), dummy, compare);
		if (ccw_iter == intervals.begin())
			ccw_iter = intervals.end();

		// Return the interval before that.
		return --ccw_iter;
	}

	IntervalSet::iterator FindInterval(const EventPtr& event)
	{
		IntervalSet::iterator iter = FindInterval(event->relative_position);
		if (event->IsVanishing())
		{
			CHECK_EQ(event, (*iter)->VanishingEvent());

			// Make sure the interval is the one linked to this vanishing event.
//      while ((*iter)->VanishingEvent() != event)
//      {
//        CHECK_NE(intervals.begin(), iter);
//        --iter;
//      }
		}
		else if (event->IsVertex())
		{
			// Make sure that the interval is correct: either both edges of the event lie on the same side of the event, or the event is on the boundary of the interval.
			event->edges[0]



		}

		return iter;
	}
}; // class SweepCircleOld


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
	//   - 1 parent (tagged) node/vertex: node/vertex n 'inside' interval, farther from the root (to connect to next node touching interval). This is actually the point in the region farthest from the root. (note that farthest should be replaced by closest for the reverse order)
	//   - Could we just mark free intervals as 'reachable' intervals without a parent?
	// Stored in (balanced) search tree by their endpoints, i.e. their edges. The order of the endpoints never changes: when this would be the case, a new region is inserted. Can we store only the left (or right) edge with each interval?
	//
	// Maybe: an interval is marked as either "obstacle" or "open". An open interval can have a tagged event (node/vertex); an open interval that either has no tagged event or that has a tagged event that is already assigned a parent is called a "free interval".
	// Is merging newly freed up intervals really necessary? Isn't this just a case of more clutter without actual problems?
	//
	// Event (per point type).

//  obst::GreedyAlgorithm algorithm;

	// Note, the reachable region should really be made explicit: it is necessary to make sure that join nodes constructed in the second pass are not constructed outside the reachable region.


	using EventPtr = obst::Event::Ptr;
	using Queue = std::priority_queue<EventPtr, std::vector<EventPtr>, obst::CompareEvents>;
	using ReverseQueue = std::priority_queue<EventPtr, std::vector<EventPtr>, obst::CompareEventsReverse>;

	//Queue queue;
	ReverseQueue queue_reverse;

	// Add the obstacles.
	for (const Obstacle& obstacle : obstacles_)
	{
		if (obstacle.empty())
			continue;

		obst::Event::Ptr vertex;
		obst::EdgePtr edge_first = nullptr;

		Obstacle::const_iterator prev_iter = --obstacle.end();
		for (Obstacle::const_iterator vertex_iter = obstacle.begin(); vertex_iter != obstacle.end(); prev_iter = vertex_iter++)
		{
			obst::EdgePtr edge_prev = std::make_shared<obst::EdgeStraight>(*prev_iter, *vertex_iter);
			if (edge_first == nullptr)
				edge_first = edge_prev;

			if (vertex)
				vertex->edges[1] = edge_prev;

			vertex = std::make_shared<obst::Event>(*vertex_iter, std::make_shared<Node>(), edge_prev, nullptr);
			queue_reverse.push(vertex);
		}

		CHECK(vertex);
		vertex->edges[1] = edge_first;
	}

	// TODO(tvl) I probably also have to add the nodes to the queue so they can be marked.

	Node::Ptr root;
	for (const Node::Ptr& node : nodes_)
	{
		CHECK_NOTNULL(node->place);

		const PolarPoint relative_position(node->place->position, root_translation_);
		if (relative_position.R() == 0)
			root = node;
	}
	CHECK_NOTNULL(root);


	// Compute the reachable region by handling the events (in reverse order) and keeping track of the boundaries of the reachable intervals.
	obst::SweepCircleOld wavefront;

	// Add the root interval to the wavefront.
	wavefront.AddRootInterval(root);




	bool test0 = true;

	// DEBUG //
	while (!queue_reverse.empty())
	{
		obst::Event::Ptr event = queue_reverse.top();
		queue_reverse.pop();

		std::cerr << (*event) << std::endl << std::endl;


		std::vector<obst::Event::Ptr> add_events;
		wavefront.Handle(event, add_events);

		for (const obst::Event::Ptr& add_event : add_events)
		{
			// All added events must occur after the current one.
			if (event->relative_position.R() < add_event->relative_position.R())
				queue_reverse.push(add_event);
		}
	}


	bool test = true;




	// TODO(tvl) temp placeholder.
	//ComputeUnobstructed();
}
*/
void SpiralTree::computeObstructed() {
	CHECK(false) << "Not implemented yet.";
}

} // namespace cartocrow::flow_map
