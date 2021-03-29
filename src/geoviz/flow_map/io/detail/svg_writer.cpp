/*
The Flow Map library implements the algorithmic geo-visualization
method by the same name, developed by Kevin Verbeek, Kevin Buchin,
and Bettina Speckmann at TU Eindhoven
(DOI: 10.1007/s00453-013-9867-z & 10.1109/TVCG.2011.202).
Copyright (C) 2019  Netherlands eScience Center and TU Eindhoven

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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 16-10-2020
*/

#include "svg_writer.h"

#include <glog/logging.h>

#include "geoviz/common/bounding_box.h"
#include "geoviz/common/spiral.h"


namespace geoviz
{
namespace
{

constexpr const char* kSvgVersion = "1.1";
constexpr const char* kSvgAttributeBounds = "bounds";

constexpr const char* kFilterDropShadowId = "filterDropShadow";

constexpr const char* kFlowStyle = "fill:none;"//rgba(100%,10%,10%,100%);"
                                   "stroke:rgba(0%,0%,0%,100%);"
                                   "stroke-linecap:butt;"
                                   "stroke-linejoin:round;";

constexpr const char* kRootStyle = "fill:rgba(100%,30%,0%,100%);"
                                   "stroke:rgba(0%,0%,0%,100%);"
                                   "stroke-linecap:butt;"
                                   "stroke-linejoin:miter;";
constexpr const char* kLeafStyle = "fill:rgba(100%,30%,0%,100%);"
                                   "stroke:rgba(0%,0%,0%,100%);"
                                   "stroke-linecap:butt;"
                                   "stroke-linejoin:round;";
constexpr const char* kJoinStyle = "fill:rgba(0%,0%,0%,100%);"
                                   "stroke:rgba(0%,0%,0%,100%);"
                                   "stroke-linecap:butt;"
                                   "stroke-linejoin:round;";
constexpr const char* kSubdivisionStyle = "fill:rgba(0%,30%,100%,100%);"
                                          "stroke:rgba(0%,0%,0%,100%);"
                                          "stroke-linecap:butt;"
                                          "stroke-linejoin:round;";

constexpr const char* kObstacleStyle = "fill:rgb(80%,80%,80%);"
                                       "stroke:rgb(50%,50%,50%,100%);"
                                       "stroke-width:0.4;"
                                       "stroke-linecap:butt;"
                                       "stroke-linejoin:round;";
constexpr const char* kVertexStyle = "fill:rgba(0%,0%,0%,100%);"
                                     "stroke:rgba(0%,0%,0%,100%);"
                                     "stroke-linecap:butt;"
                                     "stroke-linejoin:round;"
                                     "stroke-width:0;";

constexpr char kAbsoluteMove = 'M';
constexpr char kAbsoluteCubicBezier = 'C';
constexpr char kAbsoluteClose = 'Z';


// Note that this source file contains string literals in various other places.
// However, it is likely that whenever these have to change, detailed knowledge of the SVG file structure is required.
// In this case, you will have to dive into the code anyway.

constexpr const double kTransformScale = 1;

constexpr const double kLineWidthPx = 0.2;//1.7;
constexpr const double kPointRegionRadiusPx = 3;
constexpr const double kRootWidthPx = 6;
constexpr const double kLeafRadiusPx = 3;
constexpr const double kJoinRadiusPx = 2;
constexpr const double kSubdivisionRadiusPx = 3;
constexpr const double kVertexRadiusPx = 1.5;
constexpr const double kBoundingBoxBufferPx = 5;

constexpr const double kSpiralStep = 0.1;
constexpr const double kSpiralMax = 6.0;

constexpr const double kDropShadowShade = 0.9;
constexpr const int kDropShadowExtentPx = 2;

void Split(const std::string& string, const std::string& split, std::string& before, std::string& after)
{
  // Find the start of the split.
  const size_t from = string.find(split.c_str());
  if (from == std::string::npos)
  {
    before = "";
    after = string;
    return;
  }

  // Find the end of the split.
  const size_t to = string.find(";", from);

  // Perform the split.
  before = string.substr(0, from);
  after = to == std::string::npos ? "" : string.substr(to + 1);
}

void GetStyle(const std::string& style, const std::string& name, std::string& value)
{
  // Find the start of the style.
  const size_t start = style.find(name.c_str());
  if (start == std::string::npos)
  {
    value = "";
    return;
  }
  const size_t from = start + name.length();

  // Find the end of the style.
  const size_t to = style.find(";", from);

  value = style.substr(from, to - from);
}

template<typename T_>
std::string ForceStyle(const std::string& style, const std::string& name, const T_& value)
{
  std::string before, after;
  Split(style, name, before, after);

  std::stringstream stream;
  stream << before << name << value << ";" << after;
  return stream.str();
}

std::string RegionToPath(const Region& region, const int precision)
{
  std::stringstream stream;
  stream << std::setprecision(precision);

  for (const Polygon_with_holes& polygon : region.shape)
  {
    for
    (
      Polygon::Vertex_const_iterator point_iter = polygon.outer_boundary().vertices_begin();
      point_iter != polygon.outer_boundary().vertices_end();
      ++point_iter
    )
    {
      if (point_iter == polygon.outer_boundary().vertices_begin())
        stream << " M ";
      else
        stream << " L ";

      stream << point_iter->x() << " " << point_iter->y();
    }

    if
    (
      1 < polygon.outer_boundary().size() &&
      polygon.outer_boundary().vertices_begin() != --polygon.outer_boundary().vertices_end()
    )
      stream << " Z";
  }

  if (stream.str().empty()) return "";
  return stream.str().substr(1);
}

std::string SpiralToPath(const Spiral& spiral, const Vector& offset, const int precision, const PolarPoint& parent)
{
  std::stringstream stream;
  stream << std::setprecision(precision);

  const Point anchor = spiral.Evaluate(0).to_cartesian() + offset;
  stream << "M " << anchor.x() << " " << anchor.y();

  if (spiral.angle_rad() != 0)
  {
    for (double t = kSpiralStep; t < kSpiralMax; t += kSpiralStep)
    {
      const PolarPoint polar = spiral.Evaluate(t);
      if (polar.R() <= parent.R())
        break;

      const Point point = polar.to_cartesian() + offset;
      stream << " L " << point.x() << " " << point.y();
    }
  }

  const Point parent_c = parent.to_cartesian() + offset;
  stream << " L " << parent_c.x() << " " << parent_c.y();
  return stream.str();
}

Box Offset(const Box& bounding_box, const Vector& offset)
{
  return Box
  (
    bounding_box.xmin() + offset.x(),
    bounding_box.ymin() + offset.y(),
    bounding_box.xmax() + offset.x(),
    bounding_box.ymax() + offset.y()
  );
}

} // anonymous namespace

namespace flow_map
{
namespace detail
{

/**@class SvgWriter
 * @brief Implementation for writing the flow map to a stream.
 *
 * Note that the actual writing is performed when this object is destroyed. While the object lives, various features can be added to the output.
 */

/**@brief Construct a writer for the flow map.
 * @param context the context regions of the flow map.
 * @param tree the flow tree.
 * @param options the options that influence how and what to write.
 * @param out the destination to write to.
 */
SvgWriter::SvgWriter
(
  const std::vector<Region>& context,
  const std::vector<Region>& obstacles,
  const FlowTree::Ptr& tree,
  const WriteOptions::Ptr& options,
  std::ostream& out
) :
  context_(context),
  obstacles_(obstacles),
  tree_(tree),
  out_(out),
  options_(options)
{
  ComputeBoundingBox();
  OpenSvg();
}

SvgWriter::~SvgWriter()
{
  CloseSvg();

  out_ << printer_.CStr();
}

/**@brief Add the context regions.
 *
 * These are drawn with the same style as the input, with the exception of the opacity. The opacity can either be set to the input opacity, or to some fixed value.
 */
void SvgWriter::DrawContext()
{
  printer_.OpenElement("g");
  printer_.PushComment("Context");

  {
    for (const Region& region : context_)
    {
      if (region.IsPoint())
        continue;

      // Draw the region as a piecewise linear polygon with same style as the input, except the opacity may be adjusted and the color may be changed.
      std::string style = region.style;
      if (0 <= options_->region_opacity)
        style = ForceStyle(style, "fill-opacity:", options_->region_opacity);

      printer_.OpenElement("path");
      printer_.PushAttribute("style", style.c_str());
      printer_.PushAttribute("d", RegionToPath(region, options_->numeric_precision).c_str());
      printer_.PushAttribute("transform", transform_matrix_.c_str());
      printer_.CloseElement(); // path
    }
  }

  printer_.CloseElement(); // g
}


/**@brief Add the obstacle regions.
 *
 * These are drawn with the same style as the input, with the exception of the opacity. The opacity can either be set to the input opacity, or to some fixed value.
 */
void SvgWriter::DrawObstacles()
{
  printer_.OpenElement("g");
  printer_.PushComment("Obstacles");

  {
    //for (const Region& obstacle : obstacles_)  // TODO(tvl) temporary replacement for debugging purposes. In the end, the original obstacles should be shown.
    for (const Region& obstacle : tree_->obstacles_)
    {
      if (obstacle.IsPoint())
        continue;

      // Draw the region as a piecewise linear polygon with same style as the input, except the opacity may be adjusted and the color may be changed.
      std::string style = kObstacleStyle;
      if (0 <= options_->region_opacity)
        style = ForceStyle(style, "fill-opacity:", options_->obstacle_opacity);

      printer_.OpenElement("path");
      printer_.PushAttribute("style", style.c_str());
      printer_.PushAttribute("d", RegionToPath(obstacle, options_->numeric_precision).c_str());
      printer_.PushAttribute("transform", transform_matrix_.c_str());
      printer_.CloseElement(); // path

      DrawObstacleVertices(obstacle);
    }
  }

  printer_.CloseElement(); // g
}

/**@brief Add the flow tree.
 *
 * The flow tree uses a fixed style, with drop-shadows to differentiate it for the underlying geography.
 */
void SvgWriter::DrawFlow()
{
  printer_.OpenElement("g");
  {
    std::stringstream stream;
    stream << "url(#" << kFilterDropShadowId << ")";
    printer_.PushAttribute("filter", stream.str().c_str());
  }
  printer_.PushComment("Flow");

  {
    const Vector offset = -tree_->root_translation_;
    //const FlowTree::Node* prev = &tree_->nodes_.back();
    /*for (const FlowTree::Node& node : tree_->nodes_)
    {
      if (node.type == SpiralTree::Node::Type::kRoot)
        continue;

      DrawSpiral(Spiral(0.61, node.relative_position), offset);
      DrawSpiral(Spiral(-0.61, node.relative_position), offset);

//      PolarPoint source = node.relative_position;
//      PolarPoint target = prev->relative_position;
//      if (source.R() < target.R())
//        std::swap(source, target);
//
//      Spiral spiral(source, target);
//      DrawSpiral(spiral, offset);
//
//      prev = &node;
    }*/

    for (const FlowTree::FlowArc& arc : tree_->arcs_)
    {
      const Spiral& spiral = arc.first;
      const PolarPoint& parent = arc.second;

      DrawSpiral(spiral, offset, parent);
    }
  }

  printer_.CloseElement(); // g
}

/**@brief Add the flow tree nodes.
 */
void SvgWriter::DrawNodes()
{
  printer_.OpenElement("g");
  {
    std::stringstream stream;
    stream << "url(#" << kFilterDropShadowId << ")";
    printer_.PushAttribute("filter", stream.str().c_str());
  }
  printer_.PushComment("Nodes");

  DrawSubdivisionNodes();
  DrawJoinNodes();
  DrawLeaves();
  DrawRoots();

  printer_.CloseElement(); // g
}

void SvgWriter::OpenSvg()
{
  const double width = bounding_box_.xmax() - bounding_box_.xmin();
  const double height = bounding_box_.ymax() - bounding_box_.ymin();
  const int pixel_height(std::ceil(height / unit_px_));

  // Open the SVG element and set its attributes.
  printer_.OpenElement("svg");
  printer_.PushAttribute("xmlns", "http://www.w3.org/2000/svg");
  printer_.PushAttribute("xmlns:xlink", "http://www.w3.org/1999/xlink");
  printer_.PushAttribute("version", kSvgVersion);
  printer_.PushAttribute("width", options_->pixel_width);
  printer_.PushAttribute("height", pixel_height);

  {
    // Set the viewbox.
    std::stringstream stream;
    stream << "0 0 " << width << " " << height;
    printer_.PushAttribute("viewBox", stream.str().c_str());
  }

  {
    // Set the (custom) bounds attribute to indicate to the website in which region in the world to place the geometry.
    // Note, the bounds are expected in latitude-longitude.
    std::stringstream stream;
    stream <<
           "[" <<
           "[" << bounding_box_.ymin() << "," << bounding_box_.xmin() << "]," <<
           "[" << bounding_box_.ymax() << "," << bounding_box_.xmax() << "]" <<
           "]";
    printer_.PushAttribute(kSvgAttributeBounds, stream.str().c_str());
  }

  {
    // Set the transform matrix to apply to the world geometry.
    std::stringstream stream;
    stream <<
           "matrix(" <<
           kTransformScale << ",0," <<
           "0," << -kTransformScale << "," <<
           -kTransformScale * bounding_box_.xmin() << "," << kTransformScale * bounding_box_.ymax() <<
           ")";
    transform_matrix_ = stream.str();
  }

  AddDropShadowFilter();
}

void SvgWriter::CloseSvg()
{
  // Add hint to display when the geometry could not be drawn.
  printer_.PushText("Sorry, your browser does not support the svg tag.");
  printer_.CloseElement(); // svg
}

void SvgWriter::ComputeBoundingBox()
{
  // Add the regions to the bounding box.
  for (const Region& region : context_)
    for (const Polygon_with_holes& polygon : region.shape)
      bounding_box_ += polygon.bbox();

  const Vector offset = -tree_->root_translation_;

  // Add the flow tree to the bounding box.
//  for (const Spiral& spiral : tree_->spirals_)
//    bounding_box_spirals_ += Offset(spiral.ComputeBoundingBox(), offset);
//  bounding_box_ += bounding_box_spirals_;

  // Add the nodes to the bounding box.
  for (const Node::Ptr& node : tree_->nodes_)
  {
    Point center = node->place->position.to_cartesian();
    const Number& radius = 5;

    const Box bead_box = GrowBoundingBox(center, radius);
    bounding_box_ += bead_box;
  }

  // Add a small buffer around the bounding box.
  const Number buffer = kBoundingBoxBufferPx * (bounding_box_.xmax() - bounding_box_.xmin()) / options_->pixel_width;
  bounding_box_ = GrowBoundingBox(bounding_box_, buffer);
  unit_px_ = (bounding_box_.xmax() - bounding_box_.xmin()) / options_->pixel_width;
}

void SvgWriter::AddDropShadowFilter()
{
  printer_.OpenElement("defs");
  printer_.OpenElement("filter");
  printer_.PushAttribute("id", kFilterDropShadowId);
  printer_.PushAttribute("filterUnits", "userSpaceOnUse");

  {
    {
      // Define the color of the drop shadow.
      const double multiply_alpha = options_->flow_opacity < 0 ? 1.0 : kDropShadowShade * options_->flow_opacity;
      std::stringstream stream;
      stream << "0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 " << multiply_alpha << " 0";

      printer_.OpenElement("feColorMatrix");
      printer_.PushAttribute("in", "SourceAlpha");
      printer_.PushAttribute("type", "matrix");
      printer_.PushAttribute("values", stream.str().c_str());
      printer_.PushAttribute("result", "sourceOblique");
      printer_.CloseElement(); // feColorMatrix
    }

    {
      // The drop shadow is in essence a blur effect with an offset.
      const double extent = kDropShadowExtentPx * unit_px_;

      std::stringstream stream;
      stream << extent;
      const std::string blur = stream.str();
      const std::string dx = blur;
      const std::string dy = blur;

      printer_.OpenElement("feGaussianBlur");
      printer_.PushAttribute("in", "sourceOblique");
      printer_.PushAttribute("stdDeviation", blur.c_str());
      printer_.PushAttribute("result", "blur");
      printer_.CloseElement(); // feGaussianBlur

      printer_.OpenElement("feOffset");
      printer_.PushAttribute("in", "blur");
      printer_.PushAttribute("dx", dx.c_str());
      printer_.PushAttribute("dy", dy.c_str());
      printer_.PushAttribute("result", "offsetBlur");
      printer_.CloseElement(); // feOffset

      printer_.OpenElement("feComposite");
      printer_.PushAttribute("in", "offsetBlur");
      printer_.PushAttribute("in2", "sourceOblique");
      printer_.PushAttribute("operator", "xor");
      printer_.PushAttribute("result", "dropShadow");
      printer_.CloseElement(); // feComposite
    }

    const bool force_opaque = options_->flow_opacity < 1;
    if (force_opaque)
    {
      // Subtract the original from the shadow.
      // This used an combination of two inputs according to the formula k1 * in * in2 + k2 * in + k3 * in2 + k4.
      printer_.OpenElement("feComposite");
      printer_.PushAttribute("in", "sourceOblique");
      printer_.PushAttribute("in2", "dropShadow");
      printer_.PushAttribute("operator", "arithmetic");
      printer_.PushAttribute("k1", "0");
      printer_.PushAttribute("k2", "-1");
      printer_.PushAttribute("k3", "1");
      printer_.PushAttribute("k4", "0");
      printer_.PushAttribute("result", "dropShadowMasked");
      printer_.CloseElement(); // feComposite
    }

    {
      // Merge the drop shadow and original.
      printer_.OpenElement("feMerge");
      printer_.OpenElement("feMergeNode");
      if (force_opaque)
        printer_.PushAttribute("in", "dropShadow");
      else
        printer_.PushAttribute("in", "dropShadowMasked");
      printer_.CloseElement(); // feMergeNode
      printer_.OpenElement("feMergeNode");
      printer_.PushAttribute("in", "SourceGraphic");
      printer_.CloseElement(); // feMergeNode
      printer_.CloseElement(); // feMerge
    }
  }

  printer_.CloseElement(); // filter
  printer_.CloseElement(); // defs
}

void SvgWriter::DrawSpiral(const Spiral& spiral, const Vector& offset, const PolarPoint& parent)
{
  std::string style = kFlowStyle;

  if (0 <= options_->flow_opacity)
    style = ForceStyle(style, "fill-opacity:", options_->flow_opacity);
  if (0 <= options_->flow_opacity && options_->flow_opacity < 1)
    style = ForceStyle(style, "stroke-width:", 0);
  else
    style = ForceStyle(style, "stroke-width:", kLineWidthPx * unit_px_);

  if (false)
  {
    const Number hue = spiral.ComputePhi(1);

    std::stringstream stream;
    stream << "hsla(" << hue << "rad,100%,50%,100%)";
    style = ForceStyle(style, "stroke:", stream.str());
  }

  printer_.OpenElement("path");
  printer_.PushAttribute("style", style.c_str());
  printer_.PushAttribute("d", SpiralToPath(spiral, offset, options_->numeric_precision, parent).c_str());
  printer_.PushAttribute("transform", transform_matrix_.c_str());
  printer_.CloseElement(); // path
}

void SvgWriter::DrawRoots()
{
  for (const Node::Ptr& node : tree_->nodes_)
  {
    if (node->GetType() != Node::ConnectionType::kRoot)
      continue;

    printer_.OpenElement("path");

    {
      std::string style = kRootStyle;
      style = ForceStyle(style, "fill-opacity:", options_->node_opacity);
      if (0 <= options_->node_opacity && options_->node_opacity < 1)
        style = ForceStyle(style, "stroke-width:", 0);
      else
        style = ForceStyle(style, "stroke-width:", kLineWidthPx * unit_px_);

      printer_.PushAttribute("style", style.c_str());
    }

    {
      Kernel::Construct_bbox_2 bbox = Kernel().construct_bbox_2_object();
      Box bounding_box = bbox(Point(CGAL::ORIGIN) - tree_->root_translation_);

      const Number extend = kRootWidthPx * 0.5 * unit_px_;
      bounding_box = GrowBoundingBox(bounding_box, extend);

      Polygon box;
      box.push_back(Point(bounding_box.xmin(), bounding_box.ymin()));
      box.push_back(Point(bounding_box.xmax(), bounding_box.ymin()));
      box.push_back(Point(bounding_box.xmax(), bounding_box.ymax()));
      box.push_back(Point(bounding_box.xmin(), bounding_box.ymax()));

      Region region("root");
      region.shape.emplace_back(box);

      printer_.PushAttribute("d", RegionToPath(region, options_->numeric_precision).c_str());
    }

    printer_.PushAttribute("transform", transform_matrix_.c_str());
    printer_.CloseElement(); // path
  }
}

void SvgWriter::DrawLeaves()
{
  for (const Node::Ptr& node : tree_->nodes_)
  {
    if (node->GetType() == Node::ConnectionType::kRoot || node->IsSteiner())
      continue;

    printer_.OpenElement("circle");

    {
      std::string style = kLeafStyle;
      style = ForceStyle(style, "fill-opacity:", options_->node_opacity);
      if (0 <= options_->node_opacity && options_->node_opacity < 1)
        style = ForceStyle(style, "stroke-width:", 0);
      else
        style = ForceStyle(style, "stroke-width:", kLineWidthPx * unit_px_);

      printer_.PushAttribute("style", style.c_str());
    }

    {
      const Point position = node->place->position.to_cartesian();
      printer_.PushAttribute("cx", position.x());
      printer_.PushAttribute("cy", position.y());
    }

    {
      std::stringstream stream;
      const Number radius = kLeafRadiusPx * unit_px_;
      stream << radius;
      printer_.PushAttribute("r", stream.str().c_str());
    }

    printer_.PushAttribute("transform", transform_matrix_.c_str());
    printer_.CloseElement(); // circle
  }
}

void SvgWriter::DrawJoinNodes()
{
  for (const Node::Ptr& node : tree_->nodes_)
  {
    if (node->GetType() != Node::ConnectionType::kJoin || !node->IsSteiner())
      continue;

    printer_.OpenElement("circle");

    {
      std::string style = kJoinStyle;
      style = ForceStyle(style, "fill-opacity:", options_->node_opacity);
      if (0 <= options_->node_opacity && options_->node_opacity < 1)
        style = ForceStyle(style, "stroke-width:", 0);
      else
        style = ForceStyle(style, "stroke-width:", kLineWidthPx * unit_px_);

      printer_.PushAttribute("style", style.c_str());
    }

    {
      const Point position = node->place->position.to_cartesian();
      printer_.PushAttribute("cx", position.x());
      printer_.PushAttribute("cy", position.y());
    }

    {
      std::stringstream stream;
      const Number radius = kJoinRadiusPx * unit_px_;
      stream << radius;
      printer_.PushAttribute("r", stream.str().c_str());
    }

    printer_.PushAttribute("transform", transform_matrix_.c_str());
    printer_.CloseElement(); // circle
  }
}

void SvgWriter::DrawSubdivisionNodes()
{
  for (const Node::Ptr& node : tree_->nodes_)
  {
    if (node->GetType() != Node::ConnectionType::kSubdivision || !node->IsSteiner())
      continue;

    printer_.OpenElement("circle");

    {
      std::string style = kSubdivisionStyle;
      style = ForceStyle(style, "fill-opacity:", options_->node_opacity);
      if (0 <= options_->node_opacity && options_->node_opacity < 1)
        style = ForceStyle(style, "stroke-width:", 0);
      else
        style = ForceStyle(style, "stroke-width:", kLineWidthPx * unit_px_);

      printer_.PushAttribute("style", style.c_str());
    }

    {
      const Point position = node->place->position.to_cartesian();
      printer_.PushAttribute("cx", position.x());
      printer_.PushAttribute("cy", position.y());
    }

    {
      std::stringstream stream;
      const Number radius = kSubdivisionRadiusPx * unit_px_;
      stream << radius;
      printer_.PushAttribute("r", stream.str().c_str());
    }

    printer_.PushAttribute("transform", transform_matrix_.c_str());
    printer_.CloseElement(); // circle
  }
}

void SvgWriter::DrawObstacleVertices(const Region& obstacle)
{
  for (const Polygon_with_holes& polygon : obstacle.shape)
  {
    DrawObstacleVertices(polygon.outer_boundary());

    for (Polygon_with_holes::Hole_const_iterator hole_iter = polygon.holes_begin(); hole_iter != polygon.holes_end(); ++hole_iter)
      DrawObstacleVertices(*hole_iter);
  }
}

void SvgWriter::DrawObstacleVertices(const Polygon& polygon)
{
  for (Polygon::Vertex_const_iterator vertex_iter = polygon.vertices_begin(); vertex_iter != polygon.vertices_end(); ++vertex_iter)
  {
    printer_.OpenElement("circle");
    printer_.PushAttribute("style", kVertexStyle);

    {
      const Point& position = *vertex_iter;
      printer_.PushAttribute("cx", position.x());
      printer_.PushAttribute("cy", position.y());
    }

    {
      std::stringstream stream;
      const Number radius = kVertexRadiusPx * unit_px_;
      stream << radius;
      printer_.PushAttribute("r", stream.str().c_str());
    }

    printer_.PushAttribute("transform", transform_matrix_.c_str());
    printer_.CloseElement(); // circle
  }
}

} // namespace detail
} // namespace flow_map
} // namespace geoviz
