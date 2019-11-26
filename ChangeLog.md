=== 0.1.1.0 ===

* Initial DoxyGen documentation for the necklace map classes in source files.
* Moved SVG parsing functionality out of necklace map application source.
* Separated Necklace Map specific and generic SVG parsing functionality.
* Renamed `document.sh` to `build_docs.sh`.
* Indicate reasons for disabling pdflatex documentation.
* Minor optimizations to scripts.
* Changed CMake required version to 3.15.2: too cumbersome to support older versions of CMake.
* Added IP copyright placeholder to license boilerplate notices.


=== 0.1.0.0 ===

* Intial website scaffolding.
  - Plain HTML5 design.
  - CSS3 styling.
  - JavaScript for minor dynamic functionality.
  - Leaflet.js for map drawing.
* Initial codebase scaffolding.
  - C++ library and command-line executable.
  - gflags for command-line argument parsing.
  - glog for logging functionality.
  - CMake for configuring code and documentation.
* Google HTML/CSS style guide, Google JavaScript style guide, Google C++ style guide.
* Doxygen for generating documentation.
* PHP for calling executable.
* Bash scripts for building various parts of the package.

Note on version:
Versions are numbered as "MAJOR.MINOR.PATCH.FEATURE", although the feature patch is unlikely to be used very often, especially when sharing a newer version.
* MAJOR versions indicate a change in functionality that breaks backwards compatibility.
* MINOR versions indicate a change in functionality that preserves backwards compatibility.
* PATCH versions indicate a correction that does not change functionality (e.g. a bug fix).
* FEATURE version indicate an addition that does not change existing functionality.

Each version must be (reasonably) stable, meaning it has gone through unit/integration testing.



