/*
Copyright (C) 2026  TU Eindhoven

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

#include "core.h"
#include <algorithm>
#include <cmath>
#include <CGAL/number_utils_classes.h>

namespace cartocrow {

Color Color::shaded(double f) const {
	f = std::clamp(f, 0.0, 2.0);
	if (f < 1) {
		// darken (shade)
		return {
			(int) std::round(r * f),
			(int) std::round(g * f),
			(int) std::round(b * f)
		};
	} else {
		// lighten (tint)
		f -= 1;
		return {
			(int) std::round(r + (255 - r) * f),
			(int) std::round(g + (255 - g) * f),
			(int) std::round(b + (255 - b) * f),
		};
	}
}
Color::Color() : r(0), g(0), b(0) {}
Color::Color(int r, int g, int b) : r(r), g(g), b(b) {}
Color::Color(int rgb) : r((rgb & 0xff0000) >> 16), g((rgb & 0x00ff00) >> 8), b(rgb & 0x0000ff) {}

Number<Exact> pretendExact(const Number<Inexact>& v) {
	return convert_kernel<Exact>(v);
}

Number<Exact> pretendExact(const Number<Exact>& v) {
	return convert_kernel<Exact>(v);
}

Number<Inexact> approximate(const Number<Inexact>& v) {
	return convert_kernel<Inexact>(v);
}

Number<Inexact> approximate(const Number<Exact>& v) {
	return convert_kernel<Inexact>(v);
}

} // namespace cartocrow
