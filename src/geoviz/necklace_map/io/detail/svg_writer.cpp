/*
The Necklace Map console application implements the algorithmic
geo-visualization method by the same name, developed by
Bettina Speckmann and Kevin Verbeek at TU Eindhoven
(DOI: 10.1109/TVCG.2010.180 & 10.1142/S021819591550003X).
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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 29-01-2020
*/

#include "svg_writer.h"

#include <sstream>

#include <glog/logging.h>

#include "geoviz/common/bounding_box.h"


namespace geoviz
{
namespace
{

constexpr const char* kSvgVersion = "1.1";
constexpr const char* kSvgAttributeBounds = "bounds";

constexpr const char* kFilterDropShadowId = "filterDropShadow";

constexpr const char* kNecklaceStyle = "fill:none;"
                                       "stroke:rgba(0%,0%,0%,100%);"
                                       "stroke-linecap:butt;"
                                       "stroke-linejoin:round;";
constexpr const char* kNecklaceKernelStyle = "fill:rgba(0%,0%,0%,100%);"
                                             "stroke:rgba(0%,0%,0%,100%);"
                                             "stroke-linecap:butt;"
                                             "stroke-linejoin:round;";
constexpr const char* kRegionContextColor = "white";
constexpr const char* kRegionUnusedColor = "rgb(90%,90%,90%)";
constexpr const char* kBeadIdFontFamily = "Verdana";

constexpr const char* kFeasibleIntervalStyle = "fill:none;"
                                               "stroke-linecap:butt;"
                                               "stroke-opacity:1;";
constexpr const char* kValidIntervalStyle = "fill:none;"
                                            "stroke-linecap:butt;"
                                            "stroke-linejoin:round;";
constexpr const char* kRegionAngleStyle = "fill:none;"
                                          "stroke:rgba(20%,20%,20%,70%);"
                                          "stroke-linecap:butt;";
constexpr const char* kBeadAngleStyle = "fill:none;"
                                        "stroke:rgba(0%,0%,0%,100%);"
                                        "stroke-linecap:butt;";

constexpr char kAbsoluteMove = 'M';
constexpr char kAbsoluteCubicBezier = 'C';
constexpr char kAbsoluteClose = 'Z';


// Note that this source file contains string literals in various other places.
// However, it is likely that whenever these have to change, detailed knowledge of the SVG file structure is required.
// In this case, you will have to dive into the code anyway.

constexpr const double kTransformScale = 1;

constexpr const double kBoundingBoxBufferPx = 5;

constexpr const double kLineWidthPx = 1.7;

constexpr const double kPointRegionRadiusPx = 3;

constexpr const int kIntervalNumericPrecision = 5;
constexpr const double kValidIntervalOpacity = 0.7;
constexpr const double kDebugLineWidthRatio = 0.5;

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


class NecklaceIntervalVisitor : public necklace_map::NecklaceShapeVisitor
{
 public:
  NecklaceIntervalVisitor() {}

  const necklace_map::CircleNecklace::Ptr& interval_shape() { return interval_shape_; }

  void Visit(necklace_map::CircleNecklace& shape)
  {
    interval_shape_ = std::make_shared<necklace_map::CircleNecklace>(shape);
  }

  void Visit(necklace_map::BezierNecklace& shape)
  {
    // The interval shape is constructed as the circle centered on the necklace kernel and fully inside the bounding box (by some margin).
    const Point& kernel = shape.kernel();
    const Box bounding_box = shape.ComputeBoundingBox();
    CHECK_LE(bounding_box.xmin(), kernel.x());
    CHECK_LE(kernel.x(), bounding_box.xmax());
    CHECK_LE(bounding_box.ymin(), kernel.y());
    CHECK_LE(kernel.y(), bounding_box.ymax());

    const Number radius = 0.9 * std::min
    (
      std::min(kernel.x() - bounding_box.xmin(), bounding_box.xmax() - kernel.x()),
      std::min(kernel.y() - bounding_box.ymin(), bounding_box.ymax() - kernel.y())
    );

    interval_shape_ = std::make_shared<necklace_map::CircleNecklace>(Circle(kernel, radius * radius));
  }

 private:
  necklace_map::CircleNecklace::Ptr interval_shape_;
}; // class NecklaceIntervalVisitor


class DrawNecklaceShapeVisitor : public necklace_map::BezierNecklaceVisitor
{
 public:
  DrawNecklaceShapeVisitor(const std::string& necklace_style, const std::string& transform_matrix, tinyxml2::XMLPrinter& printer) :
    necklace_style_(necklace_style), transform_matrix_(transform_matrix), printer_(printer)
  {}

