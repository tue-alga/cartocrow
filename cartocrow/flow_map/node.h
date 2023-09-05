#ifndef CARTOCROW_FLOW_MAP_NODE_H
#define CARTOCROW_FLOW_MAP_NODE_H

#include "place.h"
#include "polar_point.h"

namespace cartocrow::flow_map {

/// A node in a spiral or flow tree.
///
/// A node may be associated with a place on the map that either sends or
/// receives flow. These nodes are the root and leaf nodes. Other nodes will
/// have the same amount of incoming flow as the sum of the outgoing flows.
struct Node {

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

	/// Constructs a new node at the given position. If \c place is \c nullptr
	/// this represents an internal node, otherwise it represents a terminal.
	explicit Node(const PolarPoint& position, const std::shared_ptr<Place> place = nullptr);

	/// Determines the type of this node, based on its number of children.
	///
	/// Each node is either the root, a leaf, a join node, or a subdivision node
	/// (see \ref ConnectionType).
	ConnectionType getType() const;

	/// Determines whether this node is a Steiner node.
	///
	/// Steiner nodes are not part of the input places. They support the tree,
	/// either by splitting the flow, or by guiding the flow around obstacles.
	bool isSteiner() const;

	/// The place associated with this node, or \c nullptr if no place is
	/// associated with this node.
	std::shared_ptr<Place> m_place;

	/// The position of this node.
	PolarPoint m_position;

	/// The parent of this node, or \c nullptr if this is the root.
	std::shared_ptr<Node> m_parent;

	/// The children of this node.
	///
	/// While generally the nodes of a tree without children are refered to as
	/// leaf nodes, a node with the leaf type may have children if it is located
	/// inside the spiral region of another node.
	std::vector<std::shared_ptr<Node>> m_children;

	/// The amount of flow through this node.
	Number<Inexact> m_flow;

	/// The index of this node in its tree.
	int m_id = -1;
};

} // namespace cartocrow::flow_map

#endif
