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

#include "cost_graph.h"

#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QStyle>
#include <cmath>

using namespace cartocrow;

CostGraph::CostGraph() {
	setMinimumSize(QSize(300, 200));
}

void CostGraph::addStep(DataPoint costs) {
	m_dataPoints.push_back(costs);
	m_maxCost = std::max(m_maxCost, costs.m_obstacle_cost + costs.m_smoothing_cost +
	                                    costs.m_angle_restriction_cost + costs.m_balancing_cost +
	                                    costs.m_straightening_cost);
	update();
}

void CostGraph::clear() {
	m_dataPoints.clear();
	m_maxCost = 0;
	update();
}

void CostGraph::paintEvent(QPaintEvent* event) {
	QPainter painter(this);
	if (m_dataPoints.empty()) {
		painter.drawText(QPoint(20, 20), "No data points");
		return;
	}

	painter.setRenderHint(QPainter::Antialiasing);

	int marginSize = 20;
	int graphWidth = width() - 2 * marginSize;
	int graphHeight = height() - 2 * marginSize;

	painter.translate(QPoint(marginSize, graphHeight + marginSize));

	// axes
	painter.drawLine(0, 0, 0, -graphHeight);
	painter.drawLine(0, 0, graphWidth, 0);

	QColor green(52, 140, 80);
	QColor orange(255, 120, 0);
	QColor purple(110, 60, 190);
	QColor blue(33, 142, 252);
	QColor red(213, 0, 74);

	QPainterPath zeroPath;
	zeroPath.moveTo(0, 0);
	zeroPath.lineTo(graphWidth * m_dataPoints.size() /
	                    std::max(100.0f, static_cast<float>(m_dataPoints.size())),
	                0);
	QPainterPath obstaclePath = createDataPath(1, graphWidth, graphHeight);
	QPainterPath smoothingPath = createDataPath(2, graphWidth, graphHeight);
	QPainterPath angleRestrictionPath = createDataPath(3, graphWidth, graphHeight);
	QPainterPath balancingPath = createDataPath(4, graphWidth, graphHeight);
	QPainterPath straighteningPath = createDataPath(5, graphWidth, graphHeight);

	// shading
	painter.setPen(Qt::NoPen);
	painter.setOpacity(0.2);
	painter.setBrush(green);
	painter.drawPath(createRegionBetween(zeroPath, obstaclePath));
	painter.setBrush(orange);
	painter.drawPath(createRegionBetween(obstaclePath, smoothingPath));
	painter.setBrush(purple);
	painter.drawPath(createRegionBetween(smoothingPath, angleRestrictionPath));
	painter.setBrush(blue);
	painter.drawPath(createRegionBetween(angleRestrictionPath, balancingPath));
	painter.setBrush(red);
	painter.drawPath(createRegionBetween(balancingPath, straighteningPath));
	painter.setOpacity(1);

	// graphs
	painter.setPen(QPen(QColor(0, 0, 0), 1));
	painter.setBrush(Qt::NoBrush);
	painter.drawPath(obstaclePath);
	painter.drawPath(smoothingPath);
	painter.drawPath(angleRestrictionPath);
	painter.drawPath(balancingPath);
	painter.setPen(QPen(QColor(240, 90, 40), 2));
	painter.drawPath(straighteningPath);
	painter.setBrush(QColor(240, 90, 40));

	// dot
	DataPoint lastData = m_dataPoints[m_dataPoints.size() - 1];
	painter.drawEllipse(QPointF(graphWidth * (m_dataPoints.size() - 1) /
	                                std::max(100.0f, static_cast<float>(m_dataPoints.size())),
	                            -graphHeight * lastData.stackedCost(5) / m_maxCost),
	                    2, 2);

	// labels
	style()->standardPalette();
	painter.setPen(green);
	painter.drawText(0, -graphHeight * lastData.stackedCost(1) / m_maxCost, graphWidth,
	                 graphHeight * lastData.m_obstacle_cost / m_maxCost,
	                 Qt::AlignRight | Qt::AlignVCenter, "obs");
	painter.setPen(orange);
	painter.drawText(0, -graphHeight * lastData.stackedCost(2) / m_maxCost, graphWidth,
	                 graphHeight * lastData.m_smoothing_cost / m_maxCost,
	                 Qt::AlignRight | Qt::AlignVCenter, "sm");
	painter.setPen(purple);
	painter.drawText(0, -graphHeight * lastData.stackedCost(3) / m_maxCost, graphWidth,
	                 graphHeight * lastData.m_angle_restriction_cost / m_maxCost,
	                 Qt::AlignRight | Qt::AlignVCenter, "AR");
	painter.setPen(blue);
	painter.drawText(0, -graphHeight * lastData.stackedCost(4) / m_maxCost, graphWidth,
	                 graphHeight * lastData.m_balancing_cost / m_maxCost,
	                 Qt::AlignRight | Qt::AlignVCenter, "bal");
	painter.setPen(red);
	painter.drawText(0, -graphHeight * lastData.stackedCost(5) / m_maxCost, graphWidth,
	                 graphHeight * lastData.m_straightening_cost / m_maxCost,
	                 Qt::AlignRight | Qt::AlignVCenter, "str");
}

QPainterPath CostGraph::createDataPath(int costIndex, int graphWidth, int graphHeight) const {
	QPainterPath dataPath;
	dataPath.moveTo(0, -graphHeight * m_dataPoints[0].stackedCost(costIndex) / m_maxCost);
	for (int i = 1; i < m_dataPoints.size(); i += 1 + i / width()) {
		dataPath.lineTo(graphWidth * i / std::max(100.0f, static_cast<float>(m_dataPoints.size())),
		                -graphHeight * m_dataPoints[i].stackedCost(costIndex) / m_maxCost);
	}
	return dataPath;
}

QPainterPath CostGraph::createRegionBetween(const QPainterPath& first,
                                            const QPainterPath& second) const {
	QPainterPath region = first;
	region.connectPath(second.toReversed());
	return region;
}
