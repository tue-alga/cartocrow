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

#include "necklace_writer.h"

#include <glog/logging.h>

#include "geoviz/common/bounding_box.h"


namespace geoviz
{
namespace
{

constexpr const char* kCopyrightNotice = "Copyright 2019 Netherlands eScience Center and TU Eindhoven\n"
                                         "Licensed under the Apache License, version 2.0. See LICENSE for details.";
constexpr const char* kSvgVersion = "1.1";
constexpr const char* kSvgAttributeBounds = "bounds";

constexpr const char* kFilterDropShadowId = "filterDropShadow";

constexpr const char* kNecklaceStyle = "fill:none;"
                                       "stroke:rgba(0%,0%,0%,100%);"
                                       "stroke-width:0.4;"
                                       "stroke-linecap:butt;"
                                       "stroke-linejoin:round;";
constexpr const char* kBeadIdFontFamily = "Verdana";

constexpr const char* kFeasibleIntervalStyle = "fill:none;"
                                               "stroke-linecap:butt;"
                                               "stroke-opacity:1;";
constexpr const char* kValidIntervalStyle = "fill:none;"
                                            "stroke-width:0.2;"
                                            "stroke-linecap:butt;"
                                            "stroke-linejoin:round;";
constexpr const char* kRegionAngleStyle = "fill:none;"
                                          "stroke:rgba(20%,20%,20%,70%);"
                                          "stroke-width:0.2;"
                                          "stroke-linecap:butt;";
constexpr const char* kBeadAngleStyle = "fill:none;"
                                        "stroke:rgba(0%,0%,0%,100%);"
                                        "stroke-width:0.2;"
                                        "stroke-linecap:butt;";

// Note that this source file contains string literals in various other places.
// However, it is likely that whenever these have to change, detailed knowledge of the SVG file structure is required.
// In this case, you will have to dive into the code anyway.

constexpr const double kTransformScale = 1;

constexpr const double kBoundingBoxBufferPx = 5;

constexpr const double kPointRegionRadiusPx = 3;
constexpr const double kBeadIdFontSizePx = 16;

constexpr const double kIntervalWidth = 0.4;
constexpr const int kIntervalNumericPrecision = 5;
constexpr const double kValidIntervalOpacity = 0.7;

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


class DrawNecklaceShapeVisitor : public necklace_map::NecklaceShapeVisitor
{
 public:
  DrawNecklaceShapeVisitor(const std::string& transform_matrix, tinyxml2::XMLPrinter& printer) :
    transform_matrix_(transform_matrix), printer_(printer)
  {}

  void Visit(necklace_map::CircleNecklace& shape)
  {
    const Point& kernel = shape.kernel();
    const Number radius = shape.ComputeRadius();

    printer_.OpenElement("circle");
    printer_.PushAttribute("style", kNecklaceStyle);
    printer_.PushAttribute("cx", kernel.x());
    printer_.PushAttribute("cy", kernel.y());
    printer_.PushAttribute("r", radius);
    printer_.PushAttribute("transform", transform_matrix_.c_str());
    printer_.CloseElement();
  }

  void Visit(necklace_map::CurveNecklace& shape)
  {
    LOG(FATAL) << "Not implemented yet.";
  }

  void Visit(necklace_map::GenericNecklace& shape)
  {
    LOG(FATAL) << "Not implemented yet.";
  }

 private:
  const std::string& transform_matrix_;

