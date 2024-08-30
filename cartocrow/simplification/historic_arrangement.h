#pragma once

#include "../core/core.h"
#include "modifiable_arrangement.h"
#include "util.h"

namespace cartocrow::simplification {

/// Concept to express that a pointer to a class \f$D\f$ can be stored via the
/// \ref cartocrow::simplification::MapType "MapType" \f$MT\f$, via the described
/// functions.
template <class MT, class D>
concept EdgeStoredHistory = requires(typename MT::Map::Halfedge_handle e, D* d) {
	requires MapType<MT>;

	/// Sets the data for a \ref HistoricArrangement
	{MT::histSetData(e, d)};
	/// Retrieves the data for a \ref HistoricArrangement
	{ MT::histGetData(e) } -> std::same_as<D*>;
};

/// The history of an edge, including pointers to other steps in the history that
/// it may have overriden. This struct is used by a \ref HistoricArrangement.
template <MapType MT> struct HalfedgeOperation {

	using Map = MT::Map;

	HalfedgeOperation(Map::Halfedge_handle he) : halfedge(he) {}

	/// Pointer to the current halfedge
	Map::Halfedge_handle halfedge;

	virtual void undo(Map& map) = 0;
	virtual void redo(Map& map) = 0;

  protected:
	// utility function to get and clear the history data stored with the given halfedge
	inline HalfedgeOperation<MT>* getAndClear(Map::Halfedge_handle e) {
		HalfedgeOperation<MT>* result = MT::histGetData(e);
		MT::histSetData(e, nullptr);
		return result;
	}
};

template <MapType MT> struct HalfedgeSplit : public HalfedgeOperation<MT> {

	using Map = MT::Map;

	HalfedgeSplit(Map::Halfedge_handle he, Point<Exact> pt) : HalfedgeOperation<MT>(he) {

		post_loc = pt;

		self = this->getAndClear(he);
	}

	// edge histories of the old edges
	HalfedgeOperation<MT>* self = nullptr;
	// edge histories of the new edges
	// probably overly many, but allowing more black-box type use of CGAL
	HalfedgeOperation<MT>* future_self = nullptr;
	HalfedgeOperation<MT>* future_twin = nullptr;
	HalfedgeOperation<MT>* future_next = nullptr;
	HalfedgeOperation<MT>* future_next_twin = nullptr;

	// the location of the to-be-introduced vertex
	Point<Exact> post_loc;

	virtual void undo(Map& map);
	virtual void redo(Map& map);
};

template <MapType MT> struct HalfedgeMerge : public HalfedgeOperation<MT> {

	using Map = MT::Map;

	HalfedgeMerge(Map::Halfedge_handle he) : HalfedgeOperation<MT>(he) {

		pre_loc = he->target()->point();

		self = this->getAndClear(he);
		next = this->getAndClear(he->next());
		self_twin = this->getAndClear(he->twin());
		next_twin = this->getAndClear(he->next()->twin());
	}

	// edge histories of the old edges
	// this is probably more than necessary, but this allows us to treat CGAL's
	// arrangement more like a black box
	HalfedgeOperation<MT>* self = nullptr;
	HalfedgeOperation<MT>* next = nullptr;
	HalfedgeOperation<MT>* self_twin = nullptr;
	HalfedgeOperation<MT>* next_twin = nullptr;

	// the location of the vertex that got merged away
	Point<Exact> pre_loc;

	virtual void undo(Map& map);
	virtual void redo(Map& map);
};

template <MapType MT> struct HalfedgeTargetShift : public HalfedgeOperation<MT> {

	using Map = MT::Map;

	HalfedgeTargetShift(Map::Halfedge_handle he, Point<Exact> post)
	    : HalfedgeOperation<MT>(he), post_loc(post) {

		pre_loc = he->target()->point();

		self = this->getAndClear(he);
	}

	// the location of the vertex
	Point<Exact> pre_loc;
	Point<Exact> post_loc;

	// edge histories of the old edges
	HalfedgeOperation<MT>* self = nullptr;

	virtual void undo(Map& map);
	virtual void redo(Map& map);
};

template <MapType MT> struct OperationBatch {

	using Map = MT::Map;

	~OperationBatch() {
		for (HalfedgeOperation<MT>* op : operations) {
			delete op;
		}
	}

	std::vector<HalfedgeOperation<MT>*> operations;

	/// Number of edges in the map after this operation
	int post_complexity;
	/// Maximum cost of operations up to this one
	Number<Exact> post_maxcost;

	void undo(Map& map);
	void redo(Map& map);
};

/// This historic arrangement keeps track of the operations performed, by storing
/// this in the edges of the map. It implements the \ref
/// cartocrow::simplification::ModifiableArrangementWithHistory
/// "ModifiableArrangementWithHistory" concept, requiring \ref
/// cartocrow::simplification::EdgeStoredHistory "EdgeStoredHistory" on the
/// maptype.
template <MapType MT>
requires(EdgeStoredHistory<MT, HalfedgeOperation<MT>>) class HistoricArrangement {
  public:
	using Map = MT::Map;

	HistoricArrangement(Map& inmap);

	~HistoricArrangement();

	Map& getMap();

	// Recovers the result as if the simplification algorithm was run once with
	/// complexity parameter \f$c\f$.
	void recallComplexity(int c);
	// Recovers the result as if the simplification algorithm was run once with
	/// threshold parameter \f$t\f$.
	void recallThreshold(Number<Exact> t);

	/// Tests whether any operations have been undone. Returns true iff no
	/// operations were undone.
	bool atPresent();

	/// Undo one operation, if one exists.
	void backInTime();

	/// Redo one operation, if one exists.
	void forwardInTime();

	// From ModifiableArrangementWithHistory
	Map::Halfedge_handle mergeWithNext(Map::Halfedge_handle e);
	Map::Halfedge_handle split(Map::Halfedge_handle e, Point<Exact> p);
	void shift(Map::Vertex_handle v, Point<Exact> p);
	void goToPresent();
	void startBatch(Number<Exact> cost);
	void endBatch();

  private:
	Number<Exact> max_cost;
	Map& map;
	int in_complexity;
	OperationBatch<MT>* building_batch = nullptr;
	std::vector<OperationBatch<MT>*> history;
	std::vector<OperationBatch<MT>*> undone;
};

} // namespace cartocrow::simplification

#include "historic_arrangement.hpp"