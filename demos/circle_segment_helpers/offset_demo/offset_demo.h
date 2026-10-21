/*
Copyright (C) 2026  TU Eindhoven

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

#pragma once

#include "cartocrow/circle_segment_helpers/cs_types.h"
#include "cartocrow/renderer/geometry_widget.h"
#include <QMainWindow>
#include <filesystem>

using namespace cartocrow;

class OffsetDemo: public QMainWindow {
	Q_OBJECT

  public:
	OffsetDemo();

  private:
	GeometryWidget* m_renderer;
	CSPolygonSet m_smoothed;
	CSPolygonSet m_smoothed_;
	CSPolygonSet m_dilated;
	CSPolygonSet m_eroded;
};
