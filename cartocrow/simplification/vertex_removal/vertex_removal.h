#pragma once

#include "../../core/core.h"
#include "../../core/region_arrangement.h"
#include "../common_arrangement.h"
#include "../common_traits.h"
#include "../historic_arrangement.h"

/// Algorithm to iteratively remove vertices from a map.
///
/// Implementation notes
///	  Runs in O(n^2) time
///	  This is a topologically-safe variant, using the principle of blocking numbers in https://doi.org/10.1145/2818373
///   It abstracts from the way on how to compute (and store) the cost of removing a vertex, its relevant edge, and the blocking number, via the \ref VertexRemovalTraits concept.
///   Each iteration, all vertices are polled for their cost: there is no specific need to store these via the vrSetCost function

namespace cartocrow::simplification {

/// Concept for functions necessary on a Map type to allow for performing \ref VertexRemovalSimplification
template <class T>
concept VertexRemovalTraits = requires(typename T::Map::Vertex_handle v, int b, Triangle<Exact> t,
                                       typename T::Map::Halfedge_handle e) {

	requires MapType<T>;

	// can get and set the cost
	// NB: this does not have to be stored necessarily, can also be derived each time GetCost is called
	{T::vrSetCost(v, t)};
	{ T::vrGetCost(v) } -> std::same_as<Number<Exact>>;

	// can store and retrieve an integer, this should be stored
	{T::vrSetBlockingNumber(v, b)};
	{ T::vrGetBlockingNumber(v) } -> std::same_as<int>;

	// can get and set incoming halfedge on the convex side
	// NB: this does not have to be stored necessarily, can also be derived each time GetHalfedge is called
	{T::vrSetHalfedge(v, e)};
	{ T::vrGetHalfedge(v) } -> std::same_as<typename T::Map::Halfedge_handle>;
};

/// Basic template for a simplification algorithm removing a vertex one at a time, while not causing topology violations.
template <ModifiableArrangement MA, VertexRemovalTraits VRT>
requires(std::same_as<typename MA::Map, typename VRT::Map>) class VertexRemovalSimplification {

  public:
	using Map = MA::Map;

	/// Constructs the algorithm for a given \ref ModifiableArrangement
	VertexRemovalSimplification(MA& ma) : map(ma.getMap()), modmap(ma){};

	/// Initializes the algorithm
	void initialize();

	/// Simplifies while operations cost at most threshold t
	void simplify(Number<Exact> t);
	/// Simplifies until the number of edges in the arrangement is at most c
	void simplify(int c);

  private:
	// Reinitializes the simplification data for the vertex
	void initVertex(Map::Vertex_handle v);
	// Finds the vertex to remove with the lowest cost. Returns true iff removable vertex was found
	// NB: assigns best and best_cost
	bool findBest(Map::Vertex_handle& best, Number<Exact>& best_cost);
	// Executes the removal of the given vertex
	void execute(Map::Vertex_handle v, Number<Exact> cost);
	// Removes v from all blocking counters
	void reduceCounts(Map::Vertex_handle v);

	// Computes the triangle spanned by v and its two neighbors
	Triangle<Exact> triangle(Map::Vertex_handle v);

	// Flags for indicating why a vertex cannot be removed
	const short BLOCKED_FLOATING = -1;
	const short NOOP_TRIANGLE = -2;
	const short NOOP_DEGREE = -3;

	Map& map;
	MA& modmap;
};

} // namespace cartocrow::simplification

#include "vertex_removal.hpp"
