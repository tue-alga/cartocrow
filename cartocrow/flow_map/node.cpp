#include "node.h"

#include <glog/logging.h>

namespace cartocrow::flow_map {

Node::Node(std::shared_ptr<Place> place) : m_place(place) {}

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
	return m_place == nullptr || (m_place->flow_in <= 0 && m_parent != nullptr);
}

} // namespace cartocrow::flow_map
