/*
The CartoCrow library implements algorithmic geo-visualization methods,
developed at TU Eindhoven.
Copyright (C) 2021  Netherlands eScience Center and TU Eindhoven

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3f of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef CARTOCROW_ISOLINE_SIMPLIFICATION_DEMO_H
#define CARTOCROW_ISOLINE_SIMPLIFICATION_DEMO_H

#include <QMainWindow>
#include "cartocrow/core/core.h"
#include "cartocrow/isoline_simplification/isoline.h"
#include "cartocrow/isoline_simplification/medial_axis.h"
#include "cartocrow/isoline_simplification/medial_axis_separator.h"
#include "cartocrow/renderer/geometry_painting.h"
#include "cartocrow/renderer/geometry_widget.h"
#include "cartocrow/core/ipe_reader.h"

using namespace cartocrow;
using namespace cartocrow::renderer;
using namespace cartocrow::isoline_simplification;

class IsolineSimplificationDemo : public QMainWindow {
	Q_OBJECT

  public:
	IsolineSimplificationDemo();
	void recalculate(bool voronoi, int target, bool cgal_simplify, int region_index, bool show_vertices);
	void addIsolineToVoronoi(const Isoline<K>& isoline);

  private:
	std::vector<Isoline<K>> m_isolines;
	std::vector<Isoline<K>> m_cgal_simplified;
	SDG2 m_delaunay;
	Separator m_separator;
	std::vector<Gt::Segment_2> m_matching;
	GeometryWidget* m_renderer;
	std::function<void()> m_recalculate;
};

std::vector<Isoline<K>> isolinesInPage(ipe::Page* page);

class MedialAxisPainting : public GeometryPainting {
  public:
	MedialAxisPainting(SDG2& delaunay);

  protected:
	void paint(GeometryRenderer& renderer) const override;

  private:
	SDG2& m_delaunay;
};

class IsolinePainting : public GeometryPainting {
  public:
	IsolinePainting(std::vector<Isoline<K>>& isolines, bool show_vertices);

  protected:
	void paint(GeometryRenderer& renderer) const override;

  private:
	std::vector<Isoline<K>>& m_isolines;
	bool m_show_vertices;
};

class MedialAxisSeparatorPainting : public GeometryPainting {
  public:
	MedialAxisSeparatorPainting(Separator& separator, SDG2& delaunay);

  protected:
	void paint(GeometryRenderer& renderer) const override;

  private:
 	Separator& m_separator;
	SDG2& m_delaunay;
};

class MatchingPainting : public GeometryPainting {
  public:
	MatchingPainting(std::vector<Segment<K>>& matching);

  protected:
	void paint(GeometryRenderer& renderer) const override;

  private:
	std::vector<Segment<K>>& m_matching;
};

class TouchedPainting : public GeometryPainting {
  public:
	TouchedPainting(std::vector<SDG2::Edge>& edges, SDG2& delaunay);

  protected:
	void paint(GeometryRenderer& renderer) const override;

  private:
	std::vector<SDG2::Edge>& m_edges;
	SDG2& m_delaunay;
};

#endif //CARTOCROW_ISOLINE_SIMPLIFICATION_DEMO_H
