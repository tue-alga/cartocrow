/*
The GeoViz library implements algorithmic geo-visualization methods,
developed at TU Eindhoven.
Copyright (C) 2021  Netherlands eScience Center and TU Eindhoven

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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 09-03-2021
*/

#include "svg_writer.h"

#include <glog/logging.h>

#include "geoviz/common/bounding_box.h"


namespace geoviz
{
namespace
{

constexpr const char* kSvgVersion = "1.1";
constexpr const char* kSvgAttributeBounds = "bounds";

constexpr const char* kLineStyle = "fill:none;"//rgba(100%,10%,10%,100%);"
                                   "stroke:rgba(0%,0%,0%,100%);"
                                   "stroke-linecap:butt;"
                                   "stroke-linejoin:round;";

constexpr const char* kPointStyle = "fill:none;"
                                    "stroke:rgba(0%,0%,0%,100%);"
                                    "stroke-linecap:butt;"
                                    "stroke-linejoin:round;";

constexpr char kAbsoluteMove = 'M';
constexpr char kAbsoluteCubicBezier = 'C';
constexpr char kAbsoluteClose = 'Z';


// Note that this source file contains string literals in various other places.
// However, it is likely that whenever these have to change, detailed knowledge of the SVG file structure is required.
// In this case, you will have to dive into the code anyway.

constexpr const double kTransformScale = 1;

constexpr const double kLineWidthPx = 1.7;
constexpr const double kPointRadiusPx = 3;
constexpr const double kBoundingBoxBufferPx = 5;

constexpr const double kSpiralStep = 0.05;//0.1;
constexpr const double kSpiralMax = 10.0;

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

std::string SpiralToPath(const Spiral& spiral, const Vector& offset, const Number& R_max, const Number& R_min, const int precision)
{
  std::stringstream stream;
  stream << std::setprecision(precision);

  const Number t_min = spiral.ComputeT(R_max);
  const Point far = spiral.Evaluate(t_min).to_cartesian() + offset;
  stream << "M " << far.x() << " " << far.y();

  if (spiral.angle_rad() != 0)
  {
    for (double t = t_min + kSpiralStep; t < kSpiralMax; t += kSpiralStep)
    {
      const PolarPoint polar = spiral.Evaluate(t);
      if (polar.R() <= R_min)
        break;

      const Point point = polar.to_cartesian() + offset;
      stream << " L " << point.x() << " " << point.y();
    }
  }

  const Point near = (R_min == 0 ? Point(0,0) : spiral.Evaluate(spiral.ComputeT(R_min)).to_cartesian()) + offset;
  stream << " L " << near.x() << " " << near.y();
  return stream.str();
}

std::string LineToPath(const PolarLine& line, const Vector& offset, const Number& t_from, const Number& t_to, const int precision)
{
  std::stringstream stream;
  stream << std::setprecision(precision);

  const Point from = line.Evaluate(t_from).to_cartesian() + offset;
  stream << "M " << from.x() << " " << from.y();

  const Point to = line.Evaluate(t_to).to_cartesian() + offset;
  stream << " L " << to.x() << " " << to.y();
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


namespace detail
{

SvgWriter::SvgWriter
(
  const std::vector<PolarPoint>& points,
  const std::vector<Spiral>& spirals,
  const std::vector<SpiralSegment>& spiral_segments,
  const std::vector<PolarLine>& lines,
  const std::vector<PolarSegment>& line_segments,
  const WriteOptions::Ptr& options,
  std::ostream& out
) :
  points_(points),
  spirals_(spirals),
  spiral_segments_(spiral_segments),
  lines_(lines),
  line_segments_(line_segments),
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

void SvgWriter::DrawSpirals()
{
  const Number x_max = std::max(std::abs(bounding_box_.xmin()), std::abs(bounding_box_.xmax()));
  const Number y_max = std::max(std::abs(bounding_box_.ymin()), std::abs(bounding_box_.ymax()));
  const Number max_dim = std::sqrt(x_max*x_max + y_max*y_max);

  for (const Spiral& spiral : spirals_)
  {
    DrawSpiral(spiral, max_dim);
  }
  for (const SpiralSegment& segment : spiral_segments_)
  {
    DrawSpiral(segment, segment.R_max(), segment.R_min());
  }
}

void SvgWriter::DrawLines()
{
  const Number x_max = std::max(std::abs(bounding_box_.xmin()), std::abs(bounding_box_.xmax()));
  const Number y_max = std::max(std::abs(bounding_box_.ymin()), std::abs(bounding_box_.ymax()));
  const Number max_dim = std::sqrt(x_max*x_max + y_max*y_max);

  for (const PolarLine& line : lines_)
  {
    DrawLine(line, -max_dim, max_dim);
  }
  for (const PolarSegment& segment : line_segments_)
  {
    DrawLine(segment, segment.FromT(), segment.ToT());
  }
}

void SvgWriter::DrawPoints()
{
  for (const PolarPoint& point : points_)
  {
    printer_.OpenElement("circle");

    {
      std::string style = kPointStyle;
      style = ForceStyle(style, "stroke-width:", kLineWidthPx * unit_px_);
      printer_.PushAttribute("style", style.c_str());
    }

    {
      const Point position = point.to_cartesian();
      printer_.PushAttribute("cx", position.x());
      printer_.PushAttribute("cy", position.y());
    }

    {
      std::stringstream stream;
      const Number radius = kPointRadiusPx * unit_px_;
      stream << radius;
      printer_.PushAttribute("r", stream.str().c_str());
    }

    printer_.PushAttribute("transform", transform_matrix_.c_str());
    printer_.CloseElement(); // circle
  }
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
}

void SvgWriter::CloseSvg()
{
  // Add hint to display when the geometry could not be drawn.
  printer_.PushText("Sorry, your browser does not support the svg tag.");
  printer_.CloseElement(); // svg
}

void SvgWriter::ComputeBoundingBox()
{
  Kernel::Construct_bbox_2 bbox = Kernel().construct_bbox_2_object();
  bounding_box_ = Box(-10, -10, 10, 10);

  // Add the points to the bounding box.
  for (const PolarPoint& point : points_)
  {
    bounding_box_ += bbox(point.to_cartesian());
  }

  // Add the spirals to the bounding box.
  for (const Spiral& spiral : spirals_)
  {
    const Point point = spiral.anchor().to_cartesian();
    bounding_box_ += bbox(point);
  }
  for (const SpiralSegment& segment : spiral_segments_)
  {
    const Point point = segment.far().to_cartesian();
    bounding_box_ += bbox(point);
  }

  // Add the lines to the bounding box.
  for (const PolarLine& line : lines_)
  {
    const Point from = line.Evaluate(-10).to_cartesian();
    const Point to = line.Evaluate(10).to_cartesian();
    bounding_box_ += bbox(from) + bbox(to);
  }
  for (const PolarSegment& segment : line_segments_)
  {
    const Point from = segment.Evaluate(0).to_cartesian();
    const Point to = segment.Evaluate(1).to_cartesian();
    bounding_box_ += bbox(from) + bbox(to);
  }

  // Add a small buffer around the bounding box.
  const Number buffer = kBoundingBoxBufferPx * (bounding_box_.xmax() - bounding_box_.xmin()) / options_->pixel_width;
  bounding_box_ = GrowBoundingBox(bounding_box_, buffer);
  unit_px_ = (bounding_box_.xmax() - bounding_box_.xmin()) / options_->pixel_width;
}

void SvgWriter::DrawSpiral(const Spiral& spiral, const Number& R_max, const Number& R_min /*= 0*/)
{
  std::string style = kLineStyle;
  style = ForceStyle(style, "stroke-width:", kLineWidthPx * unit_px_);

  printer_.OpenElement("path");
  printer_.PushAttribute("style", style.c_str());
  printer_.PushAttribute("d", SpiralToPath(spiral, Vector(0, 0), R_max, R_min, options_->numeric_precision).c_str());
  printer_.PushAttribute("transform", transform_matrix_.c_str());
  printer_.CloseElement(); // path
}

void SvgWriter::DrawLine(const PolarLine& line, const Number& t_from, const Number& t_to)
{
  std::string style = kLineStyle;
  style = ForceStyle(style, "stroke-width:", kLineWidthPx * unit_px_);

  printer_.OpenElement("path");
  printer_.PushAttribute("style", style.c_str());
  printer_.PushAttribute("d", LineToPath(line, Vector(0, 0), t_from, t_to, options_->numeric_precision).c_str());
  printer_.PushAttribute("transform", transform_matrix_.c_str());
  printer_.CloseElement(); // path
}

} // namespace detail
} // namespace geoviz
