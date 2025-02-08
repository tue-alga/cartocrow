\mainpage Introduction

CartoCrow is a framework that simplifies the implementation of algorithms in cartographic visualization. It allows researchers to experiment with these algorithms and use them to generate maps. The framework behind CartoCrow can be used to run other cartography algorithms online. CartoCrow consists of a C++ library (which is documented here) which also provides a set of command-line applications, and a web interface (see [cartocrow-web](https://github.com/tue-alga/cartocrow-web)) which allows end users to generate maps in a user-friendly way.

The CartoCrow project was originally started by Thijs van Lankveld ([Netherlands eScience Center](https://esciencecenter.nl/)) and is now being extended and maintained by Willem Sonke ([Algorithms, Geometry & Applications](https://alga.win.tue.nl) cluster, [TU Eindhoven](https://tue.nl)) with contributions by Wouter Meulemans.

## Modules

CartoCrow consists of the following modules.

### Thematic maps

* The **necklace_map** module implements \ref cartocrow::necklace_map::NecklaceMap "necklace maps" \cite necklace_maps_tvcg \cite necklace_maps_ijcga, a type of symbol map where the symbols are not displayed on top of each region, but rather placed as ‘beads’ on curves (‘necklaces’) surrounding the map.  
  _Authors: Thijs van Lankveld, Willem Sonke_
\image html necklace-map-example.png

* The **flow_map** (work in progress!) module implements \ref cartocrow::flow_map::FlowMap "flow maps" \cite flow_maps_algorithmica \cite flow_maps_tvcg, a style of thematic maps that visualizes the movement of objects, such as people or goods, between regions.  
  _Authors: Willem Sonke, Thijs van Lankveld_
\image html flow-map-example.png

### Spatial set visualization

* The **simplesets** module implements \ref cartocrow::simplesets::SimpleSetsPainting "SimpleSets" \cite simplesets_tvcg, a spatial set visualization technique that uses simple shapes to enclosing categorical point patterns.  
  _Author: Steven van den Broek_
\image html SimpleSets-mills.svg

### Chorematic maps
* The **chorematic_map** module implements \ref cartocrow::chorematic_map::fitDisks "an algorithm" for summarizing classed region maps with a single disk choreme.  
  _Author: Steven van den Broek_
\image html chorematic_map.svg

### Simplification

The **simplification** module implements several algorithms to simplify maps consisting of polygonal regions. All algorithms are implemented to be topologically safe. Currently, the following algorithms are available:

* A generic \ref cartocrow::simplification::VertexRemovalSimplification "vertex-removal algorithm" that repeatedly removes a single vertex. A concrete implementation of the Visvalingam-Whyatt algorithm is provided via \ref cartocrow::simplification::VWTraits "VWTraits".  
  _Author: Wouter Meulemans_

* A generic \ref cartocrow::simplification::EdgeCollapseSimplification "edge-collapse algorithm" (work in progress) that repeatedly collapses an edge onto a single vertex.  
  _Author: Wouter Meulemans_

* A generic \ref cartocrow::simplification::EdgeMoveSimplification "edge-move algorithm" (work in progress) that repeatedly moves pairs of edges to reduce the number of edges, while maintaining the area of each region.  
  _Author: Wouter Meulemans_

\image html simplification-example.png

* The **isoline_simplification** module implements \ref cartocrow::isoline_simplification::IsolineSimplifier "an algorithm" \cite scalable_harmonious_simplification_cosit to simplify isolines simultaneously such that common features are maintained.  
  _Author: Steven van den Broek_
\image html harmonious-simplification.svg

### Cartograms

(A cartogram is a map in which the shapes of regions are distorted such that the size of each region corresponds to some variable, such as population.)

* The **rectangular_cartogram** (future work!) module implements \ref cartocrow::rectangular_cartogram::RectangularCartogram "rectangular cartograms" \cite rectangular_cartograms_cgta, a style of cartogram in which each region is rectangle-shaped and adjacencies between regions are strictly maintained.  
  _Authors: (to be done)_
  \image html rectangular-cartogram-example.png

* The **mosaic_map** (future work!) module implements \ref cartocrow::mosaic_map::MosaicMap "mosaic maps" \cite mosaic_maps_cgf, a style of cartogram that allows users to count individual tiles in each region to get a more precise reading.  
  _Authors: (to be done)_
  \image html mosaic-map-example.png

### Utilities

* The **core** module contains the basic primitives and utilities that are in use throughout the library. This includes a list of type aliases for basic [CGAL](https://cgal.org) types (\ref cartocrow::Point<K> "Point", \ref cartocrow::Circle<K> "Circle", \ref cartocrow::Polygon<K> "Polygon", etc.) and tools to read maps from Ipe files (\ref cartocrow::RegionList "RegionList").  
  _Author: Willem Sonke_

* The **renderer** module implements an interface that can be used to render maps. Each map type provides a \ref cartocrow::renderer:GeometryPainting "GeometryPainting", which can be handed to a \ref cartocrow::renderer::GeometryRenderer "GeometryRenderer" to draw it. In particular, the \ref cartocrow::renderer::IpeRenderer "IpeRenderer" renders the painting to an [Ipe](https://ipe.otfried.org) drawing, while the \ref cartocrow::renderer::GeometryWidget "GeometryWidget" renders the painting to a \ref QWidget for inclusion in a Qt application.  
  _Author: Willem Sonke_

* The **circle_segment_helpers** module provides helper functions for working with geometry that consist of line segments and circular arcs. The geometry classes use CGAL's circle-segment traits. Notable functions in this module compute circle (bi)tangents and a convex hull of circles. Additionally, the module provides a wrapper around the CavalierContours library, which is used to provide offset and smoothing operators on a set of circle-segment polygons.  
  _Author: Steven van den Broek_