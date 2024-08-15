#ifndef CARTOCROW_DRAWING_ALGORITHM_H
#define CARTOCROW_DRAWING_ALGORITHM_H

#include "partition.h"
#include "../renderer/geometry_painting.h"

namespace cartocrow::simplesets {
struct FaceInfo {
	std::vector<int> origins;
};

using DilatedPatternArrangement =
    CGAL::Arrangement_2<CSTraits,
                        CGAL::Arr_face_extended_dcel<CSTraits, FaceInfo>>;

DilatedPatternArrangement
dilateAndArrange(const Partition& partition, const GeneralSettings& gs, const ComputeDrawingSettings& cds);

class ArrangementPainting : public renderer::GeometryPainting {
  public:
	ArrangementPainting(const DilatedPatternArrangement& arr, const GeneralSettings& gs,
	                    const DrawSettings& ds, const Partition& partition);
	void paint(renderer::GeometryRenderer& renderer) const override;

  private:
	const DilatedPatternArrangement& m_arr;
	const DrawSettings& m_ds;
	const GeneralSettings& m_gs;
	const Partition& m_partition;
};
}

#endif //CARTOCROW_DRAWING_ALGORITHM_H
