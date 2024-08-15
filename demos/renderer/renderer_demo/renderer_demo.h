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

#include <QMainWindow>

#include <cartocrow/core/core.h>
#include <cartocrow/renderer/geometry_painting.h>
#include <cartocrow/renderer/geometry_widget.h>

using namespace cartocrow;
using namespace cartocrow::renderer;

/// The painting that is being drawn in the renderer demo.
class DemoPainting : public GeometryPainting {
	void paint(GeometryRenderer &renderer) const override;
};

/// A simple demo application that displays a \ref GeometryWidget with a few
/// shapes in it, and allows changing its settings.
class RendererDemo : public QMainWindow {
	Q_OBJECT;

  public:
	RendererDemo();

  private:
	GeometryWidget* m_renderer;
};
