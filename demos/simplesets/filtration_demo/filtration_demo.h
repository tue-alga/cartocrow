#ifndef CARTOCROW_FILTRATION_DEMO_H
#define CARTOCROW_FILTRATION_DEMO_H

#include <QMainWindow>
#include "cartocrow/simplesets/partition.h"
#include "cartocrow/renderer/geometry_widget.h"
#include <filesystem>

using namespace cartocrow::simplesets;
using namespace cartocrow::renderer;

class FiltrationDemo: public QMainWindow {
	Q_OBJECT

  public:
	FiltrationDemo();

  private:
	void loadFile(const std::filesystem::path& filePath);
	std::vector<std::pair<double, Partition>> m_partitions;
	GeneralSettings m_gs;
	DrawSettings m_ds;
	PartitionSettings m_ps;
	double m_cover;
	std::vector<CatPoint> m_points;
	GeometryWidget* m_renderer;
};

#endif //CARTOCROW_FILTRATION_DEMO_H
