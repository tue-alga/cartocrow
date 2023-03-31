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

#include <QLabel>
#include <QMainWindow>
#include <QSlider>

#include <cartocrow/core/core.h>
#include <cartocrow/renderer/geometry_widget.h>

using namespace cartocrow;
using namespace cartocrow::renderer;

class SpiralTreeDemo : public QMainWindow {
	Q_OBJECT;

  public:
	SpiralTreeDemo();

  private:
	void recalculate();
	Point<Inexact>* findClosestPoint(Point<Inexact> p, Number<Inexact> radius);
	Polygon<Inexact> m_obstacle;
	Point<Inexact>* m_draggedPoint = nullptr;
	Number<Inexact> m_alpha = 25 * M_PI / 180;

	GeometryWidget* m_renderer;
	QSlider* m_alphaSlider;
	QLabel* m_alphaLabel;
};
