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

#ifndef CARTOCROW_ISOLINE_SIMPLIFICATION_DEMO_H
#define CARTOCROW_ISOLINE_SIMPLIFICATION_DEMO_H

#include "cartocrow/core/core.h"
#include "cartocrow/core/ipe_reader.h"
#include "cartocrow/isoline_simplification/isoline.h"
#include "cartocrow/isoline_simplification/isoline_simplifier.h"
#include "cartocrow/isoline_simplification/types.h"
#include "cartocrow/isoline_simplification/voronoi_helpers.h"
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
	void recalculate(bool debugInfo, bool show_vertices, bool applySmoothing);

  private:
	std::vector<Isoline<K>> m_cgal_simplified;
	std::unique_ptr<IsolineSimplifier> m_isoline_simplifier;
	std::optional<std::shared_ptr<SlopeLadder>> m_debug_ladder;

	GeometryWidget* m_renderer;
	std::function<void()> m_recalculate;
	std::function<void()> m_reload;
	std::function<void()> m_save;
	std::optional<std::string> m_dir;
	std::optional<std::string> m_output_dir;
};

Polygon<K> slope_ladder_polygon(const SlopeLadder& slope_ladder);

class VoronoiPainting : public GeometryPainting {
  public:
	VoronoiPainting(const SDG2& delaunay);

  protected:
	void paint(GeometryRenderer& renderer) const override;

  private:
	const SDG2& m_delaunay;
};

class IsolinePainting : public GeometryPainting {
  public:
	IsolinePainting(const std::vector<Isoline<K>>& isolines, bool show_vertices, bool light, bool ipe,
	                bool apply_smoothing);

  protected:
	void paint(GeometryRenderer& renderer) const override;

  private:
	const std::vector<Isoline<K>>& m_isolines;
	bool m_show_vertices;
	bool m_light;
	bool m_ipe;
	bool m_apply_smoothing;
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
	MatchingPainting(Matching& matching, std::function<bool(Point<K>)> predicate);

  protected:
	void paint(GeometryRenderer& renderer) const override;

  private:
	Matching& m_matching;
	std::function<bool(Point<K>)> m_predicate;
};

class CompleteMatchingPainting : public GeometryPainting {
  public:
	CompleteMatchingPainting(Matching& matching);

  protected:
	void paint(GeometryRenderer& renderer) const override;

  private:
	Matching& m_matching;
};

class SlopeLadderPainting : public GeometryPainting {
  public:
	SlopeLadderPainting(const Heap& slope_ladders);

  protected:
	void paint(GeometryRenderer& renderer) const override;

  private:
	const Heap& m_slope_ladders;
};

class CollapsePainting : public GeometryPainting {
  public:
	CollapsePainting(IsolineSimplifier& simplifier);

  protected:
	void paint(GeometryRenderer& renderer) const override;

  private:
	IsolineSimplifier& m_simplifier;
};

class DebugLadderPainting : public GeometryPainting {
  public:
	DebugLadderPainting(IsolineSimplifier& simplifier, SlopeLadder& ladder);

  protected:
	void paint(GeometryRenderer& renderer) const override;

  private:
	IsolineSimplifier& m_simplifier;
	SlopeLadder& m_ladder;
};

class MedialAxisExceptSeparatorPainting : public GeometryPainting {
  public:
	MedialAxisExceptSeparatorPainting(IsolineSimplifier& simplifier);

  protected:
	void paint(GeometryRenderer& renderer) const override;

  private:
	IsolineSimplifier& m_simplifier;
};

class VoronoiExceptMedialPainting : public GeometryPainting {
  public:
	VoronoiExceptMedialPainting(IsolineSimplifier& simplifier);

  protected:
	void paint(GeometryRenderer& renderer) const override;

  private:
	IsolineSimplifier& m_simplifier;
};

#endif //CARTOCROW_ISOLINE_SIMPLIFICATION_DEMO_H
