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

#include "cartocrow/core/core.h"
#include "cartocrow/core/ipe_reader.h"
#include "cartocrow/isoline_simplification/isoline.h"
#include "cartocrow/isoline_simplification/isoline_simplifier.h"
#include "cartocrow/isoline_simplification/medial_axis_separator.h"
#include "cartocrow/isoline_simplification/types.h"
#include "cartocrow/renderer/geometry_painting.h"
#include "cartocrow/renderer/geometry_widget.h"
#include <QMainWindow>

using namespace cartocrow;
using namespace cartocrow::renderer;
using namespace cartocrow::isoline_simplification;

class IsolineSimplificationDemo : public QMainWindow {
	Q_OBJECT

  public:
	IsolineSimplificationDemo();
	void recalculate(bool voronoi, int target, bool cgal_simplify, int region_index, bool show_vertices, int isoline_index);

  private:
	std::vector<Isoline<K>> m_cgal_simplified;
	std::unique_ptr<IsolineSimplifier> m_isoline_simplifier;

	GeometryWidget* m_renderer;
	std::function<void()> m_recalculate;
};

std::vector<Isoline<K>> isolinesInPage(ipe::Page* page);

class MedialAxisPainting : public GeometryPainting {
  public:
	MedialAxisPainting(const SDG2& delaunay);

  protected:
	void paint(GeometryRenderer& renderer) const override;

  private:
	const SDG2& m_delaunay;
};

class IsolinePainting : public GeometryPainting {
  public:
	IsolinePainting(const std::vector<Isoline<K>>& isolines, bool show_vertices);

  protected:
	void paint(GeometryRenderer& renderer) const override;

  private:
	const std::vector<Isoline<K>>& m_isolines;
	bool m_show_vertices;
};

class MedialAxisSeparatorPainting : public GeometryPainting {
  public:
	MedialAxisSeparatorPainting(const Separator& separator, const SDG2& delaunay);

  protected:
	void paint(GeometryRenderer& renderer) const override;

  private:
 	const Separator& m_separator;
	const SDG2& m_delaunay;
};

class MatchingPainting : public GeometryPainting {
  public:
	MatchingPainting(Matching& matching, std::function<bool(Gt::Point_2)> predicate);

  protected:
	void paint(GeometryRenderer& renderer) const override;

  private:
	Matching& m_matching;
	std::function<bool(Gt::Point_2)> m_predicate;
};

class TouchedPainting : public GeometryPainting {
  public:
	TouchedPainting(std::vector<SDG2::Edge> edges, const SDG2& delaunay);

  protected:
	void paint(GeometryRenderer& renderer) const override;

  private:
	std::vector<SDG2::Edge> m_edges;
	const SDG2& m_delaunay;
};

class SlopeLadderPainting : public GeometryPainting {
  public:
	SlopeLadderPainting(const std::vector<std::shared_ptr<SlopeLadder>>& slope_ladders);

  protected:
	void paint(GeometryRenderer& renderer) const override;

  private:
	const std::vector<std::shared_ptr<SlopeLadder>>& m_slope_ladders;
};

class CollapsePainting : public GeometryPainting {
  public:
	CollapsePainting(const IsolineSimplifier& simplifier);

  protected:
	void paint(GeometryRenderer& renderer) const override;

  private:
	const IsolineSimplifier& m_simplifier;
};

#endif //CARTOCROW_ISOLINE_SIMPLIFICATION_DEMO_H