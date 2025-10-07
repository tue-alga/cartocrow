#pragma once

#include <QMainWindow>

#include "cartocrow/renderer/geometry_widget.h"

using namespace cartocrow;
using namespace cartocrow::renderer;

class BezierDemo : public QMainWindow {
  Q_OBJECT

  private:
  	GeometryWidget* m_renderer;

  public:
	BezierDemo();
};
