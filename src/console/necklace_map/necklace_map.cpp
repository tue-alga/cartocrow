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

Created by tvl (t.vanlankveld@esciencecenter.nl) on 10-09-2019
*/

// Note dependencies:
// - glog
// - gflags

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <gflags/gflags.h>
#include <glog/logging.h>

#include <cmake/geoviz_config.h>
#include <geoviz/necklace_map/necklace_map.h>

#include "console/common/utils_cla.h"
#include "console/common/detail/svg_polygon_parser.h"
#include "console/common/detail/svg_visitor.h"

#include "console/necklace_map/svg_reader.h"
#include "console/necklace_map/data_reader.h"



// Source files for
// * generic SVG parser
// * necklace specific parser and reader.



// When reading example data:
// .mrk files: number of IDs; per line: ID.
// .txt files: number of countries; per country: ID, name, necklace value, 'flow value'
// .ipe files: country and necklace shapes (check java implementation for format...)
// as input, I expect either an XML file with SVG content, or (later on) that content as command line argument...
// another input is the data values per region and possibly other aspects like the color.

//TODO(tvl) extract string literals and make them static const

using namespace geoviz;  //TODO(tvl) remove: name types explicitly!
using namespace geoviz::necklace_map;  //TODO(tvl) remove: name types explicitly!






std::string toString(const Region::PolygonSet& shape)
{
  std::stringstream sout;
  sout << std::setprecision(9);  // TODO(tvl) we should align this precision with the maximum expected precision of the data. Probably should be a parameter with default.

  for (const Polygon_with_holes& polygon : shape)
  {
    for
    (
      Polygon::Vertex_const_iterator point_iter = polygon.outer_boundary().vertices_begin();
      point_iter != polygon.outer_boundary().vertices_end();
      ++point_iter
    )
    {
      if (point_iter == polygon.outer_boundary().vertices_begin())
        sout << " M ";
      else
        sout << " L ";

      sout << point_iter->x() << " " << point_iter->y();
    }

    if
    (
      1 < polygon.outer_boundary().size() &&
      polygon.outer_boundary().vertices_begin() != --polygon.outer_boundary().vertices_end()
    )
      sout << " Z";
  }

  if (sout.str().empty()) return "";
  return sout.str().substr(1);
}

