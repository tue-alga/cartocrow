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
#include <QComboBox>
#include <QLabel>

using namespace cartocrow;
using namespace cartocrow::renderer;
using namespace cartocrow::chorematic_map;

using LandmarksPl = CGAL::Arr_landmarks_point_location<RegionArrangement>;

class VoronoiRegionArrangementPainting : public GeometryPainting {
  public:
	std::shared_ptr<VoronoiRegionArrangement> m_arr;

	VoronoiRegionArrangementPainting(std::shared_ptr<VoronoiRegionArrangement> arr)
	    : m_arr(std::move(arr)){};

	void paint(GeometryRenderer& renderer) const override;
};

class RegionArrangementPainting : public GeometryPainting {
  public:
	std::shared_ptr<RegionArrangement> m_arr;
	std::shared_ptr<std::unordered_map<std::string, double>> m_weights;
	RegionArrangementPainting(std::shared_ptr<RegionArrangement> arr,
	                          std::shared_ptr<std::unordered_map<std::string, double>> weights)
	    : m_arr(std::move(arr)), m_weights(std::move(weights)){};

	void paint(GeometryRenderer& renderer) const override;
};

class ChorematicMapDemo : public QMainWindow {
	Q_OBJECT

  private:
	GeometryWidget* m_renderer;
	std::shared_ptr<RegionArrangement> m_regionArr;
	std::shared_ptr<LandmarksPl> m_pl;
	std::unique_ptr<Sampler<LandmarksPl>> m_sampler;
	std::vector<WeightedPoint> m_samples;
	std::shared_ptr<std::unordered_map<std::string, double>> m_regionData;
	std::shared_ptr<std::unordered_map<std::string, double>> m_regionWeight;
	std::optional<Circle<Inexact>> m_disk;
	std::vector<Component<RegionArrangement>> m_comps;
	std::vector<std::shared_ptr<RegionArrangement>> m_compArrs;
	std::vector<std::shared_ptr<LandmarksPl>> m_pls;
	std::vector<std::shared_ptr<VoronoiRegionArrangementPainting>> m_voronois;
	std::vector<Rectangle<Exact>> m_bbs;
	std::vector<PolygonWithHoles<Exact>> m_outerPolys;
	std::shared_ptr<RegionArrangementPainting> m_rap;
	QSlider* m_threshold;
	QSpinBox* m_seed;
	QSpinBox* m_nSamples;
	QSpinBox* m_voronoiIters;
	QCheckBox* m_invert;
	QCheckBox* m_recomputeAutomatically;
	QComboBox* m_samplingStrategy;
	QLabel* m_diskCostLabel;

	void resample();
	void reweight();
	void refit();
	void rebin();
	void loadMap(const std::filesystem::path& mapPath);
	void loadData(const std::filesystem::path& dataPath);
	void computeComponentInfo();
  public:
	ChorematicMapDemo();
};

#endif //CARTOCROW_CHOREMATIC_MAP_DEMO_H
