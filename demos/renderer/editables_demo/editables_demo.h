/*
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
#include <QMainWindow>

#include <cartocrow/core/core.h>
#include <cartocrow/renderer/geometry_painting.h>
#include <cartocrow/renderer/geometry_widget.h>

using namespace cartocrow;
using namespace cartocrow::renderer;

/// The painting that is being drawn in the editables demo.
class DemoPainting : public GeometryPainting {
  public:
	DemoPainting(std::shared_ptr<Point<Inexact>> p1, std::shared_ptr<Point<Inexact>> p2,
	             std::shared_ptr<Point<Inexact>> p3);
	void paint(GeometryRenderer &renderer) const override;

  private:
	std::shared_ptr<Point<Inexact>> m_p1;
	std::shared_ptr<Point<Inexact>> m_p2;
	std::shared_ptr<Point<Inexact>> m_p3;
};

/// A demo application that displays a \ref GeometryWidget with some editables.
class EditablesDemo : public QMainWindow {
	Q_OBJECT;

  public:
	EditablesDemo();

  private:
	GeometryWidget* m_renderer;
};
