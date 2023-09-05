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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 16-10-2020
*/

#ifndef CARTOCROW_CORE_SPIRAL_H
#define CARTOCROW_CORE_SPIRAL_H

#include <ostream>

#include "../core/core.h"
#include "polar_point.h"

namespace cartocrow::flow_map {

/// A logarithmic spiral rooted at the origin.
///
/// We say that a path \f$p\f$ is *angle-restricted* for some angle \f$\alpha\f$
/// if at any point on \f$p\f$, the angle between the tangent at that point and
/// the straight line segment to the origin is at most \f$\alpha\f$. A
/// *logarithmic spiral* is a path for which at any point this angle is exactly
/// \f$\alpha\f$, in other words, logarithmic spirals from a point bound the
/// area reachable from that point by angle-restricted paths.
///
/// A spiral is characterized by its angle \f$\alpha\f$ and its starting point,
/// expressed in polar coordinates as \f$(r, \phi)\f$. We assume here that the
/// angle is signed, i.e. if \f$\alpha > 0\f$, we get a *right spiral* (which
/// curves towards the left, rotating around the root in counter-clockwise
/// direction), and if \f$\alpha < 0\f$, we get a *left spiral* (which curves
/// towards the right, rotating around the root in clockwise direction). To
/// represent a straight line segment, we can set \f$\alpha = 0\f$.
///
/// The points on the spiral can be expressed in polar coordinates as \f$p(t)
/// = (r(t), \phi(t))\f$, where \f$r(t) = r \cdot e^{-t}\f$ and \f$\phi(t) =
/// \phi + \tan(\alpha) \cdot t\f$.
class Spiral {
  public:
	/// Constructs a spiral with the given anchor \f$(r, \phi)\f$ and angle
	/// \f$\alpha\f$.
	///
	/// Throws if \f$r = 0\f$.
	Spiral(const PolarPoint& anchor, const Number<Inexact>& angle);

	/// Constructs the shortest logarithmic spiral containing the two given
	/// points \f$p_1 = (r_1, \phi_1)\f$ and \f$p_2 = (r_2, \phi_2)\f$.
	///
	/// The anchor of the spiral is set to \f$p_1\f$ if \f$r_1 > r_2\f$ and to
	/// \f$p_2\f$ if \f$r_1 < r_2\f$. This constructor throws if \f$r_1 =
	/// r_2\f$ (in which case no spiral connecting them exists).
	///
	/// If \f$p_1\f$ or \f$p_2\f$ is the root, then the result is a line
	/// segment.
	Spiral(const PolarPoint& p1, const PolarPoint& p2);

