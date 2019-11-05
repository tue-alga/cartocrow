/*
Application to expose the functionality of the library by the same name.
Copyright (C) 2019  Netherlands eScience Center and [IP's institution]

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

#include <iostream>
#include <map>
#include <memory>
#include <tinyxml2.h>
#include <vector>

#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
//#include <gflags/gflags.h>
#include <glog/logging.h>

#include <cmake/geoviz_config.h>
#include <geoviz/necklace_map/necklace_map.h>

#include "console/common/utils_cla.h"


// When reading example data:
// .mrk files: number of IDs; per line: ID.
// .txt files: number of countries; per country: ID, name, necklace value, 'flow value'
// .ipe files: country and necklace shapes (check java implementation for format...)
// as input, I expect either an XML file with SVG content, or )later on) that content as command line argument...

using namespace geoviz::necklace_map;  //TODO(tvl) remove: name types explicitly!

class SvgInputVisitor : public tinyxml2::XMLVisitor
{
 public:
  SvgInputVisitor
  (
    std::vector<Region>& regions,
    std::shared_ptr<NecklaceType>& necklace,
    std::map<std::string, size_t>& id_to_index
  )
  : regions_(regions), necklace_(necklace), id_to_index_(id_to_index)
  {}


  bool 	VisitEnter
  (
    const tinyxml2::XMLElement& element,
    const tinyxml2::XMLAttribute* attribute
  ) {
    const std::string elem_name = element.Name();
    if (elem_name != "path") return true;

    bool necklace = false;
    std::string path = "";
    std::string id = "";

    for (; attribute != nullptr; attribute = attribute->Next())
    {
      const std::string name = attribute->Name();
      const std::string value = attribute->Value();
      if (name == "d")
        path = value;
      else if (name == "id")
        id = value;
      else if (name == "style" && value.find("stroke-dasharray") != std::string::npos)
        necklace = true;
    }

    CHECK_NE(path, "");

    if (necklace)
      AddNecklace(path);
    else
    {
      if (id.empty()) return true;
      AddRegion(path, id);
    }

    return true;
  }

 private:
  void AddNecklace(const std::string& path)
  {

  }

  void AddRegion(const std::string& path, const std::string& id)
  {

  }

  std::vector<Region>& regions_;
  std::shared_ptr<NecklaceType>& necklace_;
  std::map<std::string, size_t>& id_to_index_;
};


bool readSvg
(
  const std::string& filename,
  std::vector<Region>& regions,
  std::shared_ptr<NecklaceType>& necklace
)
{
  // Note that while the SVG input is stored in an XML file, it does not make use of the main feature that makes XML preferable over JSON: validatibility (i.e. assigning a schema).
  // This means that we do not need to use a comprehensive XML library such as xerces; we can instead use a lightweight library such as tinyxml.

  tinyxml2::XMLDocument doc;
  tinyxml2::XMLError result = doc.LoadFile( filename.c_str() );
  //tinyxml2::XMLError result = doc.Parse( content ); // This would be how to parse a string using tinyxml2
  if (result != tinyxml2::XML_SUCCESS) return false;

  std::map<std::string, size_t> id_to_index;

  SvgInputVisitor visitor
    (
      regions,
      necklace,
      id_to_index
    );

  doc.Accept(&visitor);


  return true;
}


int main(int argc, char **argv)
{
  initApplication
  (
    argc,
    argv,
    "Command line application that exposes the functionality of the GeoViz necklace map.",
    {"<some arg>", "<another arg>"}
  );

  // Writing to the standard output is reserved for text that should be returned to a calling website.
  FLAGS_logtostderr = true;

  std::cerr << "Test" << std::endl;

  std::vector<Region> regions;
    std::shared_ptr<NecklaceType> necklace;
  readSvg
  (
    "/storage/GeoViz/wwwroot/data/Example_wEU/wEU_svg.xml",
    regions,
    necklace
  );



  using K = CGAL::Exact_predicates_inexact_constructions_kernel;
  using Point_3 = K::Point_3;


  Point_3 p(0,1,2.3);




  // Note that here is a good place to check the validity of the flags.
  // While this can be done by adding flag validators using gflags,
  // These generally only allow validating independent flags,
  // they do not allow validating programmatically set flags,
  // and they may throw inconvenient compiler warnings about unused variables
  // (depending on the implementation).

  LOG(INFO) << "Args:";
  for (int i = 1; i < argc; ++i)
    LOG(INFO) << "\t" << argv[i];

  LOG(INFO) << "Necklace map: " << geoviz::proc_necklace_map();

  LOG(INFO) << "GeoViz version: " << GEOVIZ_VERSION;

  std::cout << "<div>";
  std::cout << "<!--To be loaded as support card.-->";
  std::cout << "<h1>Application-based component</h1>";
  std::cout << "<p>This page is just to test running a server-side application using PHP.</p>";
  std::cout << "<p>The following command line arguments were caught:<br>";
  for (int i = 1; i < argc; ++i)
    std::cout << argv[i] << "<br>";
  std::cout << "</p>";
  std::cout << "</div>";

  return 0;
}
