/*
The CartoCrow library implements algorithmic geo-visualization methods,
developed at TU Eindhoven.
Copyright (C) 2024 TU Eindhoven

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

#ifndef CARTOCROW_SIMPLESETS_DEMO_H
#define CARTOCROW_SIMPLESETS_DEMO_H

#include "cartocrow/core/core.h"
#include "cartocrow/core/ipe_reader.h"
#include "cartocrow/renderer/geometry_painting.h"
#include "cartocrow/renderer/geometry_widget.h"
#include "cartocrow/simplesets/types.h"
#include "cartocrow/simplesets/partition.h"
#include "cartocrow/simplesets/drawing_algorithm.h"
#include <QMainWindow>
#include <filesystem>

using namespace cartocrow;
using namespace cartocrow::renderer;
using namespace cartocrow::simplesets;

class SimpleSetsDemo : public QMainWindow {
	Q_OBJECT

  public:
	SimpleSetsDemo();
	void resizeEvent(QResizeEvent *event) override;

  private:
	std::vector<CatPoint> m_points;
	Partition m_partition;
	std::shared_ptr<DilatedPatternDrawing> m_dpd;
	GeneralSettings m_gs;
	DrawSettings m_ds;
	PartitionSettings m_ps;
	ComputeDrawingSettings m_cds;
	GeometryWidget* m_renderer;
	std::vector<std::pair<Number<Inexact>, Partition>> m_partitions;

	std::shared_ptr<Point<Inexact>> m_cc;
	void fitToScreen();
	void loadFile(const std::filesystem::path& filePath);
	void computePartitions();
	void computeDrawing(double cover);
};

#endif //CARTOCROW_SIMPLESETS_DEMO_H