bool writeDummySvg
(
  const std::vector<MapElement::Ptr>& elements,  //TODO(tvl) rename "elements"
  const Necklace::Ptr& necklace,
  const Number& scale_factor,
  std::string& svg,
  bool write_debug_info = false
)
{
  //std::vector<Number> angles_rad = {0, M_PI_4, M_PI_2, M_PI_2 + M_PI_4, M_PI, M_PI + M_PI_2};
  //const std::vector<MapElement::Ptr>& elements = necklace->beads;
  const std::vector<NecklaceGlyph::Ptr>& beads = necklace->beads;


  // Create dummy necklace glyphs and write everything to an SVG string.
  const Point& kernel = necklace->shape->kernel();

  //std::shared_ptr<CircleNecklace> circle_necklace = std::static_pointer_cast<CircleNecklace>(necklace);
  //const Number radius = circle_necklace == nullptr ? 0 : CGAL::sqrt(circle_necklace->shape_.squared_radius());
  const Box bounding_box = necklace->shape->ComputeBoundingBox();
  const Number necklace_radius = (bounding_box.xmax() - bounding_box.xmin()) / 2.0;



  Number bounds[] = {kernel.x(), kernel.y(), kernel.x(), kernel.y()};

  // Necklace
  std::vector<Number> glyph_radii(beads.size(), 0);
  std::vector<Point> glyph_centers;
  for (const NecklaceGlyph::Ptr& bead : beads)
  {
    CHECK_NOTNULL(bead);
    Point glyph_center;
    CHECK(necklace->shape->IntersectRay(bead->angle_rad, glyph_center));

    const Number& radius = scale_factor * bead->radius_base;

    bounds[0] = std::min(bounds[0], glyph_center.x() - radius);
    bounds[1] = std::min(bounds[1], glyph_center.y() - radius);
    bounds[2] = std::max(bounds[2], glyph_center.x() + radius);
    bounds[3] = std::max(bounds[3], glyph_center.y() + radius);
  }

  // Regions.
  for (const MapElement::Ptr& element : elements)
  {
    for (const Polygon_with_holes& polygon : element->region.shape)
    {
      for
      (
        Polygon::Vertex_const_iterator point_iter = polygon.outer_boundary().vertices_begin();
        point_iter != polygon.outer_boundary().vertices_end();
        ++point_iter
      )
      {
        bounds[0] = std::min(bounds[0], point_iter->x());
        bounds[1] = std::min(bounds[1], point_iter->y());
        bounds[2] = std::max(bounds[2], point_iter->x());
        bounds[3] = std::max(bounds[3], point_iter->y());
      }
    }
  }

  // Buffer.
  const Number buffer = 2;
  bounds[0] -= buffer;
  bounds[1] -= buffer;
  bounds[2] += buffer;
  bounds[3] += buffer;

  const Number width = bounds[2] - bounds[0];
  const Number height = bounds[3] - bounds[1];
  const Number pixel_width = 500;
  const Number pixel_height = pixel_width * height / width;

  const Number transform_scale = 1;
  std::stringstream transform_str;
  transform_str << "matrix(" <<
    transform_scale << ",0,0," <<
   -transform_scale << "," <<
   -transform_scale * bounds[0] << "," <<
    transform_scale * bounds[3] << ")";


  tinyxml2::XMLPrinter printer;
  printer.PushComment
  (
    "Copyright 2019 Netherlands eScience Center and TU Eindhoven\n"
    "Licensed under the Apache License, version 2.0. See LICENSE for details."
  );

  printer.OpenElement("svg");
  printer.PushAttribute("xmlns", "http://www.w3.org/2000/svg");
  printer.PushAttribute("xmlns:xlink", "http://www.w3.org/1999/xlink");
  printer.PushAttribute("width", pixel_width);
  printer.PushAttribute("height", pixel_height);

  {
    std::stringstream view_str;
    //view_str << bounds[0] << " " << bounds[1] << " " << width << " " << height;
    view_str << "0 0 " << width << " " << height;
    printer.PushAttribute("viewBox", view_str.str().c_str());
  }

  printer.PushAttribute("version", "1.1");

  {
    std::stringstream bounds_str;
    bounds_str << "[[" << bounds[1] << "," << bounds[0] << "],[" << bounds[3] << "," << bounds[2] << "]]";  // Note, bounds expected in latitude-longitude.
    printer.PushAttribute("bounds", bounds_str.str().c_str());
    //printer.PushAttribute("bounds", "[[52.356,4.945],[52.354,4.947]]");  // TODO(tvl) Note: lat-lon!
  }

  for (const MapElement& element : elements)
  {
    const Region& region = element.region;

    printer.OpenElement("path");
    printer.PushAttribute("style", region.style.c_str());
    printer.PushAttribute("d", toString(region.shape).c_str());
    printer.PushAttribute("id", region.id.c_str());
    printer.PushAttribute("transform", transform_str.str().c_str());
    printer.CloseElement();
  }

  printer.OpenElement("circle");
  printer.PushAttribute("style", "fill:none;stroke-width:0.4;stroke-linecap:butt;stroke-linejoin:round;stroke:rgb(0%,0%,0%);stroke-opacity:1;stroke-miterlimit:10;");
  printer.PushAttribute("cx", kernel.x());
  printer.PushAttribute("cy", kernel.y());
  printer.PushAttribute("r", necklace_radius);
  printer.PushAttribute("transform", transform_str.str().c_str());
  printer.CloseElement();

  for (size_t n = 0; n < centers.size(); ++n)
  {
    const std::string style = elements[n].region.style;

    {
      const std::shared_ptr<NecklaceInterval>& interval = elements[n].glyph.interval;

      const size_t from = style.find("fill:");
      const size_t to = style.find(")", from);
      const std::string color = style.substr(from + 5, to - from - 4);
      const std::string interval_style =
        "fill:none;stroke-width:0.4;stroke-linecap:butt;stroke-linejoin:round;stroke:"
        + color +
        ";stroke-opacity:1;stroke-miterlimit:10;";

      const Number my_radius = necklace_radius + (0.4 * (n+1));

      CircleNecklace my_necklace(Circle(kernel, my_radius * my_radius));

      Point from_pt, to_pt;
      my_necklace.IntersectRay(interval->angle_cw_rad(), from_pt);
      my_necklace.IntersectRay(interval->angle_ccw_rad(), to_pt);

      std::stringstream sout;
      sout << std::setprecision(9);  // TODO(tvl) we should align this precision with the maximum expected precision of the data. Probably should be a parameter with default.
      sout << "M " << from_pt.x() << " " << from_pt.y();
      sout << " A " << my_radius << " " << my_radius << " 0 0 1 ";
      sout  << to_pt.x() << " " << to_pt.y();
      std::string path_str = sout.str();

      printer.OpenElement("path");
      printer.PushAttribute("style", interval_style.c_str());
      printer.PushAttribute("d", path_str.c_str());
      printer.PushAttribute("transform", transform_str.str().c_str());
      printer.CloseElement();
    }

    {
      const Number radius = std::sqrt(elements[n].value);

      const size_t from = style.find("fill-opacity:");
      const size_t to = style.find(";", from);
      const std::string glyph_style =
        style.substr(0, from+13)
        + "0.5" +
        style.substr(to);

      printer.OpenElement("circle");
      printer.PushAttribute("style", glyph_style.c_str());
      printer.PushAttribute("cx", centers[n].x());
      printer.PushAttribute("cy", centers[n].y());
      printer.PushAttribute("r", radius);
      printer.PushAttribute("transform", transform_str.str().c_str());
      printer.CloseElement();
    }

    {
      std::stringstream sout;
      sout << std::setprecision(9
                               );  // TODO(tvl) we should align this precision with the maximum expected precision of the data. Probably should be a parameter with default.
      sout << "M " << kernel.x() << " " << kernel.y() << " L " << centers[n].x() << " " << centers[n].y();
      std::string path_str = sout.str();

      printer.OpenElement("path");
      printer.PushAttribute
      (
        "style",
        "fill:none;stroke-width:0.2;stroke-linecap:butt;stroke-linejoin:round;stroke:rgb(0%,0%,0%);stroke-opacity:1;stroke-miterlimit:10;"
      );
      printer.PushAttribute("d", path_str.c_str());
      printer.PushAttribute("transform", transform_str.str().c_str());
      printer.CloseElement();
    }
  }

  printer.PushText("Sorry, your browser does not support the svg tag.");
  printer.CloseElement();
  svg = printer.CStr();
}


