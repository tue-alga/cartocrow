#ifndef CARTOCROW_CHOREMATIC_MAP_DEMO_H
#define CARTOCROW_CHOREMATIC_MAP_DEMO_H

#include <QMainWindow>
#include "cartocrow/core/core.h"
#include "cartocrow/core/ipe_reader.h"
#include "cartocrow/renderer/geometry_painting.h"
#include "cartocrow/renderer/geometry_widget.h"
#include "cartocrow/core/region_map.h"
#include "cartocrow/core/region_arrangement.h"

#include "cartocrow/chorematic_map/weighted_point.h"
#include "cartocrow/chorematic_map/sampling.h"

#include <CGAL/Arr_landmarks_point_location.h>

#include <QSpinBox>
#include <QSlider>
#include <QCheckBox>

using namespace cartocrow;
using namespace cartocrow::renderer;
using namespace cartocrow::chorematic_map;

using Landmarks_pl = CGAL::Arr_landmarks_point_location<RegionArrangement>;

class ChorematicMapDemo : public QMainWindow {
	Q_OBJECT

  private:
	GeometryWidget* m_renderer;
	std::shared_ptr<RegionArrangement> m_regionArr;
	std::shared_ptr<Landmarks_pl> m_pl;
	std::unique_ptr<Sampler<Landmarks_pl>> m_sampler;
	std::vector<WeightedPoint> m_samples;
	std::shared_ptr<std::unordered_map<std::string, double>> m_regionData;
	std::optional<Circle<Inexact>> m_disk;
	QSlider* m_threshold;
	QSpinBox* m_seed;
	QSpinBox* m_nSamples;
	QCheckBox* m_invert;

	void recompute();
  public:
	ChorematicMapDemo();
};

#endif //CARTOCROW_CHOREMATIC_MAP_DEMO_H
