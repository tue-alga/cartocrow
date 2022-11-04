#pragma once

#include "../core/core.h"
#include "common_arrangement.h"
#include "common_traits.h"

namespace cartocrow::simplification {

template <class MT, class D>
concept EdgeStoredHistory = requires(typename MT::Map::Halfedge_handle e, D* d) {
	requires MapType<MT>;

	{MT::histSetData(e, d)};
	{ MT::histGetData(e) } -> std::same_as<D*>;
};

template <MapType MT> struct EdgeHistory {

	using Map = MT::Map;

	EdgeHistory(Map::Halfedge_handle he, EdgeHistory<MT>* p, EdgeHistory<MT>* n,
	            EdgeHistory<MT>* pt, EdgeHistory<MT>* nt, int pc, Number<Exact> pmc, Point<Exact> pl)
	    : halfedge(he), prev(p), next(n), prev_twin(pt), next_twin(nt), post_complexity(pc),
	      post_maxcost(pmc), pre_loc(pl) {}

	Map::Halfedge_handle halfedge;

	EdgeHistory<MT>* prev = nullptr;
	EdgeHistory<MT>* next = nullptr;
	EdgeHistory<MT>* prev_twin = nullptr;
	EdgeHistory<MT>* next_twin = nullptr;

	int post_complexity;
	Number<Exact> post_maxcost;
	Point<Exact> pre_loc;
};

// implements the ModifiableArrangementWithHistory concept, requiring EdgeStoredHistory on the maptype
template <MapType MT> requires(EdgeStoredHistory<MT, EdgeHistory<MT>>) class HistoricArrangement {
  public:
	using Map = MT::Map;

	HistoricArrangement(Map& inmap) : map(inmap), max_cost(0) {
		in_complexity = inmap.number_of_edges();
	};

	~HistoricArrangement() {
		for (EdgeHistory<MT>* op : history) {
			delete op;
		}

		for (EdgeHistory<MT>* op : undone) {
			delete op;
		}
	}

	Map& getMap() {
		return map;
	}

	Map::Halfedge_handle mergeWithNext(Map::Halfedge_handle e, Number<Exact> cost) {
		EdgeHistory<MT>* prev = MT::histGetData(e);
		EdgeHistory<MT>* next = MT::histGetData(e->next());
		EdgeHistory<MT>* prev_twin = MT::histGetData(e->twin());
		EdgeHistory<MT>* next_twin = MT::histGetData(e->next()->twin());

		Point<Exact> pre_loc = e->target()->point();
		if (cost > max_cost) {
			max_cost = cost;
		}

		typename Map::Halfedge_handle newe = merge_with_next(map, e);

		EdgeHistory<MT>* newd = new EdgeHistory<MT>(newe, prev, next, prev_twin, next_twin,
		                                            map.number_of_edges(), max_cost, pre_loc);

		MT::histSetData(newe, newd);

		history.push_back(newd);

		return newe;
	}

	void goToPresent() {
		while (!atPresent()) {
			forwardInTime();
		}
	}

	bool atPresent() {
		return undone.empty();
	}

	void recallComplexity(int c) {
		while ( // if history is a single element, check input complexity
		    (history.size() == 1 && in_complexity <= c)
		    // if history is more than a single element, check the previous operation
		    || (history.size() > 1 && history[history.size() - 2]->post_complexity <= c)) {
			backInTime();
		}
		while (!undone.empty() && map.number_of_edges() > c) {
			forwardInTime();
		}
	}

	void backInTime() {
		EdgeHistory<MT>* op = history.back();
		history.pop_back();
		undone.push_back(op);

		typename Map::Halfedge_handle inc = split(map, op->halfedge, op->pre_loc);

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

	void forwardInTime() {

		EdgeHistory<MT>* op = undone.back();
		undone.pop_back();
		history.push_back(op);

		op->halfedge = merge_with_next(map, op->halfedge);
	}

  private:
	Number<Exact> max_cost;
	Map& map;
	int in_complexity;
	std::vector<EdgeHistory<MT>*> history;
	std::vector<EdgeHistory<MT>*> undone;
};

} // namespace cartocrow::simplification