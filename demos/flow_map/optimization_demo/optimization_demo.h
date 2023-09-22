/*
The Necklace Map console application implements the algorithmic
geo-visualization method by the same name, developed by
Bettina Speckmann and Kevin Verbeek at TU Eindhoven
(DOI: 10.1109/TVCG.2010.180 & 10.1142/S021819591550003X).
Copyright (C) 2021  Netherlands eScience Center and TU Eindhoven

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <QCheckBox>
#include <QLabel>
#include <QMainWindow>
#include <QPushButton>
#include <QSlider>
#include <QTimer>

#include <memory>
#include <vector>

#include "cartocrow/core/core.h"
#include "cartocrow/flow_map/smooth_tree.h"
#include "cartocrow/renderer/geometry_widget.h"

#include "cost_graph.h"

using namespace cartocrow;
using namespace cartocrow::renderer;

class OptimizationDemo : public QMainWindow {
	Q_OBJECT;

  public:
	OptimizationDemo();

  private:
	void recalculate();
	void updateCostLabel();
	Number<Inexact> m_alpha = 25 * M_PI / 180;

	std::vector<std::shared_ptr<Point<Inexact>>> m_places;
	std::shared_ptr<cartocrow::flow_map::SmoothTree> m_smoothTree;
	int m_iterationCount = 0;

	GeometryWidget* m_renderer;
	QSlider* m_alphaSlider;
	QLabel* m_alphaLabel;
	QPushButton* m_optimizeButton;
	QCheckBox* m_stopOnNanCheckbox;
	QPushButton* m_optimizeOneStepButton;
	QTimer* m_optimizeTimer;
	QLabel* m_costLabel;
	CostGraph* m_costGraph;
};
