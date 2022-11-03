#include "visvalingam_whyatt.h"

namespace cartocrow::simplification {

void VWTraits::vrSetCost(Map::Vertex_handle v, Triangle<Exact> T) {
	v->data().cost = T.area();
}

Number<Exact> VWTraits::vrGetCost(Map::Vertex_handle v) {
	return v->data().cost;
}

int VWTraits::vrGetBlockingNumber(Map::Vertex_handle v) {
	return v->data().block;
}

void VWTraits::vrSetBlockingNumber(Map::Vertex_handle v, int b) {
	v->data().block = b;
}

VWTraits::Map::Halfedge_handle VWTraits::vrGetHalfedge(Map::Vertex_handle v) {
	return v->data().inc;
}

void VWTraits::vrSetHalfedge(Map::Vertex_handle v, Map::Halfedge_handle inc) {
	v->data().inc = inc;
}

void VWTraits::histSetData(Map::Halfedge_handle e, HistoricArrangement<VWTraits>::EdgeData* data) {
	e->data().hist = data;
}

HistoricArrangement<VWTraits>::EdgeData* VWTraits::histGetData(Map::Halfedge_handle e) {
	return e->data().hist;
}

} // namespace cartocrow::simplification