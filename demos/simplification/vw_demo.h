#include <QLabel>
#include <QMainWindow>
#include <QSlider>

#include "cartocrow/simplification/vertex_removal/visvalingam_whyatt.h"
#include <cartocrow/core/core.h>
#include "cartocrow/core/boundary_map.h"
#include <cartocrow/renderer/geometry_widget.h>

using namespace cartocrow;
using namespace cartocrow::simplification;
using namespace cartocrow::renderer;

class VWDemo : public QMainWindow {
	Q_OBJECT;

  public:
	VWDemo();
	~VWDemo() {
		delete hist;
	};

  private:
	void recalculate();

	GeometryWidget* m_renderer;
	QSlider* m_cSlider;
	QLabel* m_cLabel;

	int c;
	std::shared_ptr<RegionArrangement> inputmap;
	std::shared_ptr<VWTraits<std::string>::Map> map;
	HistoricArrangement<VWTraits<std::string>>* hist;
};
