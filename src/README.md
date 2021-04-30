# GeoViz README

This repository gives a C++ library for geographical visualization algorithms.
Note that this implementation depends on quick processing times. Heavier processing may take longer than the allowed response delay, in which case more complex processing control may be required.

This library has been developed according to the following design principles to promote usability, sustainability, and platform independence:

- Separation of functional code and user interface code.
- Project configuration using CMake.
- Separate libraries and applications per method.

## Licensing

Copyright 2021 Netherlands eScience Center and TU Eindhoven
Licensed under the GPLv3.0 license. See LICENSE for details.

# Dependencies

For the code dependencies, see the top level README.md

# Code structure

For the code structure and implementation guidelines, see CONTRIBUTE.md

# Connection to website

This library can be used as the backend to the website shipped in this repository. These two are connected using PHP scripts, as described in CONTRIBUTE.md
