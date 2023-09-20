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

#ifndef CARTOCROW_DEMOS_FLOWMAP_OPTIMIZATIONDEMO_COSTGRAPH_H
#define CARTOCROW_DEMOS_FLOWMAP_OPTIMIZATIONDEMO_COSTGRAPH_H

#include <QWidget>

#include <array>
#include <vector>

#include "cartocrow/core/core.h"

using namespace cartocrow;

/// Simple widget that displays the cost of a \ref SmoothTree as it changes
/// during the optimization procedure.
class CostGraph : public QWidget {
	Q_OBJECT;

  public:
	/// One data point containing all cost types.
	struct DataPoint {
		Number<Inexact> m_obstacle_cost;
		Number<Inexact> m_smoothing_cost;
		Number<Inexact> m_angle_restriction_cost;
		Number<Inexact> m_balancing_cost;
		Number<Inexact> m_straightening_cost;

		inline Number<Inexact> stackedCost(int count) const {
			std::array<Number<Inexact> DataPoint::*, 5> stackingOrder = {
			    &DataPoint::m_obstacle_cost, &DataPoint::m_smoothing_cost,
			    &DataPoint::m_angle_restriction_cost, &DataPoint::m_balancing_cost,
			    &DataPoint::m_straightening_cost};
			Number<Inexact> sum = 0;
			for (int i = 0; i < count; ++i) {
				sum += this->*stackingOrder[i];
			}
			return sum;
		}
	};
	/// Creates a cost graph without any cost data.
	CostGraph();
	/// Adds a new cost data point to the graph.
	void addStep(DataPoint costs);
	/// Clears all the cost data points.
	void clear();

  protected:
	void paintEvent(QPaintEvent* event) override;

  private:
	/// The ordered list of cost data points.
	std::vector<DataPoint> m_dataPoints;
	/// The maximum total cost we've seen so far.
	Number<Inexact> m_maxCost = 0;

	QPainterPath createDataPath(int costIndex, int graphWidth, int graphHeight) const;
};

#endif
