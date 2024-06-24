#ifndef CARTOCROW_MOSAIC_CARTOGRAM_PARAMETERS_H
#define CARTOCROW_MOSAIC_CARTOGRAM_PARAMETERS_H

#include <stdexcept>

#include "../core/core.h"

namespace cartocrow::mosaic_cartogram {

/// The parameters used for computing the mosaic cartogram.
struct Parameters {

	/// Whether the sea and outer regions have already been added to the input map. This parameter
	/// is optional and \c false by default.
	bool manualSeas;

	/// The value represented by one tile. This parameter is required.
	Number<Inexact> unitValue;

	/// Checks whether the parameters are valid. If not, an exception is thrown.
	void validate() const {
		if (unitValue <= 0) {
			throw std::invalid_argument("`unitValue` must be positive (Did you forget to specify it?)");
		}
	}

};

} // namespace cartocrow::mosaic_cartogram

#endif //CARTOCROW_MOSAIC_CARTOGRAM_PARAMETERS_H
