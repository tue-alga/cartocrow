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
*/

#ifndef CARTOCROW_FLOW_MAP_SMOOTH_TREE_H
#define CARTOCROW_FLOW_MAP_SMOOTH_TREE_H

#include <deque>
#include <map>
#include <memory>
#include <queue>
#include <vector>

#include "spiral_tree.h"

namespace cartocrow::flow_map {

/// A smoothed tree.
class SmoothTree {

  public:
	/// Constructs a smooth tree.
	SmoothTree(const std::shared_ptr<SpiralTree> spiralTree);

	/// Returns a list of the nodes in this smooth tree.
	const std::vector<std::shared_ptr<Node>>& nodes() const;

	/// Computes the total cost of the tree.
	Number<Inexact> computeCost();

	/// Performs one optimization step.
	void optimize();

  //private:  // TODO temporary
	/// The spiral tree underlying this smooth tree.
	std::shared_ptr<SpiralTree> m_tree;

	/// List of nodes in this tree.
	std::vector<std::shared_ptr<Node>> m_nodes;

	std::shared_ptr<Node> constructSmoothTree(const std::shared_ptr<Node>& node, Number<Inexact> maxRStep);

	struct PolarForce {
		Number<Inexact> r = 0;
		Number<Inexact> phi = 0;
	};
	std::vector<PolarForce> m_forces;

	/// Computes the smoothing cost for the subdivision node `i` at \f$(r,
	/// \phi)\f$, with parent `iParent` at \f$(r_p, \phi_p)\f$ and child
	/// `iChild` at \f$(r_c, \phi_c)\f$.
	///
	/// \f[
	///     F_\text{smooth}(r, \phi, r_p, \phi_p, r_c, \phi_c) =
	///     c_\text{smooth} \cdot
	///     \big( \alpha(r_p, \phi_p, r, \phi)
	///         - \alpha(r, \phi, r_c, \phi_c) \big)^2
	///     \text{.}
	/// \f]
	Number<Inexact> computeSmoothingCost(int i, int iParent, int iChild);
	/// Applies smoothing forces in \ref m_forces to the subdivision node `i`,
	/// its parent `iParent`, and its child `iChild`.
	///
	/// The forces are defined by the partial derivatives of the smoothing cost
	/// (see \ref computeSmoothingCost), which are:
	///
	/// \f{align*}{
	///     \frac{\partial F_\text{smooth}}{\partial r}(r, \phi, r_p, \phi_p, r_c, \phi_c)\ &=
	///     2c_\text{smooth} \cdot
	///     \big( \alpha(r_p, \phi_p, r, \phi) - \alpha(r, \phi, r_c, \phi_c) \big) \cdot
	///     \Big( \frac{\partial\alpha}{\partial r_2}(r_p, \phi_p, r, \phi)
	///         - \frac{\partial\alpha}{\partial r_1}(r, \phi, r_c, \phi_c) \Big)
	///     \text{;} \\
	///     \frac{\partial F_\text{smooth}}{\partial r_p}(r, \phi, r_p, \phi_p, r_c, \phi_c)\ &=
	///     2c_\text{smooth} \cdot
	///     \big( \alpha(r_p, \phi_p, r, \phi) - \alpha(r, \phi, r_c, \phi_c) \big) \cdot
	///     \frac{\partial\alpha}{\partial r_1}(r_p, \phi_p, r, \phi)
	///     \text{;} \\
	///     \frac{\partial F_\text{smooth}}{\partial r_c}(r, \phi, r_p, \phi_p, r_c, \phi_c)\ &=
	///     2c_\text{smooth} \cdot
	///     \big( \alpha(r_p, \phi_p, r, \phi) - \alpha(r, \phi, r_c, \phi_c) \big) \cdot
	///     -\frac{\partial\alpha}{\partial r_2}(r, \phi, r_c, \phi_c)
	///     \text{;} \\
	/// \f}
	/// et cetera.
	void applySmoothingForce(int i, int iParent, int iChild);
	
	/// Computes the angle restriction cost for the join node `i` at \f$(r,
	/// \phi)\f$, with children `iChild1` at \f$(r_{c_1}, \phi_{c_1})\f$ and
	/// `iChild2` at \f$(r_{c_2}, \phi_{c_2})\f$.
	///
	/// \f[
	///     F_\text{AR}(r, \phi, r_{c_1}, \phi_{c_1}, r_{c_2}, \phi_{c_2}) =
	///     c_\text{AR} \cdot
	///     \big( \log(\sec \alpha(r, \phi, r_{c_1}, \phi_{c_1}))
	///         + \log(\sec \alpha(r, \phi, r_{c_2}, \phi_{c_2})) \big)
	///     \text{.}
	/// \f]
	Number<Inexact> computeAngleRestrictionCost(int i, int iChild1, int iChild2);
	/// Applies angle restriction forces in \ref m_forces to the subdivision
	/// node `i` and its children `iChild1` and `iChild2`.
	///
	/// The forces are defined by the partial derivatives of the angle
	/// restriction cost (see \ref computeAngleRestrictionCost), which are:
	///
	/// \f{align*}{
	///     \frac{\partial F_\text{AR}}{\partial r}(r, \phi, r_{c_1}, \phi_{c_1}, r_{c_2}, \phi_{c_2})\ &=
	///     c_\text{AR} \cdot
	///     \Big( \frac{\partial\alpha}{\partial r_1}(r, \phi, r_{c_1}, \phi_{c_1}) \cdot
	///             \tan \alpha(r, \phi, r_{c_1}, \phi_{c_1}) +
	///         \frac{\partial\alpha}{\partial r_1}(r, \phi, r_{c_2}, \phi_{c_2}) \cdot
	///             \tan \alpha(r, \phi, r_{c_2}, \phi_{c_2}) \Big)
	///     \text{;} \\
	///     \frac{\partial F_\text{AR}}{\partial r_{c_1}}(r, \phi, r_{c_1}, \phi_{c_1}, r_{c_2}, \phi_{c_2})\ &=
	///     c_\text{AR} \cdot
	///     \frac{\partial\alpha}{\partial r_2}(r, \phi, r_{c_1}, \phi_{c_1}) \cdot
	///         \tan \alpha(r, \phi, r_{c_1}, \phi_{c_1})
	///     \text{;} \\
	/// \f}
	/// et cetera.
	void applyAngleRestrictionForce(int i, int iChild1, int iChild2);

	Number<Inexact> m_obstacle_factor = 2.0;
	Number<Inexact> m_smoothing_factor = 0.4;
	Number<Inexact> m_straightening_factor = 0.4;
	Number<Inexact> m_angle_restriction_factor = 0.077;
};

} // namespace cartocrow::flow_map

#endif
