#pragma once

#include "../core/core.h"
#include "common_arrangement.h"
#include "common_traits.h"

namespace cartocrow::simplification {

template <BaseSimplificationTraits BST> class HistoricArrangement {
  public:
	HistoricArrangement(BST::Map& inmap) : map(inmap), max_cost(0) {
		in_complexity = inmap.number_of_edges();
	};

	~HistoricArrangement() {
		for (EdgeData* op : history) {
			delete op;
		}

		for (EdgeData* op : undone) {
			delete op;
		}
	}

	struct EdgeData {

		EdgeData(BST::Map::Halfedge_handle he, EdgeData* p, EdgeData* n, EdgeData* pt, EdgeData* nt,
		         int pc, Number<Exact> pmc, Point<Exact> pl)
		    : halfedge(he), prev(p), next(n), prev_twin(pt), next_twin(nt), post_complexity(pc),
		      post_maxcost(pmc), pre_loc(pl) {}

		BST::Map::Halfedge_handle halfedge;

		EdgeData* prev = nullptr;
		EdgeData* next = nullptr;
		EdgeData* prev_twin = nullptr;
		EdgeData* next_twin = nullptr;

		int post_complexity;
		Number<Exact> post_maxcost;
		Point<Exact> pre_loc;
	};

	BST::Map::Halfedge_handle mergeWithNext(BST::Map::Halfedge_handle e, Number<Exact> cost) {
		EdgeData* prev = BST::histGetData(e);
		EdgeData* next = BST::histGetData(e->next());
		EdgeData* prev_twin = BST::histGetData(e->twin());
		EdgeData* next_twin = BST::histGetData(e->next()->twin());

		Point<Exact> pre_loc = e->target()->point();
		if (cost > max_cost) {
			max_cost = cost;
		}

		typename BST::Map::Halfedge_handle newe = merge_with_next(map, e);

		EdgeData* newd = new EdgeData(newe, prev, next, prev_twin, next_twin, map.number_of_edges(), max_cost, pre_loc);

		BST::histSetData(newe, newd);

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
		EdgeData* op = history.back();
		history.pop_back();
		undone.push_back(op);

		typename BST::Map::Halfedge_handle inc = split(map, op->halfedge, op->pre_loc);

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

		EdgeData* op = undone.back();
		undone.pop_back();
		history.push_back(op);

		op->halfedge = merge_with_next(map, op->halfedge);
	}

  private:
	Number<Exact> max_cost;
	BST::Map& map;
	int in_complexity;
	std::vector<EdgeData*> history;
	std::vector<EdgeData*> undone;
};

} // namespace cartocrow::simplification