  tinyxml2::XMLPrinter& printer_;
};

} // anonymous namespace


WriterOptions::Ptr WriterOptions::Default()
{
  WriterOptions::Ptr options = std::make_shared<WriterOptions>();

  options->pixel_width = 500;

  options->region_precision = 9;
  options->region_opacity = -1;
  options->bead_opacity = 1;

  options->draw_necklace_curve = true;
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

  options->draw_necklace_curve = true;
  options->draw_bead_ids = true;

  options->draw_feasible_intervals = true;
  options->draw_valid_intervals = true;
  options->draw_region_angles = false;
  options->draw_bead_angles = true;

  return options;
}


namespace detail
{

/**@class NecklaceWriter
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
NecklaceWriter::NecklaceWriter
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
  CreateBeadIntervalShapes();

  OpenSvg();
}

NecklaceWriter::~NecklaceWriter()
{
  CloseSvg();

  out_ << printer_.CStr();
}

/**@brief Add the regions.
 *
 * These are drawn with the same style as the input, with the exception of the opacity. The opacity can either be set to the input opacity, or to some fixed value.
 */
void NecklaceWriter::DrawRegions()
{
  printer_.OpenElement("g");
  printer_.PushComment("Regions");

  {
    for (const MapElement::Ptr& element : elements_)
      DrawRegion(element->region);
  }

  printer_.CloseElement(); // g
}

/**@brief Add the necklace curves.
 *
 * The necklace curves are always drawn as a solid black curve.
 */
void NecklaceWriter::DrawNecklaces()
{
  if (!options_->draw_necklace_curve)
    return;

  printer_.OpenElement("g");
  printer_.PushComment("Necklaces");

  // How to draw each necklace depends on the necklace shape type.
  // We use a visitor pattern to overcome this ambiguity.
  DrawNecklaceShapeVisitor draw_visitor(transform_matrix_, printer_);
  for (const Necklace::Ptr& necklace : necklaces_)
    necklace->shape->Accept(draw_visitor);

  printer_.CloseElement(); // g
}

/**@brief Add the necklace beads.
 *
 * The necklace beads use mostly the same style as the regions, with drop-shadows to differentiate them for the underlying geography.
 * However, they can be forced to be semi-transparant. This also influences how their drop shadows and their borders are drawn.
 */
void NecklaceWriter::DrawBeads()
{
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
void NecklaceWriter::DrawFeasibleIntervals()
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
        if (!bead->valid)
          continue;

        NecklaceShape::Ptr interval_shape = bead_shape_map_[bead];
        CHECK_NOTNULL(interval_shape);

        printer_.OpenElement("path");
        {
          // The color of the interval is based on the region color.
          const std::string& style = bead->region_style;
          std::string color;
          GetStyle(style, "fill:", color);

          std::stringstream stream;
          stream << kFeasibleIntervalStyle << "stroke-width:" << kIntervalWidth << ";" << "stroke:" + color + ";";
          printer_.PushAttribute("style", stream.str().c_str());
        }
        {
          // Draw the feasible interval as a circular path.
          const Number& angle_cw_rad = bead->feasible->angle_cw_rad();
          const Number& angle_ccw_rad = bead->feasible->angle_ccw_rad();

          Point endpoint_cw, endpoint_ccw;
          CHECK(interval_shape->IntersectRay(angle_cw_rad, endpoint_cw));
          CHECK(interval_shape->IntersectRay(angle_ccw_rad, endpoint_ccw));
          const Number radius = interval_shape->ComputeRadius();

          std::stringstream stream;
          stream << std::setprecision(kIntervalNumericPrecision);
          stream <<
            "M " << endpoint_cw.x() << " " << endpoint_cw.y() <<
            " A " << radius << " " << radius << " 0 0 1 " <<
            endpoint_ccw.x() << " " << endpoint_ccw.y();

          printer_.PushAttribute("d", stream.str().c_str());
        }

        printer_.PushAttribute("transform", transform_matrix_.c_str());
        printer_.CloseElement();
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
void NecklaceWriter::DrawValidIntervals()
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

        NecklaceShape::Ptr interval_shape = bead_shape_map_[bead];
        CHECK_NOTNULL(interval_shape);

        printer_.OpenElement("path");
        {
          // The color of the interval is based on the region color.
          const std::string& style = bead->region_style;
          std::string color;
          GetStyle(style, "fill:", color);

          std::stringstream stream;
          stream <<
            kValidIntervalStyle <<
            "stroke:" << color << ";" <<
            "stroke-opacity:" << kValidIntervalOpacity << ";";

          printer_.PushAttribute("style", stream.str().c_str());
        }
        {
          // Draw the valid interval as a wedge from the necklace kernel to either the necklace, or the feasible interval if it is also drawn.
          const Number& angle_cw_rad = bead->valid->angle_cw_rad();
          const Number& angle_ccw_rad = bead->valid->angle_ccw_rad();

          Point endpoint_cw, endpoint_ccw;
          CHECK(interval_shape->IntersectRay(angle_cw_rad, endpoint_cw));
          CHECK(interval_shape->IntersectRay(angle_ccw_rad, endpoint_ccw));

          std::stringstream stream;
          stream << std::setprecision(kIntervalNumericPrecision);
          stream <<
               "M " << endpoint_cw.x() << " " << endpoint_cw.y() <<
               " L " << interval_shape->kernel().x() << " " << interval_shape->kernel().y() <<
               " L " << endpoint_ccw.x() << " " << endpoint_ccw.y();

          printer_.PushAttribute("d", stream.str().c_str());
        }

        printer_.PushAttribute("transform", transform_matrix_.c_str());
        printer_.CloseElement();
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
void NecklaceWriter::DrawRegionAngles()
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
      if (element->beads.empty())
        continue;

      Polygon simple;
      element->region.MakeSimple(simple);
      const Point centroid = compute_centroid(simple);

      for (const MapElement::BeadMap::value_type& map_value : element->beads)
      {
        const Necklace::Ptr& necklace = map_value.first;
        const Bead::Ptr& bead = map_value.second;
        if (!bead->valid)
          continue;

        printer_.OpenElement("path");
        printer_.PushAttribute("style", kRegionAngleStyle);
        {
          NecklaceShape::Ptr interval_shape = bead_shape_map_[bead];
          CHECK_NOTNULL(interval_shape);

          const Point& kernel = interval_shape->kernel();
          const Number angle_centroid = interval_shape->ComputeAngle(centroid);
          Point endpoint;
          CHECK(interval_shape->IntersectRay(angle_centroid, endpoint));

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

/**@brief Add line segments connecting the necklace kernel(s) with the bead centers.
 *
 * These line segments are always colored black.
 */
void NecklaceWriter::DrawBeadAngles()
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
      const Number necklace_radius = necklace->shape->ComputeRadius();

      size_t count = 1;
      for (const Bead::Ptr& bead : necklace->beads)
      {
        if (!bead->valid)
          continue;

        printer_.OpenElement("path");
        printer_.PushAttribute("style", kBeadAngleStyle);
        {
          NecklaceShape::Ptr interval_shape = bead_shape_map_[bead];
          CHECK_NOTNULL(interval_shape);

          Point endpoint;
          CHECK(interval_shape->IntersectRay(bead->angle_rad, endpoint));

          std::stringstream stream;
          stream << std::setprecision(kIntervalNumericPrecision);
          stream << "M " << kernel.x() << " " << kernel.y() << " L " << endpoint.x() << " " << endpoint.y();

          printer_.PushAttribute("d", stream.str().c_str());
        }
        printer_.PushAttribute("transform", transform_matrix_.c_str());
        printer_.CloseElement();
      }
    }
  }

  printer_.CloseElement(); // g
}

void NecklaceWriter::OpenSvg()
{
  // The file must start with the copyright notice,
  printer_.PushComment(kCopyrightNotice);

  // Compute the bounding box and determine the conversion units (world -> pixel).
  ComputeBoundingBox();

  const double width = bounding_box_.xmax() - bounding_box_.xmin();
  const double height = bounding_box_.ymax() - bounding_box_.ymin();
  unit_px_ = width / options_->pixel_width;
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

void NecklaceWriter::CloseSvg()
{
  // Add hint to display when the geometry could not be drawn.
  printer_.PushText("Sorry, your browser does not support the svg tag.");
  printer_.CloseElement();
}

void NecklaceWriter::ComputeBoundingBox()
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

        const double buffer = kIntervalWidth * (necklace->beads.size() + 1);
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
}

void NecklaceWriter::CreateBeadIntervalShapes()
{
  for (const Necklace::Ptr& necklace : necklaces_)
  {
    size_t count = 0;
    for (const Bead::Ptr& bead : necklace->beads)
    {
      if (!bead->valid)
        continue;

      if (bead_shape_map_.find(bead) != bead_shape_map_.end())
        continue;

      NecklaceShape::Ptr interval_shape = necklace->shape;
      if (options_->draw_feasible_intervals)
      {
        // Create a new circle shape to use for this bead.
        const Number radius = necklace->shape->ComputeRadius() + kIntervalWidth * ++count;
        interval_shape = std::make_shared<CircleNecklace>(Circle(necklace->shape->kernel(), radius * radius));
      }

      bead_shape_map_[bead] = interval_shape;
    }
  }
}

void NecklaceWriter::AddDropShadowFilter()
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

void NecklaceWriter::DrawRegion(const Region& region)
{
  const std::string style =
    options_->region_opacity < 0
    ? region.style
    : ForceStyle(region.style, "fill-opacity:", options_->region_opacity);

  if (region.shape.size() == 1 && region.shape[0].outer_boundary().size() == 1)
  {
    const Point& position = region.shape[0].outer_boundary()[0];

    // Draw the region with the region as a circle with same style as the input, except the opacity may be adjusted.
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

    printer_.PushAttribute("id", region.id.c_str());
    printer_.PushAttribute("transform", transform_matrix_.c_str());
    printer_.CloseElement();

    return;
  }

  // Draw the region with the region as a piecewise linear polygon with same style as the input, except the opacity may be adjusted.
  printer_.OpenElement("path");
  printer_.PushAttribute("style", style.c_str());
  printer_.PushAttribute("d", RegionToPath(region, options_->region_precision).c_str());
  printer_.PushAttribute("id", region.id.c_str());
  printer_.PushAttribute("transform", transform_matrix_.c_str());
  printer_.CloseElement();
}

void NecklaceWriter::DrawBeadIds()
{
  if (!options_->draw_bead_ids)
    return;

  printer_.OpenElement("g");
  printer_.PushAttribute("font-family", kBeadIdFontFamily);
  {
    const Number font_size = kBeadIdFontSizePx * unit_px_;
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
      for (const MapElement::BeadMap::value_type& map_value : element->beads)
      {
        const Necklace::Ptr& necklace = map_value.first;
        const Bead::Ptr& bead = map_value.second;
        if (!bead->valid)
          continue;

        printer_.OpenElement("text");
        printer_.PushAttribute("text-anchor", "middle");
        printer_.PushAttribute("alignment-baseline", "central");
        {
          Point position;
          CHECK(necklace->shape->IntersectRay(bead->angle_rad, position));

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
  }

  printer_.CloseElement(); // g
}

} // namespace detail
} // namespace geoviz
