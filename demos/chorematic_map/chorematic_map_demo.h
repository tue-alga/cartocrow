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
#include "cartocrow/chorematic_map/choropleth_disks.h"
#include "cartocrow/chorematic_map/sampler.h"
#include "cartocrow/chorematic_map/weighted_point.h"
#include "cartocrow/chorematic_map/input_parsing.h"

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

class ChorematicMapDemo : public QMainWindow {
	Q_OBJECT

  private:
	GeometryWidget* m_renderer;
	std::unique_ptr<Choropleth> m_choropleth;
	std::unique_ptr<Sampler<LandmarksPl>> m_sampler;
	WeightedRegionSample<Exact> m_sample;
    std::shared_ptr<std::unordered_map<std::string, RegionWeight>> m_regionWeightMap;
	std::vector<BinDisk> m_disks;
	std::vector<std::shared_ptr<VoronoiRegionArrangementPainting>> m_voronois;
	std::shared_ptr<ChoroplethPainting> m_choroplethP;
	QSlider* m_threshold;
	QSlider* m_gridSize;
	QSpinBox* m_seed;
	QSpinBox* m_nSamples;
	QSpinBox* m_voronoiIters;
    QSpinBox* m_colorBinSelector;
    QSpinBox* m_numberOfBins;
	QCheckBox* m_recomputeAutomatically;
	QCheckBox* m_invertFittingOrder;
    QCheckBox* m_applyHeuristic;
	QComboBox* m_samplingStrategy;
    QComboBox* m_dataAttribute;
	QLabel* m_diskScoreLabel;
	QLabel* m_dataInfoLabel;

	void resample();
	void refit();
	void rebin();
	void recolor();
	void loadMap(const std::filesystem::path& mapPath);
	void loadData(const std::filesystem::path& dataPath);
  public:
	ChorematicMapDemo();
};

#endif //CARTOCROW_CHOREMATIC_MAP_DEMO_H
