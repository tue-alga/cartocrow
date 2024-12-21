namespace cartocrow::simplification {

template <MapType MT> void HalfedgeMerge<MT>::undo(Map& map) {

	typename MT::Map::Halfedge_handle inc = util::split(map, this->halfedge, pre_loc);

	if (self != nullptr)
		self->halfedge = inc;
	if (next != nullptr)
		next->halfedge = inc->next();
	if (self_twin != nullptr)
		self_twin->halfedge = inc->twin();
	if (next_twin != nullptr)
		next_twin->halfedge = inc->next()->twin();
	this->halfedge = inc;
}

template <MapType MT> void HalfedgeMerge<MT>::redo(Map& map) {

	this->halfedge = util::mergeWithNext(map, this->halfedge);
}


template <MapType MT> void HalfedgeSplit<MT>::undo(Map& map) {

	// TODO: need to finalize pointer juggling
	future_self = this->getAndClear(this->halfedge);
	future_twin = this->getAndClear(this->halfedge->twin());
	future_next = this->getAndClear(this->halfedge->next());
	future_next_twin = this->getAndClear(this->halfedge->next()->twin());

	typename MT::Map::Halfedge_handle inc = util::mergeWithNext(map, this->halfedge);

	if (self != nullptr)
		self->halfedge = inc;
	this->halfedge = inc;
}

template <MapType MT> void HalfedgeSplit<MT>::redo(Map& map) {

	// TODO: need to finalize pointer juggling
	this->halfedge = util::split(map, this->halfedge, this->post_loc);
}

template <MapType MT> void HalfedgeTargetShift<MT>::undo(Map& map) {

	util::shift(map, this->halfedge->target(), pre_loc);

	if (self != nullptr)
		self->halfedge = this->halfedge;
}

template <MapType MT> void HalfedgeTargetShift<MT>::redo(Map& map) {

	util::shift(map, this->halfedge->target(), post_loc);
}

template <MapType MT> void OperationBatch<MT>::undo(Map& map) {

	// NB: reverse
	for (auto it = operations.rbegin(); it != operations.rend(); it++) {

		HalfedgeOperation<MT>* op = *it;
		op->undo(map);
	}
}

template <MapType MT> void OperationBatch<MT>::redo(Map& map) {
	for (HalfedgeOperation<MT>* op : operations) {
		op->redo(map);
	}
}

template <MapType MT>
requires(EdgeStoredHistory<MT, HalfedgeOperation<MT>>)
    HistoricArrangement<MT>::HistoricArrangement(Map& inmap)
    : map(inmap), max_cost(0) {
	in_complexity = inmap.number_of_edges();
};

template <MapType MT>
requires(EdgeStoredHistory<MT, HalfedgeOperation<MT>>) HistoricArrangement<MT>::~HistoricArrangement() {
	for (OperationBatch<MT>* op : history) {
		delete op;
	}

	for (OperationBatch<MT>* op : undone) {
		delete op;
	}
}

template <MapType MT>
requires(EdgeStoredHistory<MT, HalfedgeOperation<MT>>)
    HistoricArrangement<MT>::Map& HistoricArrangement<MT>::getMap() {
	return map;
}

template <MapType MT>
requires(EdgeStoredHistory<MT, HalfedgeOperation<MT>>) HistoricArrangement<MT>::Map::Halfedge_handle
    HistoricArrangement<MT>::mergeWithNext(Map::Halfedge_handle e) {

	assert(building_batch != nullptr);

	HalfedgeOperation<MT>* op = new HalfedgeMerge<MT>(e);
	op->redo(map);
	MT::histSetData(op->halfedge, op);
	building_batch->operations.push_back(op);
	return op->halfedge;
}

template <MapType MT>
requires(EdgeStoredHistory<MT, HalfedgeOperation<MT>>) void HistoricArrangement<MT>::shift(
    Map::Vertex_handle v, Point<Exact> p) {

	assert(building_batch != nullptr);

	HalfedgeOperation<MT>* op = new HalfedgeTargetShift<MT>(v->inc(), p);
	op->redo(map);
	MT::histSetData(op->halfedge, op);
	building_batch->operations.push_back(op);
}

template <MapType MT>
requires(EdgeStoredHistory<MT, HalfedgeOperation<MT>>) void HistoricArrangement<MT>::goToPresent() {
	while (!atPresent()) {
		forwardInTime();
	}
}

template <MapType MT>
requires(EdgeStoredHistory<MT, HalfedgeOperation<MT>>) bool HistoricArrangement<MT>::atPresent() {
	return undone.empty();
}

template <MapType MT>
requires(EdgeStoredHistory<MT, HalfedgeOperation<MT>>) void HistoricArrangement<MT>::backInTime() {
	
	
	assert(building_batch == nullptr);

	OperationBatch<MT>* batch = history.back();
	history.pop_back();
	undone.push_back(batch);

	batch->undo(map);
}

template <MapType MT>
requires(EdgeStoredHistory<MT, HalfedgeOperation<MT>>) void HistoricArrangement<MT>::forwardInTime() {

	assert(building_batch == nullptr);

	OperationBatch<MT>* batch = undone.back();
	undone.pop_back();
	history.push_back(batch);

	batch->redo(map);
}

template <MapType MT>
requires(EdgeStoredHistory<MT, HalfedgeOperation<MT>>) void HistoricArrangement<MT>::recallComplexity(
    int c) {

	assert(building_batch == nullptr);

	while ( // if history is a single element, check input complexity
	    (history.size() >= 1 && in_complexity <= c)
	    // if history is more than a single element, check the previous operation
	    || (history.size() > 1 && history[history.size() - 2]->post_complexity <= c)) {
		backInTime();
	}
	while (!undone.empty() && map.number_of_edges() > c) {
		forwardInTime();
	}
}

template <MapType MT>
requires(EdgeStoredHistory<MT, HalfedgeOperation<MT>>) void HistoricArrangement<MT>::recallThreshold(
    Number<Exact> t) {
		
	assert(building_batch == nullptr);

	while ((history.size() >= 1 && history.last()->post_maxcost > t)) {
		backInTime();
	}
	while (!undone.empty() && undone.last()->post_maxcost <= t) {
		forwardInTime();
	}
}

template <MapType MT>
requires(EdgeStoredHistory<MT, HalfedgeOperation<MT>>) void HistoricArrangement<MT>::startBatch(
    Number<Exact> cost) {

	
	assert(building_batch == nullptr);
	assert(atPresent());

	building_batch = new OperationBatch<MT>();
	history.push_back(building_batch);

	if (max_cost < cost) {
		max_cost = cost;
	}

	building_batch->post_maxcost = max_cost;
}

template <MapType MT>
requires(EdgeStoredHistory<MT, HalfedgeOperation<MT>>) void HistoricArrangement<MT>::endBatch() {
	
	assert(building_batch != nullptr);

	building_batch->post_complexity = map.number_of_edges();
	building_batch = nullptr;
}

} // namespace cartocrow::simplification