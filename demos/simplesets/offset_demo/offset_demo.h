#ifndef CARTOCROW_OFFSET_DEMO_H
#define CARTOCROW_OFFSET_DEMO_H

#include <QMainWindow>
#include "cartocrow/simplesets/partition.h"
#include "cartocrow/renderer/geometry_widget.h"
#include <filesystem>
#include "cavc/include/cavc/polylineoffset.hpp"

using namespace cartocrow::simplesets;
using namespace cartocrow::renderer;

class OffsetDemo: public QMainWindow {
	Q_OBJECT

  public:
	OffsetDemo();

  private:
	GeometryWidget* m_renderer;
	std::vector<cavc::Polyline<double>> m_results;
};

#endif //CARTOCROW_OFFSET_DEMO_H
