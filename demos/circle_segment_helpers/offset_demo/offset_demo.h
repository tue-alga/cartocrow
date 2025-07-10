#ifndef CARTOCROW_OFFSET_DEMO_H
#define CARTOCROW_OFFSET_DEMO_H

#include "cartocrow/circle_segment_helpers/cs_types.h"
#include "cartocrow/renderer/geometry_widget.h"
#include "cartocrow/simplesets/partition.h"
#include <QMainWindow>
#include <filesystem>

using namespace cartocrow;
using namespace cartocrow::renderer;

class OffsetDemo: public QMainWindow {
	Q_OBJECT

  public:
	OffsetDemo();

  private:
	GeometryWidget* m_renderer;
	CSPolygonSet m_smoothed;
	CSPolygonSet m_smoothed_;
	CSPolygonSet m_dilated;
	CSPolygonSet m_eroded;
};

#endif //CARTOCROW_OFFSET_DEMO_H