int main(int argc, char **argv)
{
  InitApplication
  (
    argc,
    argv,
    "Command line application that exposes the functionality of the GeoViz necklace map.",
    {"<some arg>", "<another arg>"}
  );

  // Writing to the standard output is reserved for text that should be returned to a calling website.
  FLAGS_logtostderr = true;

  // Note that here is a good place to check the semantic validity of the flags.
  // While this can be done by adding flag validators using gflags,
  // These generally only allow (statically) validating independent flags,
  // they do not allow validating programmatically set flags,
  // and they may throw inconvenient compiler warnings about unused static variables
  // (depending on your implementation).
  // Instead of registering validators, I prefer to add a "ValidateFlags()"
  // method call here that performs all the semantic validation and error messaging
  // (syntactic validation will be done by gflags automatically).
  // For added value, this method could always log the values of important flags
  // to make it easier to reproduce a particular program execution.

  //LOG(INFO) << "GeoViz version: " << GEOVIZ_VERSION;

  // TODO(tvl) when moving to text input as flag, be careful of special characters such as ':', '"', ''', '='!

  const std::string svg_filename = "/storage/GeoViz/wwwroot/data/Example_wEU/wEU_svg.xml";
  const std::string data_filename = "/storage/GeoViz/wwwroot/data/Example_wEU/wEU.txt";
  const std::string value_name = "value";

  std::vector<MapElement::Ptr> elements;
  std::vector<Necklace::Ptr> necklaces;




  // TODO(tvl) capture in method that also does the retries and make stable!
  const int max_tries = 5;
  int c = 0;
  bool success_read_data = false;
  do
  {
    try
    {
      DataReader data_reader(elements);
      success_read_data = data_reader.Read(data_filename, value_name);
    }
    catch (const std::exception& e)
    {
      success_read_data = false;
      LOG(ERROR) << e.what();
    }

    if (success_read_data)
      break;
    else
      LOG(INFO) << "Failed to read necklace map data file " << data_filename;
  } while (max_tries < ++c);

  c = 0;
  bool success_read_svg = false;
  do
  {
    try
    {
      SvgReader svg_reader
      (
        elements,
        necklaces
      );
      success_read_svg = svg_reader.Read(svg_filename);
      LOG_IF(INFO, !elements.empty())
      << "Read necklace map geometry file " << svg_filename
      << " (" << elements.size() << " regions; "<< necklaces.size() << " necklaces)";
      // Note(tvl) we should allow the SVG to not contain the necklace: then create the necklace as smallest enclosing circle.
    }
    catch (const std::exception& e)
    {
      success_read_svg = false;
      LOG(ERROR) << e.what();
    }

    if (success_read_svg)
      break;
    else
      LOG(INFO) << "Failed to read necklace map geometry file " << svg_filename;
  } while (max_tries < ++c);

  // Generate intervals based on the regions and necklaces.
  std::unique_ptr<IntervalGenerator> make_intervals;
  make_intervals.reset(new IntervalCentroidGenerator(FLAGS_centroid_interval_length_rad));
  (*make_intervals)(elements);

  // Compute the scaling factor.
  std::unique_ptr<GlyphScaler> scaler(new GlyphScalerFixedOrder(FLAGS_min_separation)); // TODO(tvl) Note the min_separation should depend on the number of elements per necklace: make sure that this separation cannot break the algorithm. i.e. there is a scale factor such that the glyphs have at least radius 0...
  const Number scale_factor = (*scaler)(necklaces);











  // TODO(tvl) move determining the positioning limits into positioner?

  // Position the glyphs at the start of their interval.
  Necklace::Ptr necklace = necklaces[0];
  for (NecklaceGlyph::Ptr& glyph : necklace->beads)
  {
    CHECK_NOTNULL(glyph);

    glyph->angle_min_rad = glyph->interval->angle_cw_rad();
    glyph->angle_max_rad = glyph->interval->angle_ccw_rad();
  }

  // Adjust the glyphs position so there are no overlapping glyphs.
  const Number necklace_radius = necklaces[0]->shape->ComputeLength() / M_2xPI;
  std::vector<Number> covering_radii_scaled_rad(elements.size(), 0);

  Number angle_prev_end_rad = -M_2xPI;
  for (int i = 0; i < 2; ++i)
  for (size_t e = 0; e < necklace->beads.size(); ++e)
  {
    NecklaceGlyph::Ptr& glyph = necklace->beads[e];
    CHECK_NOTNULL(glyph);

    // Determine the covering radius.
    const Number radius_scaled = scale_factor * glyph->radius_base + (FLAGS_min_separation / 2);
    //const Number& covering_radius_scaled_rad = covering_radii_scaled_rad[n] = 2 * std::asin(radius_scaled / (2 * necklace_radius));
    const Number& covering_radius_scaled_rad = covering_radii_scaled_rad[e] = std::asin(radius_scaled / necklace_radius);
    // The glyph must start past the previous endpoint.
    Number start = glyph->angle_min_rad - covering_radius_scaled_rad;
    if (start < 0) start += M_2xPI;
    if (start < angle_prev_end_rad)
    {
      glyph->angle_min_rad = angle_prev_end_rad + covering_radius_scaled_rad;
      if (glyph->interval->angle_ccw_rad() < glyph->angle_min_rad)
        glyph->angle_min_rad = glyph->interval->angle_ccw_rad();
      if (M_2xPI < glyph->angle_min_rad)
        glyph->angle_min_rad -= M_2xPI;
    }

    angle_prev_end_rad = glyph->angle_min_rad + covering_radius_scaled_rad;
    if (M_2xPI < angle_prev_end_rad)
      angle_prev_end_rad -= M_2xPI;
  }

  Number angle_prev_start_rad = 2 * M_2xPI;
  for (int i = 0; i < 2; ++i)
  for (ptrdiff_t n = necklace->beads.size()-1; 0 <= n; --n)
  {
    NecklaceGlyph::Ptr& glyph = necklace->beads[n];
    CHECK_NOTNULL(glyph);

    const Number& covering_radius_scaled_rad = covering_radii_scaled_rad[n];

    // The glyph must end before the previous startpoint.
    Number end = glyph->angle_max_rad + covering_radius_scaled_rad;
    /*if (M_2xPI < end)
      end -= M_2xPI;*/
    if (angle_prev_start_rad < end)
    {
      glyph->angle_max_rad = angle_prev_start_rad - covering_radius_scaled_rad;
      if (glyph->angle_max_rad < 0)
        glyph->angle_max_rad += M_2xPI;
      if (glyph->angle_max_rad < glyph->interval->angle_cw_rad())
        glyph->angle_max_rad = glyph->interval->angle_cw_rad();
    }

    angle_prev_start_rad = glyph->angle_max_rad - covering_radius_scaled_rad;
    if (angle_prev_start_rad < 0)
      angle_prev_start_rad += M_2xPI;
  }








  // Tested: the elements must start in a valid position.
  for (NecklaceGlyph::Ptr& glyph : necklace->beads)
    glyph->angle_rad = glyph->angle_min_rad;

  const Number repVal = FLAGS_glyph_repulsion;

  const Number midStr = 1;
  const Number repStr = std::pow(repVal, 4);
  const Number EPSILON = 1e-7;

  const size_t num_beads = necklace->beads.size();
  for (int n = 0; n < 30; ++n)
  {
    for (size_t i = 0; i < num_beads; ++i)
    {
      NecklaceGlyph::Ptr& glyph = necklace->beads[i];
      CHECK_NOTNULL(glyph);

      int j1 = (i + num_beads - 1)%num_beads;
      /*while (!necklace->beads[j1]->glyph)
        j1 = (j1 + num_beads - 1)%num_beads;*/
      int j2 = (i+1)%num_beads;
      /*while (!necklace->beads[j2]->glyph)
        j2 = (j2+1)%num_beads;*/

      NecklaceGlyph::Ptr& prev = necklace->beads[j1];
      NecklaceGlyph::Ptr& next = necklace->beads[j2];

      NecklaceInterval r1(prev->angle_rad, glyph->angle_rad);
      NecklaceInterval r2(prev->angle_rad, next->angle_rad);
      NecklaceInterval r3(glyph->interval->ComputeCentroid(), glyph->angle_rad);

      const Number length_r1 = r1.ComputeLength();
      const Number length_r2 = r2.ComputeLength();
      const Number length_r3 = r3.ComputeLength();
      
      // determ. cov. radii (can be stored in vector, but then make sure these are also swapped if the elements are swapped...)
      const Number elem_radius = std::asin((scale_factor * glyph->radius_base + (FLAGS_min_separation / 2)) / necklace_radius);
      const Number prev_radius = std::asin((scale_factor * prev->radius_base + (FLAGS_min_separation / 2)) / necklace_radius);
      const Number next_radius = std::asin((scale_factor * next->radius_base + (FLAGS_min_separation / 2)) / necklace_radius);


      double d1 = prev_radius + elem_radius;
      double d2 = length_r2 - next_radius - elem_radius;
      if (j1 == j2)
        d2 = M_2xPI - next_radius - elem_radius;
      double m =
        ((length_r3 < M_PI)
         ? (length_r1 - length_r3)
         : (length_r1 + (M_2xPI - length_r3)));
      double e = midStr * m * d1 * d2 - repStr * (d1 + d2);
      double f = 2.0 * repStr - midStr * ((d1 + m) * (d2 + m) - m * m);
      double g = midStr * (d1 + d2 + m);
      double h = -midStr;



      // solve h x^3 + g x^2 + f x + e == 0
      if (std::abs(h) < EPSILON && std::abs(g) < EPSILON)
      {
        double x = -e / f + prev->angle_rad;
        while (x > M_2xPI) x -= M_2xPI;
        while (x < 0) x += M_2xPI;
        if (!glyph->interval->IntersectsRay(x))
        {
          if (2.0 * length_r1 - (d1 + d2) > 0.0)
            x = glyph->interval->angle_cw_rad();
          else
            x = glyph->interval->angle_ccw_rad();
        }
        glyph->angle_rad = x;
      }
      else
      {
        double q = (3.0 * h * f - g * g) / (9.0 * h * h);
        double r = (9.0 * h * g * f - 27.0 * h * h * e - 2.0 * g * g * g) / (54.0 * h * h * h);
        //double z = q * q * q + r * r;
        double rho = CGAL::sqrt(-q * q * q);
        if (std::abs(r) > rho) rho = std::abs(r);
        double theta = std::acos(r / rho);
        rho = std::pow(rho, 1.0 / 3.0);
        double x = -rho * std::cos(theta / 3.0) - g / (3.0 * h) + rho * CGAL::sqrt(3.0) * std::sin(theta/3.0);
        x += prev->angle_rad;
        while (x > M_2xPI) x -= M_2xPI;
        while (x < 0) x += M_2xPI;
        if (!glyph->interval->IntersectsRay(x)) {
          if (repStr * (2.0 * length_r1 - (d1 + d2)) + midStr * (m - length_r1) * (length_r1 - d1) * (length_r1 - d2) > 0.0)
            x = glyph->interval->angle_cw_rad();
          else x = glyph->interval->angle_ccw_rad();
        }
        glyph->angle_rad = x;
      }
    }






    // swapping action
    for (int i = 0; i < num_beads; i++)
    {
      int j = (i+1)%num_beads;

      NecklaceGlyph::Ptr& glyph = necklace->beads[i];
      NecklaceGlyph::Ptr& next = necklace->beads[j];

      const Number elem_radius = std::asin((scale_factor * glyph->radius_base + (FLAGS_min_separation / 2)) / necklace_radius);
      const Number next_radius = std::asin((scale_factor * next->radius_base + (FLAGS_min_separation / 2)) / necklace_radius);


      double newAngle1 = next->angle_rad + next_radius - elem_radius;
      double newAngle2 = glyph->angle_rad - elem_radius + next_radius;
      while (M_2xPI < newAngle1) newAngle1 -= M_2xPI;
      while (newAngle1 < 0) newAngle1 += M_2xPI;
      while (M_2xPI < newAngle2) newAngle2 -= M_2xPI;
      while (newAngle2 < 0) newAngle2 += M_2xPI;



      if (glyph->interval->IntersectsRay(newAngle1) && next->interval->IntersectsRay(newAngle2))
      {
        double mid1 = glyph->interval->ComputeCentroid();
        double mid2 = next->interval->ComputeCentroid();

        NecklaceInterval r1(glyph->angle_rad, mid1);
        if (r1.ComputeLength() > M_PI) r1 = NecklaceInterval(mid1, glyph->angle_rad);
        NecklaceInterval r2(next->angle_rad, mid2);
        if (r2.ComputeLength() > M_PI) r2 = NecklaceInterval(mid2, next->angle_rad);
        double dif1 = r1.ComputeLength() * r1.ComputeLength() + r2.ComputeLength() * r2.ComputeLength();

        r1 = NecklaceInterval(newAngle1, mid1);
        if (r1.ComputeLength() > M_PI) r1 = NecklaceInterval(mid1, newAngle1);
        r2 = NecklaceInterval(newAngle2, mid2);
        if (r2.ComputeLength() > M_PI) r2 = NecklaceInterval(mid2, newAngle2);
        double dif2 = r1.ComputeLength() * r1.ComputeLength() + r2.ComputeLength() * r2.ComputeLength();

        if (dif2 < dif1) {
          glyph->angle_rad = newAngle1;
          next->angle_rad = newAngle2;

          std::swap(necklace->beads[i], necklace->beads[j]);
        }
      }
    }
  }




  std::string out;
  writeDummySvg
  (
    elements,
    out
  );

  std::cout << out;

  if (FLAGS_debug_out)
  {
    std::ofstream fout("/storage/GeoViz/wwwroot/data/Example_wEU/test_out.xml");
    fout << out;
    fout.close();
  }
  return 0;
}
