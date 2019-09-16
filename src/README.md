=== GeoViz code library ===

This directory contains the GeoViz C++ library. The following descibes the subdirectory structure:

- cmake : configuration and template files for CMake.
- console : command-line applications to expose the functionality of the library.
- geoviz : all the functional code. Note that I/O methods can be found with the relevant applications.

This library has been developed according to the following design principles to promote usability, sustainability, and platform independence:

- Separation of functional code and user interface code.
- Project configuration using CMake.
- Separate libraries and applications per method.

Note that within the CMakeLists files, we follow a loose naming convention:
- All variables are UPPER_CASE, all methods are lower_case.
- All project-specific CMake build options have the GEOVIZ_ prefix.
- Similar to CMake, most variables are [project_name]_<target_name>_<type> (e.g. GEOVIZ_INSTALL_SOURCE_DIR or NECKLACE_MAP_TARGET)._
- Applications exposing the functionality of an individual library are suffixed by _CLA (for command-line application).