# Example website with C++ back-end

This repository gives a minimal starting point for deploying a C++ back-end that generates data to be shown on a geographical map in a website.
The back-end is contained in the `./src/` subdirectory. The front-end is contained in the `./wwwroot/` subdirectory.
Both of these subdirectories contain a more specific README.md file describing some particularities and a LICENSE file with the license that applies to that part.

This directory contains several linux bash scripts to simplify building and using the package. These are meant to be called from the current (root) directory.
* `clean.sh` removes the things generated by the other scripts.
* `convert_favicon.sh` converts the image `./wwwroot/res/b166_16-512.png` into the webpage icon.
* `document.sh` generates DoxyGen documentation based on the comments in the back-end source code.
* `make.sh` compiles the back-end applications and installs the source code into their desired subdirectories of `./wwwroot/`.
* `rebuild_all.sh` runs all the above scripts to regenerate the back-end.
* `serve.sh [port#]` starts serving the website on localhost. By default port 8000 is used.

To generate and start serving the site (note that these scripts expect to be called from the current directory):

```sh
./rebuild_all.sh
./serve.sh
```

The code depends on several packages that may need to be installed before it can compile successfully:
* CMake
* convert
* CGAL
* DoxyGen
* gflags
* glog
* latex
* php
* tinyxml2
* unittest-cpp

On Ubuntu, most can be installed by running:

```sh
sudo apt install build-essential cmake libcgal-dev doxygen libgoogle-glog-dev libgflags-dev imagemagick-6.q16 texlive-xetex php7.2-cli libtinyxml2-dev
```

unittest-cpp requires manually cloning, building, and installation (see https://github.com/unittest-cpp/unittest-cpp):
```sh
git clone -b v2.0.0 https://github.com/unittest-cpp/unittest-cpp
cd unittest-cpp/builds/
cmake ../
make && sudo make install
```

To run unit tests (from the current directory):

```sh
./test.sh
```

