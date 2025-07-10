#ifndef CARTOCROW_CHOREMATIC_MAP_DEMO_H
#define CARTOCROW_CHOREMATIC_MAP_DEMO_H

#include <QMainWindow>
#include "cartocrow/core/core.h"
#include "cartocrow/renderer/geometry_painting.h"
#include "cartocrow/renderer/geometry_widget.h"
#include "cartocrow/core/region_map.h"
#include "cartocrow/core/region_arrangement.h"

#include "cartocrow/chorematic_map/choropleth.h"
#include "cartocrow/chorematic_map/choropleth_disks.h"
#include "cartocrow/chorematic_map/sampler.h"
#include "cartocrow/chorematic_map/weighted_point.h"
#include "cartocrow/chorematic_map/input_parsing.h"

#include "demos/widgets/double_slider.h"
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QSlider>
#include <QSpinBox>
#include <QLineEdit>

using namespace cartocrow;
using namespace cartocrow::renderer;
using namespace cartocrow::chorematic_map;

class ChorematicMapDemo : public QMainWindow {
	Q_OBJECT

  private:
	GeometryWidget* m_renderer;
	std::unique_ptr<Choropleth> m_choropleth;
	std::unique_ptr<Sampler> m_sampler;
	WeightedRegionSample<Exact> m_sample;
    std::shared_ptr<std::unordered_map<std::string, RegionWeight>> m_regionWeightMap;
	std::vector<BinDisk> m_disks;
	std::shared_ptr<ChoroplethPainting> m_choroplethP;
	DoubleSlider* m_threshold;
	DoubleSlider* m_gridSize;
	QSpinBox* m_seed;
	QSpinBox* m_nSamples;
	QSpinBox* m_voronoiIters;
    QSpinBox* m_colorBinSelector;
    QSpinBox* m_numberOfBins;
	QCheckBox* m_recomputeAutomatically;
	QCheckBox* m_invertFittingOrder;
    QCheckBox* m_applyHeuristic;
    QCheckBox* m_useSymDiff;
	QCheckBox* m_useNaturalBreaks;
	QCheckBox* m_searchForGridSize;
	QCheckBox* m_labelAtCentroid;
	QComboBox* m_samplingStrategy;
    QComboBox* m_dataAttribute;
	QLineEdit* m_regionNameAttribute;
	QLabel* m_diskScoreLabel;
	QLabel* m_dataInfoLabel;

	void resample();
	void refit();
	void rebin();
	void recolor();
	void loadMap(const std::filesystem::path& mapPath);
	void loadData(const std::filesystem::path& dataPath);
	void exportToGpkg(const std::filesystem::path& outputPath);
  public:
	ChorematicMapDemo();
};

#endif //CARTOCROW_CHOREMATIC_MAP_DEMO_H
