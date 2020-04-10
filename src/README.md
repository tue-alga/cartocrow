# GeoViz README

This library is a minimal (dummy) C++ library meant to demonstrate functionality that can be called from a website and return new content for that website.
Note that this implementation depends on quick processing times. Heavier processing may take longer than the allowed response delay, in which case more complex processing control may be required.

This library has been developed according to the following design principles to promote usability, sustainability, and platform independence:

- Separation of functional code and user interface code.
- Project configuration using CMake.
- Separate libraries and applications per method.

## Licensing

Copyright 2019 Netherlands eScience Center and TU Eindhoven
Licensed under the GPLv3.0 license. See LICENSE for details.

## Directory structure

This directory contains the GeoViz C++ library.
Subdirectories are organized as follows:

- cmake : configuration and template files for CMake.
- console : command-line applications to expose the functionality of the library.
- geoviz : all the functional code. Note that I/O methods can be found with the relevant applications.
- test : unit testing applications.

# Dependencies

For the code dependencies, see the top level README.md