	/// Computes the \f$\alpha\f$ of the shortest logarithmic spiral connecting
	/// the two given points \f$(r_1, \phi_1)\f$ and \f$(r_2, \phi_2)\f$.
	///
	/// This value can be computed as follows:
	/// \f[
	///     \alpha (r_1, \phi_1, r_2, \phi_2) = \arctan \left(
	///         \frac{\phi_2 - \phi_1}{-\ln(r_2 / r_1)}
	///     \right) \text{.}
	/// \f]
	static Number<Inexact> alpha(const PolarPoint& p1, const PolarPoint& p2);
	/// Computes \f$\frac{\partial\alpha}{\partial r_1}\f$ for the \f$\alpha\f$
	/// function (see \ref alpha).
	///
	/// This value can be computed as follows:
	/// \f[
	///     \frac{\partial\alpha}{\partial r_1}(r_1, \phi_1, r_2, \phi_2) =
	///     \frac{-(\phi_2 - \phi_1) \ / \ r_1}{\ln^2(r_2 / r_1) + (\phi_2 - \phi_1)^2}
	///     \text{.}
	/// \f]
	static Number<Inexact> dAlphaDR1(const PolarPoint& p1, const PolarPoint& p2);
	/// Computes \f$\frac{\partial\alpha}{\partial r_2}\f$ for the \f$\alpha\f$
	/// function (see \ref alpha).
	///
	/// This value can be computed as follows:
	/// \f[
	///     \frac{\partial\alpha}{\partial r_2}(r_1, \phi_1, r_2, \phi_2) =
	///     \frac{(\phi_2 - \phi_1) \ / \ r_2}{\ln^2(r_2 / r_1) + (\phi_2 - \phi_1)^2}
	///     \text{.}
	/// \f]
	static Number<Inexact> dAlphaDR2(const PolarPoint& p1, const PolarPoint& p2);
	/// Computes \f$\frac{\partial\alpha}{\partial\phi_1}\f$ for the
	/// \f$\alpha\f$ function (see \ref alpha).
	///
	/// This value can be computed as follows:
	/// \f[
	///     \frac{\partial\alpha}{\partial\phi_1}(r_1, \phi_1, r_2, \phi_2) =
	///     \frac{\ln(r_2 / r_1)}{\ln^2(r_2 / r_1) + (\phi_2 - \phi_1)^2}
	///     \text{.}
	/// \f]
	static Number<Inexact> dAlphaDPhi1(const PolarPoint& p1, const PolarPoint& p2);
	/// Computes \f$\frac{\partial\alpha}{\partial\phi_2}\f$ for the
	/// \f$\alpha\f$ function (see \ref alpha).
	///
	/// This value can be computed as follows:
	/// \f[
	///     \frac{\partial\alpha}{\partial\phi_2}(r_1, \phi_1, r_2, \phi_2) =
	///     \frac{-\ln(r_2 / r_1)}{\ln^2(r_2 / r_1) + (\phi_2 - \phi_1)^2}
	///     \text{.}
	/// \f]
	static Number<Inexact> dAlphaDPhi2(const PolarPoint& p1, const PolarPoint& p2);

	/// Returns the anchor of this spiral.
	const PolarPoint& anchor() const;
	/// Returns the angle of this spiral.
	const Number<Inexact>& angle() const;

	/// Checks if this spiral is a left spiral.
	bool isLeft() const;
	/// Checks if this spiral is a right spiral.
	bool isRight() const;
	/// Checks if this spiral is a line segment.
	bool isSegment() const;

	/// Evaluates this spiral at a given \f$t\f$.
	PolarPoint evaluate(const Number<Inexact>& t) const;

	/// Computes the time at which this spiral reaches the given distance
	/// \f$r\f$ from the root. Throws if \f$r \leq 0\f$.
	Number<Inexact> parameterForR(const Number<Inexact>& r) const;

	/// Computes the polar angle of the point on the spiral with the given
	/// distance \f$r\f$ from the root. Throws if \f$r \leq 0\f$.
	Number<Inexact> phiForR(const Number<Inexact>& r) const;

	/// Computes a time at which this spiral reaches the given polar angle
	/// \f$\phi\f$. Note that in general, there are infinitely many such
	/// times; this method returns an arbitrary one. To find the others,
	/// use \ref period().
	Number<Inexact> parameterForPhi(const Number<Inexact>& phi) const;

	/// Computes the angular period of this spiral. The angular period is the
	/// smallest \f$\beta\f$ such that at the points on the spiral at \f$t\f$
	/// and \f$t + \beta\f$ have the same \f$\phi\f$.
	Number<Inexact> period() const;

	/// Moves the anchor on the spiral to the given distance from the origin.
	/// Throws if \f$r \leq 0\f$.
	void moveAnchor(const Number<Inexact>& r);

  protected:
	/// The anchor \f$(r, \phi)\f$ of this spiral.
	PolarPoint m_anchor;
	/// The angle \f$\alpha\f$ of this spiral, in radians.
	Number<Inexact> m_angle;
};

std::ostream& operator<<(std::ostream& os, const Spiral& spiral);

} // namespace cartocrow::flow_map

#endif //CARTOCROW_CORE_SPIRAL_H
