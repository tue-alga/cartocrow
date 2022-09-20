/*
The CartoCrow library implements algorithmic geo-visualization methods,
developed at TU Eindhoven.
Copyright (C) 2021  Netherlands eScience Center and TU Eindhoven

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef CARTOCROW_FLOW_MAP_SWEEP_CIRCLE_H
#define CARTOCROW_FLOW_MAP_SWEEP_CIRCLE_H

#include <map>
#include <ostream>

#include "../core/core.h"
#include "polar_point.h"
#include "sweep_edge.h"
#include "sweep_interval.h"

namespace cartocrow::flow_map {

/// Representation of the sweep circle used in the \ref
/// SpiralTreeObstructedAlgorithm.
///
/// The sweep circle stores an ordered set of the \ref SweepEdge "SweepEdges" it
/// intersects. These are ordered in counter-clockwise order along the circle,
/// starting at \f$\phi = 0\f$, ending at (and excluding) \f$\phi = 2\pi\f$. The
/// intervals between the edges are also stored: each SweepEdge stores the \ref
/// SweepInterval that comes next to the SweepEdge. The first interval on the
/// circle is stored separately, by the SweepCircle. SweepEdges also have a
/// pointer to the previous SweepInterval, and SweepIntervals have pointers to
/// the next and previous SweepEdges. This way, the edges and intervals can be
/// traversed easily.
///
/// Initializing the sweep circle results in a circle of radius 0 with no edges.
/// At any time, \ref r() returns the current radius of the circle. The radius
/// can be increased with \ref grow(). However, at any time, the circle needs to
/// be kept _valid_: the intersected edges need to be kept in order. (This can
/// be double-checked using \ref isValid().) Hence, when growing the circle,
/// even though the \f$\phi\f$ values of edges change, they cannot swap places.
/// In other words, it is not allowed to grow the circle over an intersection.
/// Instead, grow it to exactly touch the intersection, then handle the
/// intersection (remove the intersecting edges and reinsert them in the right
/// order), and then continue growing the circle.
///
/// The structural changes needed for handling such events are implemented in
/// this class as methods: \ref splitFromInterval(), \ref splitFromEdge(), \ref
/// switchEdge(), \ref mergeToEdge(), and \ref mergeToInterval().
class SweepCircle {
  public:
	/// Creates a sweep circle of radius 0, consisting of a single reachable
	/// interval.
	SweepCircle();

	/// Returns the current radius of the sweep circle.
	Number<Inexact> r();

	/// Sets the radius to the given value. This does not update anything
	/// structurally; in other words, it assumes that the circle does not pass
	/// over vertices or intersections. If it does, the sweep circle may become
	/// invalid (see \ref isValid()).
	void grow(Number<Inexact> r);

	/// Checks if this sweep circle is still valid, that is, if the edges and
	/// intervals in this sweep circle are still ordered in counter-clockwise
	/// order around the origin, and if all the pointers between neighboring
	/// edges and intervals are still set correctly.
	bool isValid() const;

	/// Prints a summary of the edges and intervals on the sweep circle. This is
	/// meant for debug purposes.
	void print() const;

	/// Returns the number of intervals on the sweep circle.
	std::size_t intervalCount() const;

	/// Returns the interval on the circle at the given angle \f$\phi\f$. If
	/// \f$\phi\f$ exactly lies on an edge (see \ref edgeAt()), the right
	/// interval of that edge is returned.
	/// \note The pointer returned by this function is invalidated as soon as
	/// any structural changes to the sweep circle occur.
	SweepInterval* intervalAt(Number<Inexact> phi);

	/// Comparator that compares sweep edges by their phi value at the current
	/// radius of the sweep circle.
	struct SweepEdgeShapeComparator {
		SweepEdgeShapeComparator(SweepCircle* owner) : m_owner(owner) {}
		SweepCircle* m_owner;
		bool operator()(SweepEdgeShape e1, SweepEdgeShape e2) const {
			return e1.phiForR(m_owner->m_r) < e2.phiForR(m_owner->m_r);
		}
		bool operator()(Number<Inexact> phi1, SweepEdgeShape e2) const {
			return phi1 < e2.phiForR(m_owner->m_r);
		}
		bool operator()(SweepEdgeShape e1, Number<Inexact> phi2) const {
			return e1.phiForR(m_owner->m_r) < phi2;
		}
		struct is_transparent {};
	};
	using EdgeMap =
	    std::multimap<SweepEdgeShape, std::shared_ptr<SweepEdge>, SweepEdgeShapeComparator>;

	/// Returns an iterator pointing at the first edge on the sweep circle.
	EdgeMap::iterator begin();

	/// Returns a pair of iterators representing the range of edges at the given
	/// angle \f$\phi\f$, like \ref std::multimap::equal_range().
	std::pair<EdgeMap::iterator, EdgeMap::iterator> edgesAt(Number<Inexact> phi);

	/// Returns a past-the-end iterator, pointing past the last edge on the
	/// sweep circle.
	EdgeMap::iterator end();

	/// The elements (intervals and edges) resulting from a three-way split
	/// operation, in order in increasing angle over the circle.
	struct ThreeWaySplitResult {
		SweepInterval* rightInterval;
		SweepInterval* middleRightInterval;
		SweepInterval* middleLeftInterval;
		SweepInterval* leftInterval;
	};

	/// Splits the given interval \f$i\f$ into three, with two new intervals in
	/// between. Assumes that the newly inserted edges have their near endpoints
	/// at the same point on this sweep circle.
	ThreeWaySplitResult splitFromInterval(std::shared_ptr<SweepEdge> newRightEdge,
	                                      std::shared_ptr<SweepEdge> newMiddleEdge,
	                                      std::shared_ptr<SweepEdge> newLeftEdge);

	/// The elements (intervals and edges) resulting from a split operation, in
	/// order in increasing angle over the circle.
	struct SplitResult {
		SweepInterval* rightInterval;
		SweepInterval* middleInterval;
		SweepInterval* leftInterval;
	};

	/// Splits the given edge \f$e\f$ into two, with a new interval in between.
	/// Assumes that the far endpoint of \f$e\f$ is currently on this sweep
	/// circle, and the newly inserted edges have their near endpoints at the
	/// same point on this sweep circle.
	SplitResult splitFromEdge(SweepEdge& oldEdge, std::shared_ptr<SweepEdge> newRightEdge,
	                          std::shared_ptr<SweepEdge> newLeftEdge);
	/// Splits the given interval \f$i\f$ into two, with a new interval in
	/// between. Assumes that the newly inserted edges have their near endpoints
	/// at the same point on this sweep circle.
	SplitResult splitFromInterval(std::shared_ptr<SweepEdge> newRightEdge,
	                              std::shared_ptr<SweepEdge> newLeftEdge);

	/// The elements (intervals and edges) resulting from a switch operation, in
	/// order in increasing angle over the circle.
	struct SwitchResult {
		SweepInterval* rightInterval;
		SweepInterval* leftInterval;
	};

	/// Replaces one edge \f$e\f$ by another. Assumes that the far endpoint of
	/// \f$e\f$ is currently on this sweep circle and coincides with the near
	/// endpoint of the new edge.
	SwitchResult switchEdge(SweepEdge& e, std::shared_ptr<SweepEdge> newEdge);

	/// The elements (intervals and edges) resulting from a merge operation, in
	/// order in increasing angle over the circle.
	struct MergeResult {
		SweepInterval* mergedInterval;
	};

	/// Removes two edges and replaces them by a single new edge. Assumes that
	/// the far endpoints of both edges coincides and lies currently on this
	/// sweep circle.
	SwitchResult mergeToEdge(SweepEdge& rightEdge, SweepEdge& leftEdge,
	                         std::shared_ptr<SweepEdge> newEdge);
	/// Removes two edges and replaces them by a new interval. Assumes that the
	/// far endpoints of both edges coincides and lies currently on this sweep
	/// circle.
	MergeResult mergeToInterval(SweepEdge& rightEdge, SweepEdge& leftEdge);

	/// A map containing the sweep edges separating the intervals, indexed by
	/// their edge shapes.
	EdgeMap m_edges{SweepEdgeShapeComparator(this)};
	/// The first interval on the circle.
	SweepInterval m_firstInterval;
	/// Current radius of the circle.
	Number<Inexact> m_r;

  public:
	/// Returns a map containing the sweep edges separating the intervals,
	/// indexed by their edge shapes.
	const EdgeMap& edges() const;
	/// Returns the first interval on the circle.
	const SweepInterval& firstInterval() const;
};

} // namespace cartocrow::flow_map

#endif //CARTOCROW_FLOW_MAP_SWEEP_CIRCLE_H
