namespace cartocrow::simplification {

template <MapType MT>
requires(EdgeStoredHistory<MT, EdgeHistory<MT>>) HistoricArrangement<MT>::HistoricArrangement(Map& inmap)
    : map(inmap), max_cost(0) {
	in_complexity = inmap.number_of_edges();
};

template <MapType MT>
requires(EdgeStoredHistory<MT, EdgeHistory<MT>>) HistoricArrangement<MT>::~HistoricArrangement() {
	for (EdgeHistory<MT>* op : history) {
		delete op;
	}

	for (EdgeHistory<MT>* op : undone) {
		delete op;
	}
}

template <MapType MT>
requires(EdgeStoredHistory<MT, EdgeHistory<MT>>)
    HistoricArrangement<MT>::Map& HistoricArrangement<MT>::getMap() {
	return map;
}

template <MapType MT>
requires(EdgeStoredHistory<MT, EdgeHistory<MT>>) HistoricArrangement<MT>::Map::Halfedge_handle
    HistoricArrangement<MT>::mergeWithNext(Map::Halfedge_handle e, Number<Exact> cost) {
	EdgeHistory<MT>* prev = MT::histGetData(e);
	EdgeHistory<MT>* next = MT::histGetData(e->next());
	EdgeHistory<MT>* prev_twin = MT::histGetData(e->twin());
	EdgeHistory<MT>* next_twin = MT::histGetData(e->next()->twin());

	Point<Exact> pre_loc = e->target()->point();
	if (cost > max_cost) {
		max_cost = cost;
	}

	typename Map::Halfedge_handle newe = util::mergeWithNext(map, e);

	EdgeHistory<MT>* newd = new EdgeHistory<MT>(newe, prev, next, prev_twin, next_twin,
	                                            map.number_of_edges(), max_cost, pre_loc);

	MT::histSetData(newe, newd);

	history.push_back(newd);

	return newe;
}

template <MapType MT>
requires(EdgeStoredHistory<MT, EdgeHistory<MT>>) void HistoricArrangement<MT>::goToPresent() {
	while (!atPresent()) {
		forwardInTime();
	}
}

template <MapType MT>
requires(EdgeStoredHistory<MT, EdgeHistory<MT>>) bool HistoricArrangement<MT>::atPresent() {
	return undone.empty();
}

template <MapType MT>
requires(EdgeStoredHistory<MT, EdgeHistory<MT>>) void HistoricArrangement<MT>::backInTime() {
	EdgeHistory<MT>* op = history.back();
	history.pop_back();
	undone.push_back(op);

	typename Map::Halfedge_handle inc = util::split(map, op->halfedge, op->pre_loc);

	if (op->prev != nullptr)
		op->prev->halfedge = inc;
	if (op->next != nullptr)
		op->next->halfedge = inc->next();
	if (op->prev_twin != nullptr)
		op->prev_twin->halfedge = inc->twin();
	if (op->next_twin != nullptr)
		op->next_twin->halfedge = inc->next()->twin();
	op->halfedge = inc;
}

template <MapType MT>
requires(EdgeStoredHistory<MT, EdgeHistory<MT>>) void HistoricArrangement<MT>::forwardInTime() {

	EdgeHistory<MT>* op = undone.back();
	undone.pop_back();
	history.push_back(op);

	op->halfedge = util::mergeWithNext(map, op->halfedge);
}

template <MapType MT>
requires(EdgeStoredHistory<MT, EdgeHistory<MT>>) void HistoricArrangement<MT>::recallComplexity(int c) {
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
requires(EdgeStoredHistory<MT, EdgeHistory<MT>>) void HistoricArrangement<MT>::recallThreshold(
    Number<Exact> t) {
	while ((history.size() >= 1 && history.last()->post_maxcost > t)) {
		backInTime();
	}
	while (!undone.empty() && undone.last()->post_maxcost <= t) {
		forwardInTime();
	}
}

} // namespace cartocrow::simplification