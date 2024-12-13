#pragma once

#include "../../core/core.h"
#include "../../core/arrangement_map.h"
#include "../historic_arrangement.h"
#include "../modifiable_arrangement.h"
#include "../util.h"

namespace cartocrow::simplification {

using PolygonVector = std::vector<Polygon<Exact>>;

enum class ECEdgeMark { NONE, MAIN, OTHER };

struct Collapse {
	// either erase_both is true, or point is set
	bool erase_both;
	Point<Exact> point;
	PolygonVector this_face_polygons;
	PolygonVector twin_face_polygons;
};



/// Concept for functions necessary to allow for performing \ref
/// cartocrow::simplification::EdgeCollapseSimplification
/// "EdgeCollapseSimplification".
template <class T>
concept EdgeCollapseTraits = requires(typename T::Map::Halfedge_handle e, int b,
                                      Collapse collapse, ECEdgeMark m) {

	requires MapType<T>;

	/// We assume that an edge collapse is identical for a halfedge and its twin. 
	/// We therefore "mark" one of them as MAIN, its twin as OTHER.
	/// Default should go to NONE to indicate an unmarked edge.
	{T::ecSetEdgeMark(e, m)};
	{ T::ecGetEdgeMark(e) } -> std::same_as<ECEdgeMark>;

	/// Compute the point to collapse edge \f$e\f$ onto. along with the
	/// symmetric difference, and stores this with the edge.
	{ T::ecComputeCollapse(e) } -> std::same_as<Collapse>;
	/// Sets the stored collapse information.
	{T::ecSetCollapse(e, collapse)};
	/// Retrieve the computed collapse information.
	{ T::ecGetCollapse(e) } -> std::same_as<Collapse>;

	/// Set the cost of collapsing edge \f$e\f$. Note that, in principle, the cost may
	/// also be derived each time upon calling \ref ecGetCost(). In such a case,
	/// this method does not have to perform any actions. The cost should be
	/// nonnegative.
	{T::ecSetCost(e)};
	/// Retrieve (or compute) the cost of collapsing edge \f$e\f$. The cost should
	/// be nonnegative.
	{ T::ecGetCost(e) } -> std::same_as<Number<Exact>>;

	/// Stores an integer \f$b\f$ with edge \f$e\f$, representing the blocking
	/// number. This should not be modified in other ways that through calls of
	/// the \ref EdgeCollapseSimplification algorithm.
	{T::ecSetBlockingNumber(e, b)};
	/// Retrieves the blocking number stored with edge \f$e\f$.
	{ T::ecGetBlockingNumber(e) } -> std::same_as<int>;
};

/// This simplification algorithm removes vertices one at a time, by replacing an
/// edge with a single point (collapsing that edge). The edge is
/// removed that incurs the smallest cost, while ensuring that no
/// intersections are created, and that no topological changes are made.
///
/// Implementation notes:
///	- Runs in \f$O(n^2)\f$ time on a map with \f$n\f$ edges
///	- This is a topologically-safe variant, using the principle of blocking
///   numbers in https://doi.org/10.1145/2818373
/// - It abstracts from the way on how to compute (and store) the collapsing point,
///   the cost of collapsing an edge, and the blocking number, via the \ref
///   cartocrow::simplification::EdgeCollapseTraits "EdgeCollapseTraits" concept.
/// - Each iteration, all edges are polled for their cost: there is no specific
///   need to store these via the ecSetCost function
template <ModifiableArrangement MA, EdgeCollapseTraits ECT>
requires(std::same_as<typename MA::Map, typename ECT::Map>) class EdgeCollapseSimplification {

  public:
	using Map = MA::Map;

	/// Constructs the algorithm for a given \ref
	/// cartocrow::simplification::ModifiableArrangement "ModifiableArrangement".
	EdgeCollapseSimplification(MA& ma) : map(ma.getMap()), modmap(ma){};

	/// Initializes the algorithm, taking \f$O(n^2)\f$ time.
	void initialize();

	/// Repeatedly performs the cheapest operation, while operations cost at most
	/// threshold \f$t\f$. Each iteration takes linear time.
	/// Notes:
	/// - Subsequent calls with a lower value for \f$t\f$ have no effect.
	/// - In case of a historic arrangement, it is always set to present.
	void simplify(Number<Exact> t);
	/// Repeatedly performs the cheapest operation, until the result has at most
	/// \f$c\f$ edges. Each iteration takes linear time.
	/// Notes:
	/// - Subsequent calls with a higher value for \f$c\f$ have no effect.
	/// - In case of a historic arrangement, it is always set to present.
	void simplify(int c);

  private:
	/// Reinitializes the simplification data for the edge
	void initEdge(Map::Halfedge_handle e);
	/// Finds the edge to remove with the lowest cost. Returns true iff removable
	/// edge was found. NB: assigns best and best_cost
	bool findBest(Map::Halfedge_handle& best, Number<Exact>& best_cost);
	/// Executes the removal of the given edge
	void execute(Map::Halfedge_handle e, Number<Exact> cost);
	/// Adjusts blocking counters of all edges in face of e that are blocked by e
	void adjustCounts(Map::Halfedge_handle e, const int adj);

	/// Flags for indicating why an edge cannot be collapsed
	const short BLOCKED_FLOATING = -1;
	/// Flags for indicating why an edge cannot be collapsed
	const short NOOP_TRIANGLE = -2;
	/// Flags for indicating why an edge cannot be collapsed
	const short NOOP_DEGREE = -3;

	/// Stores the actual arrangement
	Map& map;
	/// Stores the modifiable arrangement
	MA& modmap;
};

} // namespace cartocrow::simplification

#include "edge_collapse.hpp"
