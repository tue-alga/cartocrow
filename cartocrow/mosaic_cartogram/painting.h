#ifndef CARTOCROW_MOSAIC_CARTOGRAM_PAINTING
#define CARTOCROW_MOSAIC_CARTOGRAM_PAINTING

#include "../renderer/geometry_painting.h"
#include "../renderer/geometry_renderer.h"
#include "mosaic_cartogram.h"

namespace cartocrow::mosaic_cartogram {

/// The \ref renderer::GeometryPainting "GeometryPainting" for a \ref MosaicCartogram.
class Painting : public renderer::GeometryPainting {

  public:
	/// Options that determine what to draw in the painting.
	struct Options {
		/// Default constructor.
		Options();
		/// TODO: Placeholder option that must be replaced later.
		int placeholder;
	};

	/// Creates a new painting for the given mosaic cartogram.
	Painting(std::shared_ptr<MosaicCartogram> mosaicCartogram, Options options = {});

  protected:
	void paint(renderer::GeometryRenderer& renderer) const override;

  private:
	/// The mosaic cartogram we are drawing.
	std::shared_ptr<MosaicCartogram> m_mosaicCartogram;
	/// The drawing options.
	Options m_options;
};

} // namespace cartocrow::mosaic_cartogram

#endif //CARTOCROW_MOSAIC_CARTOGRAM_PAINTING
