namespace cartocrow::simplification {

template <class FaceData>
void VWTraits<FaceData>::vrSetCost(Map::Vertex_handle v, Triangle<Exact> T) {
	v->data().cost = T.area();
}

template <class FaceData>
Number<Exact> VWTraits<FaceData>::vrGetCost(Map::Vertex_handle v) {
	return v->data().cost;
}

template <class FaceData>
int VWTraits<FaceData>::vrGetBlockingNumber(Map::Vertex_handle v) {
	return v->data().block;
}

template <class FaceData>
void VWTraits<FaceData>::vrSetBlockingNumber(Map::Vertex_handle v, int b) {
	v->data().block = b;
}

template <class FaceData>
VWTraits<FaceData>::Map::Halfedge_handle VWTraits<FaceData>::vrGetHalfedge(Map::Vertex_handle v) {
	return v->data().inc;
}

template <class FaceData>
void VWTraits<FaceData>::vrSetHalfedge(Map::Vertex_handle v, Map::Halfedge_handle inc) {
	v->data().inc = inc;
}

template <class FaceData>
void VWTraits<FaceData>::histSetData(Map::Halfedge_handle e, HalfedgeOperation<VWTraits>* data) {
	e->data().hist = data;
}

template <class FaceData>
HalfedgeOperation<VWTraits<FaceData>>* VWTraits<FaceData>::histGetData(Map::Halfedge_handle e) {
	return e->data().hist;
}

} // namespace cartocrow::simplification