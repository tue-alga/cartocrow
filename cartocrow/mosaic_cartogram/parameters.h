#ifndef CARTOCROW_MOSAIC_CARTOGRAM_PARAMETERS_H
#define CARTOCROW_MOSAIC_CARTOGRAM_PARAMETERS_H

#include <stdexcept>

#include "../core/core.h"

namespace cartocrow::mosaic_cartogram {

/// The parameters used for computing the mosaic cartogram.
struct Parameters {

	/// Specifies the size of a tile. Specifically, each square or hexagonal tile will be scaled
	/// such that their area matches a circle with this radius.
	Number<Inexact> tileRadius;

	/// The value represented by one tile. This parameter is required.
	Number<Inexact> unitValue;

	/// Checks whether the parameters are valid. If not, an exception is thrown.
	void validate() const {
		if (tileRadius <= 0) {
			throw std::invalid_argument("'tileRadius' must be positive (Did you forget to specify it?)");
		}
		if (unitValue <= 0) {
			throw std::invalid_argument("'unitValue' must be positive (Did you forget to specify it?)");
		}
	}

};

} // namespace cartocrow::mosaic_cartogram

#endif //CARTOCROW_MOSAIC_CARTOGRAM_PARAMETERS_H
