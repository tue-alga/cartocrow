#include <QLabel>
#include <QMainWindow>
#include <QSlider>

#include "cartocrow/simplification/edge_collapse/kronenfeld_etal.h"
#include <cartocrow/core/core.h>
#include <cartocrow/core/boundary_map.h>
#include <cartocrow/renderer/geometry_widget.h>

using namespace cartocrow;
using namespace cartocrow::simplification;
using namespace cartocrow::renderer;

class KSBBDemo : public QMainWindow {
	Q_OBJECT;

  public:
	KSBBDemo();
	~KSBBDemo() {
		delete hist;
		delete alg;
	};

  	void recalculate();

	GeometryWidget* m_renderer;
	QSlider* m_cSlider;
	QLabel* m_cLabel;

	int c;
	std::shared_ptr<BoundaryMap> inputmap;
	std::shared_ptr<KSBBTraits::Map> map;
	HistoricArrangement<KSBBTraits>* hist;
	KSBBSimplificationWithHistory* alg;
};