  const necklace_map::Necklace::Ptr& necklace() const { return necklace_; }
  necklace_map::Necklace::Ptr& necklace() { return necklace_; }

  void Visit(necklace_map::CircleNecklace& shape) override
  {
    CHECK_NOTNULL(necklace_);

    const Point& kernel = shape.kernel();
    const Number radius = shape.ComputeRadius();

    printer_.OpenElement("circle");
    printer_.PushAttribute("style", necklace_style_.c_str());
    printer_.PushAttribute("cx", kernel.x());
    printer_.PushAttribute("cy", kernel.y());
    printer_.PushAttribute("r", radius);
    printer_.PushAttribute("transform", transform_matrix_.c_str());
    printer_.PushAttribute("necklace_id", necklace_->id.c_str());
    printer_.CloseElement();  // circle
  }

  void Visit(necklace_map::BezierNecklace& shape) override
  {
    CHECK_NOTNULL(necklace_);

    printer_.OpenElement("path");
    printer_.PushAttribute("style", necklace_style_.c_str());

    path_.clear();
    shape.IterateCurves(*this);
    path_ <<
      " " << kAbsoluteClose <<
      " " << kAbsoluteMove << start_point_;
    printer_.PushAttribute("d", path_.str().c_str());

    printer_.PushAttribute("kx", necklace_->shape->kernel().x());
    printer_.PushAttribute("ky", necklace_->shape->kernel().y());

    printer_.PushAttribute("transform", transform_matrix_.c_str());
    printer_.PushAttribute("necklace_id", necklace_->id.c_str());
    printer_.CloseElement();  // path
  }

  void Visit(necklace_map::BezierCurve& curve) override
  {
    if (path_.str().empty())
    {
      start_point_ = curve.source();
      path_ << kAbsoluteMove << " " << start_point_;
    }

    path_ <<
      " " << kAbsoluteCubicBezier <<
      " " << curve.source_control() <<
      " " << curve.target_control() <<
      " " << curve.target();
  }

 private:
  std::stringstream path_;
  Point start_point_;

  necklace_map::Necklace::Ptr necklace_;

  const std::string& necklace_style_;
  const std::string& transform_matrix_;

