# Example minimal website with map and C++ backend.

This website is meant to be a minimal starting point for deploying a C++ back-end that generates data to be shown on a geographical map.

This website has been developed with design principles to promote responsiveness and good mobile usability:

- Mobile-first stylesheets.
- Minimize 'clutter'; maximize 'content'.
- Show something soon; indicate more is coming.
- Minimize reloading heavy content.

## Licensing

Copyright 2019 Netherlands eScience Center
Licensed under the Apache License, version 2.0. See LICENSE for details.

This website depends on the GeoViz software library distributed together with this website.
The GeoViz software library is Licensed under the GPLv3.0 license. See its LICENSE for details.

## Directory structure

This directory contains the example website.
Subdirectories are organized as follows:

- bin : native (server-side C++) applications.
- data : data stores such as databases.
- html : doxygen-generated HTML documentation.
- include : source code for native applications.
- latex : docygen-generated LaTeX docuementation.
- lib : native (server-side C++) libraries.
- page : referenced webpages.
- res : webpage resources like images and icons.
- script : javascript and other (client-side) scripts.
- style : stylesheets.

## Style

The HTML and CSS webpage files follow the Google HTML/CSS style guide (https://google.github.io/styleguide/htmlcssguide.html) with the following changes:

- Ignore section 3.1.7 on optional elements: always keep the <head> and <body> tags for separation of concerns (e.g. separating metadata from data).
- Ignore section 4.1.8 on hexadecimal notation: always use 6- or 8-character notation. Shorter notations obfuscate the actual value.
- All style classes for general use within the group use the "aga-" prefix. All style classes for specific use within a project use a short prefix based on the project name.

The JavaScript files follow the Google JavaScript style guide (https://google.github.io/styleguide/jsguide.html) with the following changes:

- Ignore section 9.3.2 on using clang-format. Instead, when using VS Code, use the Prettier plugin to format the code. One of the reasons is that clang-format violates section 4.1.2 on closing brace placement.
- Instead of using 4+ spaces fo continuation indentation (section 4.5.2), continuations are idented by 2 spaces compared to the original.
- Whenever spreading function parameters/arguments over multiple lines, always put the first one on a new line. Prefer to use exactly 1 parameter/argument per line.
- When naming elements in camel case, only capitalize the first letter of an abbreviation.
