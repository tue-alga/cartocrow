#pragma once

#include "../../core/core.h"
#include "../../core/region_arrangement.h"
#include "../util.h"
#include "../modifiable_arrangement.h"
#include "../historic_arrangement.h"

namespace cartocrow::simplification {

/// Concept for functions necessary to allow for performing \ref 
/// cartocrow::simplification::VertexRemovalSimplification 
/// "VertexRemovalSimplification".
template <class T>
concept VertexRemovalTraits = requires(typename T::Map::Vertex_handle v, int b, Triangle<Exact> t,
                                       typename T::Map::Halfedge_handle e) {

	requires MapType<T>;

	/// Set the cost of removing vertex \f$v\f$ spanning a triangle \f$t\f$ with 
	/// its neighbors. Note that, in principle, the cost may also be derived 
	/// each time upon calling \ref vrGetCost(). In such a case, this method does 
	/// not have to perform any actions. The cost should be nonnegative.
	{T::vrSetCost(v, t)};
	/// Retrieve (or compute) the cost of removing vertex \f$v\f$. The cost should
	/// be nonnegative.
	{ T::vrGetCost(v) } -> std::same_as<Number<Exact>>;

	/// Stores an integer \f$b\f$ with vertex \f$v\f$, representing the blocking 
	/// number. This should not be modified in other ways than through calls of 
	/// the \ref VertexRemovalSimplification algorithm.
	{T::vrSetBlockingNumber(v, b)};
	/// Retrieves the blocking number stored with vertex \f$v\f$.
	{ T::vrGetBlockingNumber(v) } -> std::same_as<int>;

	/// Sets the incoming halfedge \f$e\f$ on the convex side of vertex \f$v\f$.
	/// Note that, in principle, this edge may also be derived each time upon 
	/// calling \ref vrGetHalfEdge(). In such a case, this method does not have to
	/// perform any actions.
	{T::vrSetHalfedge(v, e)};
	/// Retrieve (or compute) the incoming halfedge on the convex side of \f$v\f$.
	{ T::vrGetHalfedge(v) } -> std::same_as<typename T::Map::Halfedge_handle>;
};

/// This simplification algorithm removes degree-2 vertices one at a time,
/// replacing it by a single edge connecting its neighbors. The vertex is
/// removed that incurs the smallest cost, while ensuring that no
/// intersections are created by the new edge, and that no topological changes are
/// made.
///
/// Implementation notes:
///	- Runs in \f$O(n^2)\f$ time on a map with \f$n\f$ edges
///	- This is a topologically-safe variant, using the principle of blocking 
///   numbers in https://doi.org/10.1145/2818373
/// - It abstracts from the way on how to compute (and store) the cost of removing
///   a vertex, its relevant edge, and the blocking number, via the \ref 
///   cartocrow::simplification::VertexRemovalTraits "VertexRemovalTraits" concept.
/// - Each iteration, all vertices are polled for their cost: there is no specific
///   need to store these via the vrSetCost function
template <ModifiableArrangement MA, VertexRemovalTraits VRT>
requires(std::same_as<typename MA::Map, typename VRT::Map>) class VertexRemovalSimplification {

  public:
	using Map = MA::Map;

	/// Constructs the algorithm for a given \ref 
	/// cartocrow::simplification::ModifiableArrangement "ModifiableArrangement".
	VertexRemovalSimplification(MA& ma) : map(ma.getMap()), modmap(ma){};

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
	/// Reinitializes the simplification data for the vertex
	void initVertex(Map::Vertex_handle v);
	/// Finds the vertex to remove with the lowest cost. Returns true iff removable
	/// vertex was found. NB: assigns best and best_cost
	bool findBest(Map::Vertex_handle& best, Number<Exact>& best_cost);
	/// Executes the removal of the given vertex
	void execute(Map::Vertex_handle v, Number<Exact> cost);
	/// Removes v from all blocking counters
	void reduceCounts(Map::Vertex_handle v);

	/// Computes the triangle spanned by v and its two neighbors
	Triangle<Exact> triangle(Map::Vertex_handle v);

	/// Flags for indicating why a vertex cannot be removed
	const short BLOCKED_FLOATING = -1;
	/// Flags for indicating why a vertex cannot be removed
	const short NOOP_TRIANGLE = -2;
	/// Flags for indicating why a vertex cannot be removed
	const short NOOP_DEGREE = -3;

	/// Stores the actual arrangement
	Map& map;
	/// Stores the modifiable arrangement
	MA& modmap;
};

} // namespace cartocrow::simplification

#include "vertex_removal.hpp"
