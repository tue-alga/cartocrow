/*
The CartoCrow library implements algorithmic geo-visualization methods,
developed at TU Eindhoven.
Copyright (C) 2024 TU Eindhoven

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

#ifndef CARTOCROW_ISOLINE_SIMPLIFICATION_H
#define CARTOCROW_ISOLINE_SIMPLIFICATION_H
#include "collapse.h"
#include "isoline.h"
#include "types.h"
#include "voronoi_helpers.h"
#include <boost/heap/d_ary_heap.hpp>

namespace cartocrow::isoline_simplification {
struct slope_ladder_comp {
	inline bool operator()(const std::shared_ptr<SlopeLadder>& sl1, const std::shared_ptr<SlopeLadder>& sl2) const {
		return sl1->m_cost > sl2->m_cost;
	}
};

typedef std::optional<std::variant<Segment<K>, std::monostate>> IntersectionResult;

typedef boost::heap::d_ary_heap<std::shared_ptr<SlopeLadder>, boost::heap::arity<2>, boost::heap::compare<slope_ladder_comp>, boost::heap::mutable_<true>> Heap;
typedef std::unordered_map<std::shared_ptr<SlopeLadder>, Heap::handle_type> LadderToHandle;

/// An algorithm that simplifies isolines simultaneously such that common features are maintained.
///
/// \image html harmonious-simplification.svg
///
/// The majority of simplification methods treat isolines independently; at best they avoid collisions between
/// adjacent simplified isolines. This algorithm \cite scalable_harmonious_simplification_cosit instead extends the
/// techniques of Van Goethem et al. \cite harmonious_simplification_giscience to simplify parts of isolines together.
///
/// ## Algorithm description
/// The algorithm consists of a preprocessing stage, and an iterative simplification stage.
/// In the preprocessing step bundles of edges called _slope ladders_ are identified that will be simplified together.
/// Then iteratively the slope ladder of lowest cost is _collapsed_ (each of its edges is replaced with a vertex).
/// The algorithm employs a segment Voronoi diagram to efficiently compute slope ladders, update them after a collapse,
/// and to check for topology violations (simplified isolines intersecting, or small simplified isolines being sweeped
/// over). This segment Voronoi diagram is stored in its dual representation as a Segment_Delaunay_graph_2 from CGAL
/// (\ref SDG2).
/// See the paper \cite scalable_harmonious_simplification_cosit for a more detailed description of the algorithm.
///
/// Many member functions and attributes of this class are public for drawing and debugging purposes.
/// However, one needs only \ref IsolineSimplifier() to construct a simplifier object and \ref simplify to simplify to a
/// target number of vertices.
///
/// ## Example
///
/// The intended way to use the algorithm to simplify a set of isolines down to e.g. 100 vertices is:
/// ```
/// IsolineSimplifier simplifier(isolines);
/// simplifier.simplify(100);
/// simplifier.m_simplified_isolines; // these are the simplified isolines
/// ```
///
/// The output isolines can be viewed or exported by creating a \ref
/// SimpleIsolinePainting "Painting" and passing it to the desired \ref
/// renderer::GeometryRenderer "GeometryRenderer":
/// ```
/// SimpleIsolinePainting painting(simplifier.m_simplified_isolines)
/// GeometryWidget widget(painting);
/// widget.show();
/// // or
/// IpeRenderer renderer(painting);
/// renderer.save("some_filename.ipe");
/// ```
///
/// Isolines can be read from an ipe file (\ref ipeToIsolines),
/// or constructed manually from a vector of points (\ref Isoline).
class IsolineSimplifier {
  public:
	/// Construct a simplifier to simplify the isolines with the specified collapse method.
	/// The angle filter and alignment filter parameters serve to filter out matchings.
	/// These angles should be specified in radians. By default the values are set to values greater than
	/// 2 pi such that no filtering occurs, as it is generally detrimental to the quality of the simplifications.
	/// The parameters can be set to negative values. Then all slope ladders consists of only one edge and the
	/// simplifier will perform only single edge collapses.
	IsolineSimplifier(std::vector<Isoline<K>> isolines = std::vector<Isoline<K>>(),
	                  std::shared_ptr<LadderCollapse> collapse = std::make_shared<LineSplineHybridCollapse>(SplineCollapse(3, 15), HarmonyLineCollapse(15)),
	                  double angle_filter = 100.0, double alignment_filter = 100.0);
	/// Collapses slope ladders until the number of vertices is at or below the specified target or no slope ladder
	/// exists that preserves topology.
	bool simplify(int target, bool debug = false);
	/// A convenience function that simplifies isolines using the CGAL implementation of the method of Dyken et al.
	/// This does perform the redundant preprocessing step of computing slope ladders so it is not the most efficient.
	bool dyken_simplify(int target, double sep_dist = 1);
	// Perform one simplification step; returns whether there was progress.
	bool step();
	/// Gets the next ladder that will be simplified (only for debugging purposes).
	std::optional<std::shared_ptr<SlopeLadder>> get_next_ladder();

	void update_ladders();
	void update_matching();

	/// The input isolines.
	std::vector<Isoline<K>> m_isolines;
	/// The simplified isolines.
	std::vector<Isoline<K>> m_simplified_isolines;
	/// Maps point to the isoline it is part of.
	PointToIsoline m_p_isoline;
	/// Maps point to the previous point on the isoline.
	PointToPoint m_p_prev;
	/// Maps point to the next point on the isoline.
	PointToPoint m_p_next;
	/// Maps point to the iterator of the isoline it is a part of for efficient removal.
	PointToIterator m_p_iterator;
	/// Maps point to the ladders it is a cap of.
	PointToSlopeLadders m_p_ladder;
	/// Maps an edge to the slope ladders it is a part of.
	EdgeToSlopeLadders m_e_ladder;
	/// Maps a point to the corresponding Delaunay vertex.
	PointToVertex m_p_vertex;
	/// Maps an edge to the corresponding segment Delaunay vertex.
	EdgeToVertex m_e_vertex;
	/// Maps an edge to slope ladders it has been detected to intersect.
	EdgeToSlopeLadders m_e_intersects;
	/// The segment Delaunay graph
	SDG2 m_delaunay;
	/// The medial axis separator. Note that this is not updated after initialization.
	Separator m_separator;
	/// The matching from which slope ladders are derived.
	Matching m_matching;
	/// The slope ladders in a min heap keyed on their simplification cost.
	Heap m_slope_ladders;
	LadderToHandle m_ladder_heap_handle;
	std::unordered_set<SDG2::Vertex_handle> m_changed_vertices;
	std::vector<Point<K>> m_deleted_points;
	/// The number of vertices present in the current isoline simplification.
	int m_current_complexity = 0;
	bool m_started = false;
	double m_angle_filter;
	double m_alignment_filter;
	/// The method used to collapse slope ladders.
	std::shared_ptr<LadderCollapse> m_collapse_ladder;
	bool check_ladder_intersections_naive(const SlopeLadder& ladder) const;
	IntersectionResult check_ladder_intersections_Voronoi(const SlopeLadder& ladder);
	std::unordered_set<SDG2::Vertex_handle> intersected_region(Segment<K> rung, Point<K> p);
	std::pair<std::vector<std::vector<SDG2::Edge>>, int>
	boundaries(const std::unordered_set<SDG2::Vertex_handle>& region) const;
	bool check_rung_collapse_topology(Segment<K> rung, Point<K> p, std::unordered_set<Point<K>>& allowed);
	bool check_ladder_collapse_topology(const SlopeLadder& ladder);
	double total_symmetric_difference() const;
	std::pair<double, double> average_max_vertex_alignment() const;
	int ladder_count();
	void clear();

  private:
	void initialize_point_data();
	void initialize_sdg();
	void initialize_slope_ladders();
	void collapse_ladder(SlopeLadder& ladder);
	void create_slope_ladder(Segment<K> seg);
	void clean_isolines();
	void remove_ladder_e(Segment<K> seg);
	std::optional<std::shared_ptr<SlopeLadder>> next_ladder();
};
}

#endif //CARTOCROW_ISOLINE_SIMPLIFICATION_H