  tinyxml2::XMLPrinter& printer_;
}; // class DrawNecklaceShapeVisitor

} // anonymous namespace

namespace necklace_map
{

WriterOptions::Ptr WriterOptions::Default()
{
  WriterOptions::Ptr options = std::make_shared<WriterOptions>();

  options->pixel_width = 500;

  options->region_precision = 9;
  options->region_opacity = -1;
  options->bead_opacity = 1;
  options->bead_id_font_size_px = 16;

  options->draw_necklace_curve = true;
  options->draw_necklace_kernel = false;
  options->draw_bead_ids = true;

  options->draw_feasible_intervals = false;
  options->draw_valid_intervals = false;
  options->draw_region_angles = false;
  options->draw_bead_angles = false;

  return options;
}

WriterOptions::Ptr WriterOptions::Debug()
{
  WriterOptions::Ptr options = std::make_shared<WriterOptions>();

  options->pixel_width = 500;

  options->region_precision = 9;
  options->region_opacity = -1;
  options->bead_opacity = 0.5;
  options->bead_id_font_size_px = 16;

  options->draw_necklace_curve = true;
  options->draw_necklace_kernel = true;
  options->draw_bead_ids = true;

  options->draw_feasible_intervals = true;
  options->draw_valid_intervals = true;
  options->draw_region_angles = false;
  options->draw_bead_angles = true;

  return options;
}


namespace detail
{

/**@class SvgWriter
 * @brief Implementation for writing the necklace map to a stream.
 *
 * Note that the actual writing is performed when this object is destroyed. While the object lives, various features can be added to the output.
 */

/**@brief Construct a writer for the necklace map.
 * @param elements the elements of the necklace map.
 * @param necklaces the necklaces of the map.
 * @param scale_factor the factor by which to scale the necklace beads.
 * @param options the options that influence how and what to write.
 * @param out the destination to write to.
 */
SvgWriter::SvgWriter
  (
    const std::vector<MapElement::Ptr>& elements,
    const std::vector<Necklace::Ptr>& necklaces,
    const Number& scale_factor,
    const WriterOptions::Ptr& options,
    std::ostream& out
  ) :
  elements_(elements),
  necklaces_(necklaces),
  scale_factor_(scale_factor),
  out_(out),
  options_(options)
{
  ComputeBoundingBox();
  CreateBeadIntervalShapes();
  OpenSvg();
}

SvgWriter::~SvgWriter()
{
  CloseSvg();

  out_ << printer_.CStr();
}

/**@brief Add the regions with polygonal shape.
 *
 * These are drawn with the same style as the input, with the exception of the opacity. The opacity can either be set to the input opacity, or to some fixed value.
 */
void SvgWriter::DrawPolygonRegions()
{
  printer_.OpenElement("g");
  printer_.PushComment("Regions");

  {
    for (const MapElement::Ptr& element : elements_)
    {
      const Region& region = element->region;
      if (region.IsPoint())
        continue;

      // Draw the region with the region as a piecewise linear polygon with same style as the input, except the opacity may be adjusted and the color may be changed.
      std::string style = region.style;
      if (0 <= options_->region_opacity)
        style = ForceStyle(style, "fill-opacity:", options_->region_opacity);
      if (!element->necklace)
        style = ForceStyle(style, "fill:", kRegionContextColor);
      else if (element->value <= 0)
        style = ForceStyle(style, "fill:", kRegionUnusedColor);

      const std::string necklace_id = element->necklace ? element->necklace->id : "";

      printer_.OpenElement("path");
      printer_.PushAttribute("style", style.c_str());
      printer_.PushAttribute("d", RegionToPath(region, options_->region_precision).c_str());
      printer_.PushAttribute("transform", transform_matrix_.c_str());

      if (element->bead)
      {
        printer_.PushAttribute("angle_rad", element->bead->angle_rad);

        if (element->bead->feasible)
        {
          std::stringstream stream;
          stream << element->bead->feasible->from_rad() << " " << element->bead->feasible->to_rad();
          printer_.PushAttribute("feasible", stream.str().c_str());
        }
      }

      printer_.PushAttribute("region_id", region.id.c_str());
      printer_.PushAttribute("necklace_id", necklace_id.c_str());
      printer_.CloseElement(); // path
    }
  }

  printer_.CloseElement(); // g
}

/**@brief Add the regions with point shape.
 *
 * These are drawn with the same style as the input, with the exception of the opacity. The opacity can either be set to the input opacity, or to some fixed value.
 */
void SvgWriter::DrawPointRegions()
{
  printer_.OpenElement("g");
  printer_.PushComment("Point Regions");

  {
    for (const MapElement::Ptr& element : elements_)
    {
      const Region& region = element->region;
      if (!region.IsPoint())
        continue;

      // Draw the region with the region as a circle with same style as the input, except the opacity may be adjusted and the color may be changed.
      const Point& position = region.shape[0].outer_boundary()[0];
      std::string style = region.style;
      if (0 <= options_->region_opacity)
        style = ForceStyle(style, "fill-opacity:", options_->region_opacity);
      if (!element->necklace)
        style = ForceStyle(style, "fill:", kRegionContextColor);
      else if (element->value <= 0)
        style = ForceStyle(style, "fill:", kRegionUnusedColor);

      printer_.OpenElement("circle");
      printer_.PushAttribute("style", style.c_str());

      {
        std::stringstream stream;
        stream << std::setprecision(options_->region_precision);
        stream << position.x();
        printer_.PushAttribute("cx", stream.str().c_str());
      }
      {
        std::stringstream stream;
        stream << std::setprecision(options_->region_precision);
        stream << position.y();
        printer_.PushAttribute("cy", stream.str().c_str());
      }
      {
        std::stringstream stream;
        stream << std::setprecision(kIntervalNumericPrecision);
        const Number radius = kPointRegionRadiusPx * unit_px_;
        stream << radius;
        printer_.PushAttribute("r", stream.str().c_str());
      }

      const std::string necklace_id = element->necklace ? element->necklace->id : "";

      printer_.PushAttribute("transform", transform_matrix_.c_str());

      if (element->bead)
      {
        printer_.PushAttribute("angle_rad", element->bead->angle_rad);

        if (element->bead->feasible)
        {
          std::stringstream stream;
          stream << element->bead->feasible->from_rad() << " " << element->bead->feasible->to_rad();
          printer_.PushAttribute("feasible", stream.str().c_str());
        }
      }

      printer_.PushAttribute("region_id", region.id.c_str());
      printer_.PushAttribute("necklace_id", necklace_id.c_str());
      printer_.CloseElement(); // circle
    }
  }

  printer_.CloseElement(); // g
}

/**@brief Add the necklace curves.
 *
 * The necklace curves are always drawn as a solid black curve.
 */
void SvgWriter::DrawNecklaces()
{
  if (!options_->draw_necklace_curve)
    return;

  printer_.OpenElement("g");
  printer_.PushComment("Necklaces");

  std::string style = kNecklaceStyle;
  style = ForceStyle(style, "stroke-width:", kLineWidthPx * unit_px_);

  // How to draw each necklace depends on the necklace shape type.
  // We use a visitor pattern to overcome this ambiguity.
  DrawNecklaceShapeVisitor draw_visitor(style, transform_matrix_, printer_);
  for (const Necklace::Ptr& necklace : necklaces_)
  {
    draw_visitor.necklace() = necklace;

    printer_.OpenElement("g");
    necklace->shape->Accept(draw_visitor);
    DrawKernel(necklace->shape->kernel());
    printer_.CloseElement(); // g
  }

  printer_.CloseElement(); // g
}

/**@brief Add the necklace beads.
 *
 * The necklace beads use mostly the same style as the regions, with drop-shadows to differentiate them for the underlying geography.
 * However, they can be forced to be semi-transparant. This also influences how their drop shadows and their borders are drawn.
 */
void SvgWriter::DrawBeads()
{
  if (scale_factor_ == 0)
    return;

  printer_.OpenElement("g");
  {
    std::stringstream stream;
    stream << "url(#" << kFilterDropShadowId << ")";
    printer_.PushAttribute("filter", stream.str().c_str());
  }
  printer_.PushComment("Beads");

  {
    // Note these are drawn per necklace as opposed to per element.
    for (const Necklace::Ptr& necklace : necklaces_)
    {
      for (const Bead::Ptr& bead : necklace->beads)
      {
        if (!bead->valid)
          continue;

        printer_.OpenElement("circle");
        {
          const std::string base_style = ForceStyle(bead->region_style, "fill-opacity:", options_->bead_opacity);
          const std::string bead_style =
            (0 <= options_->bead_opacity && options_->bead_opacity < 1)
            ? ForceStyle(base_style, "stroke-width:", 0)
            : base_style;

          printer_.PushAttribute("style", bead_style.c_str());
        }
        {
          Point position;
          CHECK(necklace->shape->IntersectRay(bead->angle_rad, position));
          printer_.PushAttribute("cx", position.x());
          printer_.PushAttribute("cy", position.y());
        }
        {
          const Number radius = scale_factor_ * bead->radius_base;
          printer_.PushAttribute("r", radius);
        }
        printer_.PushAttribute("transform", transform_matrix_.c_str());
        printer_.CloseElement(); // circle
      }
    }
  }

  printer_.CloseElement(); // g

  DrawBeadIds();
}

/**@brief Add the feasible intervals.
 *
 * The feasible intervals are drawn as non-overlapping circular arcs with their color matching the interior color of the regions.
 */
void SvgWriter::DrawFeasibleIntervals()
{
  if (!options_->draw_feasible_intervals)
    return;

  printer_.OpenElement("g");
  printer_.PushComment("Feasible Intervals");

  {
    // Note these are drawn per necklace as opposed to per element.
    for (const Necklace::Ptr& necklace : necklaces_)
    {
      for (const Bead::Ptr& bead : necklace->beads)
      {
        if (!bead->feasible)
          continue;

        CircleNecklace::Ptr interval_shape = bead_interval_map_[bead];
        CHECK_NOTNULL(interval_shape);

        printer_.OpenElement("path");
        {
          // The color of the interval is based on the region color.
          const std::string& style_region = bead->region_style;
          std::string color;
          GetStyle(style_region, "fill:", color);

          std::string style_interval = kFeasibleIntervalStyle;
          style_interval = ForceStyle(style_interval, "stroke-width:", kLineWidthPx * unit_px_);
          style_interval = ForceStyle(style_interval, "stroke:", color);
          printer_.PushAttribute("style", style_interval.c_str());
        }
        {
          // Draw the feasible interval as a circular path.
          const Number& from_rad = bead->feasible->from_rad();
          const Number& to_rad = bead->feasible->to_rad();
          const int large_arc_flag = M_PI < (to_rad - from_rad) ? 1 : 0;

          Point endpoint_cw, endpoint_ccw;
          CHECK(interval_shape->IntersectRay(from_rad, endpoint_cw));
          CHECK(interval_shape->IntersectRay(to_rad, endpoint_ccw));
          const Number radius = interval_shape->ComputeRadius();

          std::stringstream stream;
          stream << std::setprecision(kIntervalNumericPrecision);
          stream <<
                 "M " << endpoint_cw.x() << " " << endpoint_cw.y() <<
                 " A " << radius << " " << radius << " 0 " << large_arc_flag << " 1 " <<
                 endpoint_ccw.x() << " " << endpoint_ccw.y();

          printer_.PushAttribute("d", stream.str().c_str());
        }

        printer_.PushAttribute("transform", transform_matrix_.c_str());
        printer_.CloseElement(); // path
      }
    }
  }

  printer_.CloseElement(); // g
}

/**@brief Add the valid intervals.
 *
 * The valid intervals are drawn as wedges with their stroke color matching the interior color of the regions.
 *
 * If the feasible regions are also drawn, the valid intervals extend to their corresponding feasible interval. Otherwise, they extend to the necklace curve.
 */
void SvgWriter::DrawValidIntervals()
{
  if (!options_->draw_valid_intervals)
    return;

  printer_.OpenElement("g");
  printer_.PushComment("Valid Intervals");

  {
    // Note these are drawn per necklace as opposed to per element.
    for (const Necklace::Ptr& necklace : necklaces_)
    {
      for (const Bead::Ptr& bead : necklace->beads)
      {
        if (!bead->valid)
          continue;

        CircleNecklace::Ptr interval_shape = bead_interval_map_[bead];
        CHECK_NOTNULL(interval_shape);

        printer_.OpenElement("path");
        {
          // The color of the interval is based on the region color.
          const std::string& style_region = bead->region_style;
          std::string color;
          GetStyle(style_region, "fill:", color);

          std::string style_interval = kValidIntervalStyle;
          style_interval = ForceStyle(style_interval, "stroke-width:", kDebugLineWidthRatio * kLineWidthPx * unit_px_);
          style_interval = ForceStyle(style_interval, "stroke:", color);
          style_interval = ForceStyle(style_interval, "stroke-opacity:", kValidIntervalOpacity);
          printer_.PushAttribute("style", style_interval.c_str());
        }
        {
          // Draw the valid interval as a wedge from the necklace kernel to either the necklace, or the feasible interval if it is also drawn.
          const Number& from_rad = bead->valid->from_rad();
          const Number& to_rad = bead->valid->to_rad();

          Point endpoint_cw, endpoint_ccw;
          CHECK(interval_shape->IntersectRay(from_rad, endpoint_cw));
          CHECK(interval_shape->IntersectRay(to_rad, endpoint_ccw));

          std::stringstream stream;
          stream << std::setprecision(kIntervalNumericPrecision);
          stream <<
                 "M " << endpoint_cw.x() << " " << endpoint_cw.y() <<
                 " L " << interval_shape->kernel().x() << " " << interval_shape->kernel().y() <<
                 " L " << endpoint_ccw.x() << " " << endpoint_ccw.y();

          printer_.PushAttribute("d", stream.str().c_str());
        }

        printer_.PushAttribute("transform", transform_matrix_.c_str());
        printer_.CloseElement(); // path
      }
    }
  }

  printer_.CloseElement(); // g
}

/**@brief Add line segments connecting the necklace kernel(s) with the region centroids.
 *
 * These line segments are always colored gray.
 *
 * If the feasible regions are also drawn, the region angles extend to their corresponding feasible interval. Otherwise, they extend to the necklace curve.
 */
void SvgWriter::DrawRegionAngles()
{
  if (!options_->draw_region_angles)
    return;

  printer_.OpenElement("g");
  printer_.PushComment("Region Centroids");

  {
    // Note these are draw per element, because these reference the region.
    ComputeCentroid compute_centroid;
    for (const MapElement::Ptr& element : elements_)
    {
      if (!element->necklace)
        continue;

      Polygon simple;
      element->region.MakeSimple(simple);
      const Point centroid = compute_centroid(simple);

      if (!element->necklace || !element->bead || !element->bead->valid)
        continue;

      printer_.OpenElement("path");

      std::string style = kRegionAngleStyle;
      style = ForceStyle(style, "stroke-width:", kDebugLineWidthRatio * kLineWidthPx * unit_px_);
      printer_.PushAttribute("style", style.c_str());

      {
        CircleNecklace::Ptr interval_shape = bead_interval_map_[element->bead];
        CHECK_NOTNULL(interval_shape);

        const Point& kernel = interval_shape->kernel();
        const Number angle_centroid_rad = interval_shape->ComputeAngleRad(centroid);
        Point endpoint;
        CHECK(interval_shape->IntersectRay(angle_centroid_rad, endpoint));

        std::stringstream stream;
        stream << std::setprecision(kIntervalNumericPrecision);
        stream << "M " << kernel.x() << " " << kernel.y() << " L " << endpoint.x() << " " << endpoint.y();

        printer_.PushAttribute("d", stream.str().c_str());
      }
      printer_.PushAttribute("transform", transform_matrix_.c_str());
      printer_.CloseElement(); // path

    }
  }

  printer_.CloseElement(); // g
}

/**@brief Add line segments connecting the necklace kernel(s) with the bead centers.
 *
 * These line segments are always colored black.
 */
void SvgWriter::DrawBeadAngles()
{
  if (!options_->draw_bead_angles)
    return;

  printer_.OpenElement("g");
  printer_.PushComment("Bead Angles");

  {
    // Note these are drawn per necklace as opposed to per element.
    for (const Necklace::Ptr& necklace : necklaces_)
    {
      const Point& kernel = necklace->shape->kernel();

      size_t count = 1;
      for (const Bead::Ptr& bead : necklace->beads)
      {
        if (!bead->valid)
          continue;

        printer_.OpenElement("path");

        std::string style = kBeadAngleStyle;
        style = ForceStyle(style, "stroke-width:", kDebugLineWidthRatio * kLineWidthPx * unit_px_);
        printer_.PushAttribute("style", style.c_str());

        {
          Point endpoint;
          CHECK(necklace->shape->IntersectRay(bead->angle_rad, endpoint));

          std::stringstream stream;
          stream << std::setprecision(kIntervalNumericPrecision);
          stream << "M " << kernel.x() << " " << kernel.y() << " L " << endpoint.x() << " " << endpoint.y();

          printer_.PushAttribute("d", stream.str().c_str());
        }
        printer_.PushAttribute("transform", transform_matrix_.c_str());
        printer_.CloseElement(); // path
      }
    }
  }

  printer_.CloseElement(); // g
}

void SvgWriter::OpenSvg()
{
  // TODO(tvl) add note to documentation that we explicitly do not claim any copyright over the output of the system, with the intention that the user is able to reserve any rights for themselves.

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

  // Store the scale factor.
  printer_.PushAttribute("scale_factor", scale_factor_);

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
  for (const MapElement::Ptr& element : elements_)
    for (const Polygon_with_holes& polygon : element->region.shape)
      bounding_box_ += polygon.bbox();

  if
    (
    options_->draw_necklace_curve ||
    options_->draw_feasible_intervals ||
    options_->draw_valid_intervals ||
    options_->draw_region_angles
    )
  {
    // Add the necklace curves to the bounding box.
    for (const Necklace::Ptr& necklace : necklaces_)
    {
      const Box necklace_box = necklace->shape->ComputeBoundingBox();
      bounding_box_ += necklace_box;

      // The feasible interval is drawn on a circle that does not overlap with the necklace (if it is a circle or curve).
      if (options_->draw_feasible_intervals)
      {
        const Point& kernel = necklace->shape->kernel();
        const Number max_side_distance = std::max
          (
            std::max(kernel.x() - necklace_box.xmin(), necklace_box.xmax() - kernel.x()),
            std::max(kernel.y() - necklace_box.ymin(), necklace_box.ymax() - kernel.y())
          );

        const double buffer = kLineWidthPx * (necklace->beads.size() + 1);
        const Box feasible_box = GrowBoundingBox(kernel, max_side_distance + buffer);

        bounding_box_ += feasible_box;
      }
    }
  }

  // Add the necklace beads to the bounding box.
  for (const Necklace::Ptr& necklace : necklaces_)
  {
    for (const Bead::Ptr& bead : necklace->beads)
    {
      CHECK_NOTNULL(bead);
      Point center;
      CHECK(necklace->shape->IntersectRay(bead->angle_rad, center));
      const Number& radius = scale_factor_ * bead->radius_base;

      const Box bead_box = GrowBoundingBox(center, radius);
      bounding_box_ += bead_box;
    }
  }

  // Add a small buffer around the bounding box.
  const Number buffer = kBoundingBoxBufferPx * (bounding_box_.xmax() - bounding_box_.xmin()) / options_->pixel_width;
  bounding_box_ = GrowBoundingBox(bounding_box_, buffer);
  unit_px_ = (bounding_box_.xmax() - bounding_box_.xmin()) / options_->pixel_width;
}

void SvgWriter::CreateBeadIntervalShapes()
{
  for (const Necklace::Ptr& necklace : necklaces_)
  {
    NecklaceIntervalVisitor visitor;
    necklace->shape->Accept(visitor);
    CircleNecklace::Ptr interval_shape = visitor.interval_shape();

    size_t count = 0;
    for (const Bead::Ptr& bead : necklace->beads)
    {
      if (!bead->feasible)
        continue;

      if (bead_interval_map_.find(bead) != bead_interval_map_.end())
        continue;

      if (options_->draw_feasible_intervals)
      {
        // Create a new circle shape to use for this bead.
        const Number radius = interval_shape->ComputeRadius() + kLineWidthPx * unit_px_ * ++count;
        bead_interval_map_[bead] = std::make_shared<CircleNecklace>(Circle(necklace->shape->kernel(), radius * radius));
      } else
      {
        bead_interval_map_[bead] = interval_shape;
      }
    }
  }
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
      const double multiply_alpha = options_->bead_opacity < 0 ? 1.0 : kDropShadowShade * options_->bead_opacity;
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

    const bool force_opaque = options_->bead_opacity < 1;
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

void SvgWriter::DrawKernel(const Point& kernel)
{
  if (!options_->draw_necklace_kernel)
    return;

  // Draw the necklace kernel as dot.
  printer_.OpenElement("circle");

  std::string style = kNecklaceKernelStyle;
  style = ForceStyle(style, "stroke-width:", kLineWidthPx * unit_px_);
  printer_.PushAttribute("style", style.c_str());

  {
    std::stringstream stream;
    stream << std::setprecision(options_->region_precision);
    stream << kernel.x();
    printer_.PushAttribute("cx", stream.str().c_str());
  }
  {
    std::stringstream stream;
    stream << std::setprecision(options_->region_precision);
    stream << kernel.y();
    printer_.PushAttribute("cy", stream.str().c_str());
  }

  printer_.PushAttribute("r", "0");
  printer_.PushAttribute("transform", transform_matrix_.c_str());
  printer_.CloseElement(); // circle
}

void SvgWriter::DrawBeadIds()
{
  if (!options_->draw_bead_ids)
    return;

  printer_.OpenElement("g");
  printer_.PushAttribute("font-family", kBeadIdFontFamily);
  {
    const Number font_size = options_->bead_id_font_size_px * unit_px_;
    std::stringstream stream;
    stream << font_size;
    printer_.PushAttribute("font-size", stream.str().c_str());
  }
  printer_.PushComment("Bead IDs");

  {
    // Note these are draw per element, because these reference the region that contains the ID.
    for (const MapElement::Ptr& element : elements_)
    {
      const std::string id = element->region.id;

      if (!element->necklace || !element->bead || !element->bead->valid)
        continue;

      printer_.OpenElement("text");
      printer_.PushAttribute("text-anchor", "middle");
      printer_.PushAttribute("alignment-baseline", "central");
      {
        Point position;
        CHECK(element->necklace->shape->IntersectRay(element->bead->angle_rad, position));

        // Note that the 'transform' argument does not apply to text coordinates.
        const Point transformed
          (
            kTransformScale * (position.x() - bounding_box_.xmin()),
            kTransformScale * (bounding_box_.ymax() - position.y())
          );

        {
          std::stringstream stream;
          stream << transformed.x();
          printer_.PushAttribute("x", stream.str().c_str());
        }
        {
          std::stringstream stream;
          stream << transformed.y();
          printer_.PushAttribute("y", stream.str().c_str());
        }
      }
      printer_.PushText(id.c_str());
      printer_.CloseElement(); // text

    }
  }

  printer_.CloseElement(); // g
}

} // namespace detail
} // namespace necklace_map
} // namespace geoviz
