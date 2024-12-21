#ifndef CARTOCROW_CHOREMATIC_MAP_DEMO_H
#define CARTOCROW_CHOREMATIC_MAP_DEMO_H

#include <QMainWindow>
#include "cartocrow/core/core.h"
#include "cartocrow/core/ipe_reader.h"
#include "cartocrow/renderer/geometry_painting.h"
#include "cartocrow/renderer/geometry_widget.h"
#include "cartocrow/core/region_map.h"
#include "cartocrow/core/region_arrangement.h"

#include "cartocrow/chorematic_map/choropleth.h"
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

using RegionWeights = std::unordered_map<std::string, double>;

class ChorematicMapDemo : public QMainWindow {
	Q_OBJECT

  private:
	GeometryWidget* m_renderer;
	std::unique_ptr<Choropleth> m_choropleth;
	std::unique_ptr<Sampler<LandmarksPl>> m_sampler;
	std::vector<WeightedPoint> m_samples;
	std::shared_ptr<RegionWeights> m_regionWeight;
    std::shared_ptr<std::unordered_map<std::string, RegionWeights>> m_regionWeightMap;
	std::optional<Circle<Inexact>> m_disk;
	std::vector<std::shared_ptr<VoronoiRegionArrangementPainting>> m_voronois;
	std::shared_ptr<ChoroplethPainting> m_choroplethP;
	QSlider* m_threshold;
	QSlider* m_gridSize;
	QSpinBox* m_seed;
	QSpinBox* m_nSamples;
	QSpinBox* m_voronoiIters;
	QSpinBox* m_binFit;
    QSpinBox* m_colorBinSelector;
    QSpinBox* m_numberOfBins;
	QCheckBox* m_recomputeAutomatically;
	QComboBox* m_samplingStrategy;
    QComboBox* m_dataAttribute;
	QLabel* m_diskCostLabel;
	QLabel* m_dataInfoLabel;

	void resample();
	void reweight();
	void refit();
	void rebin();
	void loadMap(const std::filesystem::path& mapPath);
	void loadData(const std::filesystem::path& dataPath);
  public:
	ChorematicMapDemo();
};

#endif //CARTOCROW_CHOREMATIC_MAP_DEMO_H
