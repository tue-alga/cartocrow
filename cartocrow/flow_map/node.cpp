#include "node.h"

namespace cartocrow::flow_map {

Node::Node(const PolarPoint& position, const std::shared_ptr<Place> place)
    : m_position(position), m_place(place) {}

Node::ConnectionType Node::getType() const {
	if (m_parent == nullptr) {
		return ConnectionType::kRoot;
	} else {
		switch (m_children.size()) {
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
	return m_place == nullptr || (m_place->m_flow <= 0 && m_parent != nullptr);
}

} // namespace cartocrow::flow_map
