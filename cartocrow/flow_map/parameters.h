/*
The Flow Map library implements the algorithmic geo-visualization
method by the same name, developed by Kevin Verbeek, Kevin Buchin,
and Bettina Speckmann at TU Eindhoven
(DOI: 10.1007/s00453-013-9867-z & 10.1109/TVCG.2011.202).
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 02-09-2020
*/

#ifndef CARTOCROW_FLOW_MAP_PARAMETERS_H
#define CARTOCROW_FLOW_MAP_PARAMETERS_H

#include "cartocrow/core/core_types.h"

namespace cartocrow::flow_map {

/// A struct to collect the parameters used for computing a flow map.
struct Parameters {
	/// Construct a collection of parameters.
	/// All parameters are initialized as valid values.
	Parameters();

	/// The maximum angle between the line connecting the root and any point on
	/// a tree arc and arc's tangent line at that point.
	/**
	 * This angle must be in the range \f$(0, \pi / 2)\f$.
	 *
	 * In practice, all arcs of a spiral tree are either straight lines or
	 * \f$\alpha\f$-spirals, where \f$\alpha\f$ is this restriction angle.
	 * These spirals can be expressed as polar coordinates around the root node
	 * \f[ p(t) = (R(t), \phi(t)), \f]
	 * where \f$R(t) = R(0) \cdot e^{-t}\f$ and \f$\phi(t) = \phi(0) +
	 * \tan(\alpha) \cdot t\f$.
	 *
	 * Note that generally \f$p(0)\f$ is the polar coordinates of a node of a
	 * spiral arc, positive \f$t\f$ values are closer to the root, and negative
	 * \f$t\f$ values are farther from the root.
	 */
	Number restricting_angle;
};

} // namespace cartocrow::flow_map

#endif //CARTOCROW_FLOW_MAP_PARAMETERS_H
