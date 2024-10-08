#ifndef CARTOCROW_CIRCLE_CONVEX_HULL_H
#define CARTOCROW_CIRCLE_CONVEX_HULL_H

#include "cartocrow/core/core.h"
#include "cartocrow/core/ipe_reader.h"
#include "cartocrow/renderer/geometry_painting.h"
#include "cartocrow/renderer/geometry_widget.h"
#include "cartocrow/simplesets/types.h"
#include "cartocrow/simplesets/partition.h"
#include "cartocrow/simplesets/drawing_algorithm.h"
#include <QMainWindow>

using namespace cartocrow;
using namespace cartocrow::renderer;
using namespace cartocrow::simplesets;

class CircleConvexHullDemo: public QMainWindow {
	Q_OBJECT

  public:
	CircleConvexHullDemo();

};

#endif //CARTOCROW_CIRCLE_CONVEX_HULL_H
