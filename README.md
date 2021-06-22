# CartoCrow - A framework for cartographic visualization algorithms

<img align="right" src="https://user-images.githubusercontent.com/7533280/122964753-ddca4b00-d387-11eb-8320-7ba7bbb7e496.png">

CartoCrow is a framework that simplifies the implementation of algorithms in cartographic visualization. It allows researchers to experiment with these algorithms and use them to generate maps. The framework behind CartoCrow can be used to run other cartography algorithms online. CartoCrow consists of a C++ library (this repository) which provides a set of command-line applications, and a web interface (see [cartocrow-web](https://github.com/tue-alga/cartocrow-web)) which allows end users to generate maps in a user-friendly way.


## Dependencies

CartoCrow depends on the following packages:

* CMake
* convert
* CGAL
* Doxygen
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

`unittest-cpp` requires manually cloning, building, and installing (see https://github.com/unittest-cpp/unittest-cpp):
```sh
git clone -b v2.0.0 https://github.com/unittest-cpp/unittest-cpp
cd unittest-cpp/builds/
cmake -DUTPP_INCLUDE_TESTS_IN_BUILD=OFF ../
make && sudo make install
```
Note that `unittest-cpp` defines several macros that overlap with macros defined by glog. This causes issues when also compiling the unit tests of `unittest-cpp`, hence we turn these off.

To run unit tests (from the current directory):


## Compiling

CartoCrow uses CMake as its build system and can therefore be built as any other CMake application, for example:

```sh
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=<install-directory> -S . -B <build-directory>
cmake --build <build-directory>
cmake --install <build-directory>
```

where `<build-directory>` is an arbitrary directory (will be created if non-existing) and `<install-directory>/bin` is the directory where the executables will be installed. If you want to use [cartocrow-web](https://github.com/tue-alga/cartocrow-web), clone that repository to a separate directory, and use that directory as `<install-directory>`, so that the executables are installed in a location where the web application can find them. (See also the [README](https://github.com/tue-alga/cartocrow-web/blob/master/README.md) in the [cartocrow-web](https://github.com/tue-alga/cartocrow-web) repository for details.)


## Testing

To run unit tests, run the `test.sh` script. To generate Doxygen documentation, run `document.sh`.


## Usage

### Necklace map

To generate a necklace map, you need a map (in SVG format) and a data file:

```bash
build/console/necklace_map/necklace_map_cla --in_geometry_filename=<map_file> --in_data_filename=<data_file> --in_value_name=<column_name> --out_filename=<output_file>
```

We provide some sample input data to experiment with:

```bash
build/console/necklace_map/necklace_map_cla --in_geometry_filename=data/necklace_map/wEU.svg --in_data_filename=data/necklace_map/wEU.txt --in_value_name=value
```


## License

Copyright (c) 2019-2021 Netherlands eScience Center and TU Eindhoven
Licensed under the GPLv3.0 license. See LICENSE for details.

