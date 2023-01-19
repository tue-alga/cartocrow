/*
The Necklace Map library implements the algorithmic geo-visualization
method by the same name, developed by Bettina Speckmann and Kevin Verbeek
at TU Eindhoven (DOI: 10.1109/TVCG.2010.180 & 10.1142/S021819591550003X).
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 10-09-2019
*/

#ifndef CARTOCROW_NECKLACE_MAP_NECKLACE_MAP_H
#define CARTOCROW_NECKLACE_MAP_NECKLACE_MAP_H

#include <vector>

#include "../core/core.h"
#include "../core/region_map.h"
#include "bead.h"
#include "feasible_interval/compute_feasible_interval.h"
#include "necklace.h"
#include "necklace_shape.h"
#include "parameters.h"
#include "scale_factor/compute_scale_factor.h"
#include "valid_placement/compute_valid_placement.h"

namespace cartocrow::necklace_map {

/// Representation of a necklace map for a set of regions and a set of
/// necklaces.
///
/// A *necklace map* is a type of proportional symbol map in which the symbols
/// displaying the data values for each region (here called *beads*) are moved
/// away from the regions themselves and instead placed on one or more curves
/// surrounding the map called *necklaces*:
///
/// \image html necklace-map-example.png
///
/// The intent of this drawing style is to reduce the clutter caused by symbols
/// covering up their regions. Necklace maps were introduced by Speckmann and
/// Verbeek \cite necklace_maps_tvcg. This class implements the algorithm for
/// the construction of necklace maps described in the companion paper
/// \cite necklace_maps_ijcga.
///
/// ## Algorithm description
///
/// A necklace map consists of one or more regions and one or more necklaces,
/// where each region is mapped to a necklace. Necklaces can be circles (\ref
/// CircleNecklace) or Bézier splines (\ref BezierNecklace). To compute the
/// map, each region is assigned a _feasible region_ on its necklace (see \ref
/// IntervalType). The beads are then placed inside their feasible regions using
/// an attraction/repulsion force model. All beads can be uniformly scaled; the
/// algorithm returns the optimal scale factor for the beads (\ref
/// scaleFactor()), such that they can all be placed on their necklace without
/// any overlap.
///
/// ## Example
///
/// The intended way to compute a necklace map is like this:
/// ```
/// necklace_map::NecklaceMap map(regions);
///
/// // add necklaces
/// auto n1 = map.addNecklace(
/// 	std::make_unique<CircleNecklace>(Circle<Inexact>(Point<Inexact>(0, 0), 100)));
/// auto n2 = map.addNecklace(
/// 	std::make_unique<CircleNecklace>(Circle<Inexact>(Point<Inexact>(200, 0), 150)));
///
/// // add beads
/// map.addBead("NL", 17'000'000, n1);
/// map.addBead("BE", 12'000'000, n1);
/// // ...
/// map.addBead("DE", 83'000'000, n1);
/// // ...
///
/// // set parameters (optional)
/// map.parameters().buffer_rad = 0.1;
/// map.parameters().interval_type = necklace_map::IntervalType::kCentroid;
///
/// // run computation
/// map.compute();
/// ```
///
/// The necklace map can be viewed or exported by creating a \ref
/// necklace_map::Painting "Painting" and passing it to the desired \ref
/// renderer::GeometryRenderer "GeometryRenderer":
/// ```
/// necklace_map::Painting painting(map);
///
/// cartocrow::renderer::GeometryWidget widget(painting);
/// widget.show();
/// // or
/// renderer::IpeRenderer renderer(painting);
/// renderer.save("some_filename.ipe");
/// ```
class NecklaceMap {

	/// Handle pointing at a necklace, used for referring to a necklace in a
	/// \ref NecklaceMap.
	struct NecklaceHandle {
	  private:
		NecklaceHandle(size_t index);
		/// The index in \ref NecklaceMap::m_necklaces.
		size_t m_index;
		friend NecklaceMap;
	};

  public:
	/// Constructs a necklace map with the given regions and no necklaces.
	///
	/// This does not compute the necklace map: use \ref compute() to run the
	/// computation. Modifying the RegionMap passed here after the necklace
	/// map has been constructed results in undefined behavior.
	NecklaceMap(const std::shared_ptr<RegionMap> map);

	/// Adds a necklace with the given shape. Returns a handle to pass to
	/// \ref addBead() to be able to add beads to the necklace.
	NecklaceHandle addNecklace(std::unique_ptr<NecklaceShape> shape);

	/// Adds a bead to this necklace map.
	/// \param regionName The \ref Region::name "name" of the region this bead
	/// represents. Throws if a region of this name is not present in the map.
	/// \param value The data value to be displayed by the bead.
	/// \param necklace The handle of the necklace (added by \ref addNecklace())
	/// to place this bead on.
	void addBead(std::string regionName, Number<Inexact> value, NecklaceHandle& necklace);

	/// Returns the computation parameters for this necklace.
	Parameters& parameters();

	/// Computes the necklace map.
	///
	/// This method can be used more than once on the same object (for example
	/// after changing the parameters) to recompute the map.
	void compute();

	/// Returns the scale factor of this necklace map, or `0` if the map has
	/// not yet been computed.
	Number<Inexact> scaleFactor();

  private:
	/// The list of regions that this necklace map is computed for.
	const std::shared_ptr<RegionMap> m_map;
	/// The list of necklaces.
	std::vector<Necklace> m_necklaces;
	/// The computed scale factor (or 0 if the necklace map has not been
	/// computed yet).
	Number<Inexact> m_scaleFactor;
	/// The computation parameters.
	Parameters m_parameters;

	friend class Painting;
};

} // namespace cartocrow::necklace_map

#endif //CARTOCROW_NECKLACE_MAP_NECKLACE_MAP_H
