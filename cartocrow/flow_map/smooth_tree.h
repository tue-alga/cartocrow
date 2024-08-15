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
	/// Computes the obstacle cost of the entire tree.
	Number<Inexact> computeObstacleCost();
	/// Computes the smoothing cost of the entire tree.
	Number<Inexact> computeSmoothingCost();
	/// Computes the angle restriction cost of the entire tree.
	Number<Inexact> computeAngleRestrictionCost();
	/// Computes the balancing cost of the entire tree.
	Number<Inexact> computeBalancingCost();
	/// Computes the straightening cost of the entire tree.
	Number<Inexact> computeStraighteningCost();

	/// Performs one optimization step.
	void optimize();

  private:
	/// The spiral tree underlying this smooth tree.
	std::shared_ptr<SpiralTree> m_tree;

	Number<Inexact> m_restrictingAngle;

	/// List of nodes in this tree.
	std::vector<std::shared_ptr<Node>> m_nodes;

	std::shared_ptr<Node> constructSmoothTree(const std::shared_ptr<Node>& node, Number<Inexact> maxRStep);

	struct PolarGradient {
		Number<Inexact> r = 0;
		Number<Inexact> phi = 0;
	};
	std::vector<PolarGradient> m_gradient;

	/// Computes the obstacle cost for the subdivision or join node `i` at
	/// \f$(r, \phi)\f$ to the given obstacle leaf node at \f$(r_{\text{obs}},
	/// \phi_{\text{obs}})\f$.
	///
	/// \f[
	///     F_\text{obs}(r, \phi) =
	///     c_\text{obs} \cdot
	///     \begin{cases}
	///         \frac{t}{BD} \big( \frac{B}{2} + t \big) +
	///             \frac{D}{Bt} \big( \frac{B}{2} - t \big) & \text{if $D < t$;} \\
	///         \big( 1 - \frac{D - t}{B} \big)^2 & \text{if $t \leq D < t + B$;} \\
	///         0 & \text{otherwise,} \\
	///     \end{cases}
	/// \f]
	///
	/// where \f$t\f$ is the thickness of the flow tree at this node, \f$B\f$ is
	/// a buffer size, and \f$D\f$ is the distance between \f$(r, \phi)\f$ and
	/// \f$(r_{\text{obs}}, \phi_{\text{obs}})\f$.
	Number<Inexact> computeObstacleCost(int i, Number<Inexact> thickness, PolarPoint obstacle);

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
	/// Applies the smoothing gradient in \ref m_gradient to the subdivision node
	/// `i`, its parent `iParent`, and its child `iChild`.
	///
	/// The gradient is defined by the partial derivatives of the smoothing cost
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
	void applySmoothingGradient(int i, int iParent, int iChild);
	
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
	/// Applies the angle restriction gradient in \ref m_gradient to the join node
	/// `i` and its children `iChild1` and `iChild2`.
	///
	/// The gradient is defined by the partial derivatives of the angle
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
	void applyAngleRestrictionGradient(int i, int iChild1, int iChild2);
	/// Computes the balancing cost for the join node `i` at \f$(r, \phi)\f$,
	/// with children `iChild1` at \f$(r_{c_1}, \phi_{c_1})\f$ and `iChild2` at
	/// \f$(r_{c_2}, \phi_{c_2})\f$.
	///
	/// \f[
	///     F_\text{balance}(r, \phi, r_{c_1}, \phi_{c_1}, r_{c_2}, \phi_{c_2}) =
	///     c_\text{AR} \cdot
	///     2 \tan^2(\alpha) \log\Big( \csc \Big(
	///         \frac{\alpha(r, \phi, r_{c_1}, \phi_{c_1}) - \alpha(r, \phi, r_{c_2}, \phi_{c_2})}{2}
	///     \Big) \Big)
	///     \text{.}
	/// \f]
	Number<Inexact> computeBalancingCost(int i, int iChild1, int iChild2);
	/// Applies the balancing gradient in \ref m_gradient to the join node `i` and
	/// its children `iChild1` and `iChild2`.
	///
	/// The gradient is defined by the partial derivatives of the balancing cost
	/// (see \ref computeBalancingCost), which are:
	///
	/// \f{multline}{
	///     \frac{\partial F_\text{balance}}{\partial r}(r, \phi, r_{c_1}, \phi_{c_1}, r_{c_2}, \phi_{c_2})\ =
	///     c_\text{AR} \cdot -\tan^2(\alpha) \cdot
	///     \cot \Big(
	///         \frac{\alpha(r, \phi, r_{c_1}, \phi_{c_1}) - \alpha(r, \phi, r_{c_2}, \phi_{c_2})}{2}
	///     \Big) \\
	///     \cdot \Big( \frac{\partial\alpha}{\partial r_1}(r, \phi, r_{c_1}, \phi_{c_1}) -
	///         \frac{\partial\alpha}{\partial r_1}(r, \phi, r_{c_2}, \phi_{c_2}) \Big)
	///     \text{;} \\
	/// \f}
	/// et cetera.
	void applyBalancingGradient(int i, int iChild1, int iChild2);
	/// Computes the straightening cost for the join node `i` at \f$(r,
	/// \phi)\f$, with parent `iParent` at \f$(r_p, \phi_p)\f$ and children
	/// `children` at \f$(r_{c_i}, \phi_{c_i})\f$.
	///
	/// \f[
	///     F_\text{balance}(r, \phi, r_p, \phi_p, \{r_{c_i}\}, \{\phi_{c_i}\}) =
	///     c_\text{straighten} \cdot
	///     \Big( \alpha(r_p, \phi_p, r, \phi) -
	///         \frac{\sum_{i \mid t_i \geq ct^*} t_i \alpha(r, \phi, r_{c_i}, \phi_{c_i})}
	///             {\sum_{i \mid t_i \geq ct^*} t_i}
	///     \Big)^2
	///     \text{.}
	/// \f]
	Number<Inexact> computeStraighteningCost(int i, int iParent,
	                                         const std::vector<std::shared_ptr<Node>>& children);
	/// Applies the straightening gradient in \ref m_gradient to the join node `i`
	/// at \f$(r, \phi)\f$, with parent `iParent` at \f$(r_p, \phi_p)\f$ and
	/// children `children` at \f$(r_{c_i}, \phi_{c_i})\f$.
	///
	/// The gradient is defined by the partial derivatives of the straightening
	/// cost (see \ref computeStraighteningCost), which are:
	///
	/// \f{multline}{
	///     \frac{\partial F_\text{straighten}}{\partial r}(r, \phi, r_p, \phi_p, \{r_{c_i}\}, \{\phi_{c_i}\}) =
	///     2c_\text{straighten} \cdot
	///     \bigg( \alpha(r_p, \phi_p, r, \phi) -
	///         \frac{\sum_{i \mid t_i \geq ct^*} t_i \alpha(r, \phi, r_{c_i}, \phi_{c_i})}
	///             {\sum_{i \mid t_i \geq ct^*} t_i} \bigg) \\
	///     \cdot \bigg(
	///         \frac{\partial\alpha}{\partial r_2}(r_p, \phi_p, r, \phi) -
	///         \frac{\sum_{i \mid t_i \geq ct^*} t_i \frac{\partial\alpha}{\partial r_1}(r, \phi, r_{c_i}, \phi_{c_i})}
	///             {\sum_{i \mid t_i \geq ct^*} t_i} \bigg)
	///     \text{;} \\
	/// \f}
	/// et cetera.
	void applyStraighteningGradient(int i, int iParent,
	                                const std::vector<std::shared_ptr<Node>>& children);

	Number<Inexact> m_obstacleFactor = 2.0;
	Number<Inexact> m_smoothingFactor = 0.4;
	Number<Inexact> m_straighteningFactor = 0.4;
	Number<Inexact> m_angle_restrictionFactor = 0.077;

	Number<Inexact> m_bufferSize = 1.0; // TODO
	Number<Inexact> m_relevantFlowFactor = 0.5;
};

} // namespace cartocrow::flow_map

#endif
