# CartoCrow - A framework for cartographic visualization algorithms

<img align="right" src="https://user-images.githubusercontent.com/7533280/122964753-ddca4b00-d387-11eb-8320-7ba7bbb7e496.png">

CartoCrow is a framework that simplifies the implementation of algorithms in cartographic visualization. It allows researchers to experiment with these algorithms and use them to generate maps. The framework behind CartoCrow can be used to run other cartography algorithms online. CartoCrow consists of a C++ library (this repository) which provides a set of command-line applications, and a web interface (see [cartocrow-web](https://github.com/tue-alga/cartocrow-web)) which allows end users to generate maps in a user-friendly way.


## Dependencies

CartoCrow depends on the following projects:

* g++ (9.3.0) / clang++ (10.0.0) / MSVC (2019)
* CMake (3.15)
* CGAL (5.0)
* gflags (2.2.2)
* glog (0.5.0)
* ipelib (7.2.24)
* Qt (5.15)
* tinyxml2 (9.0.0)
* unittest-cpp (2.0.0)

The version numbers listed are indicative. Newer (and possibly somewhat older) versions will most likely work as well.


### Windows



On Windows systems, we recommend using [vcpkg](https://github.com/microsoft/vcpkg) to install and manage dependencies. The following steps install everything necessary to build CartoCrow.

* **MSVC.** Download MSVC 2019 from [Microsoft's website](https://docs.microsoft.com/en-us/visualstudio/releases/2019/release-notes) and install it.

* **CMake.** Download CMake from [here](https://cmake.org/download/) and install it. (Note: If you have a version of CMake installed in Cygwin, this does not seem to play well with vcpkg. Please install a native version of CMake.)

* **vcpkg.** The standard procedure to setup vcpkg on Windows:

  ```sh
  git clone https://github.com/microsoft/vcpkg
  cd vcpkg
  .\bootstrap-vcpkg.bat
  ```

  In our experience, vcpkg may misbehave when installed in a directory with a long path name, or a path name containing exotic characters. vcpkg itself recommends `C:\src\vcpkg`.

  For more information on installing vcpkg, see [here](https://github.com/microsoft/vcpkg#quick-start-windows).

* **Install dependencies.** As described [here](https://doc.cgal.org/latest/Manual/windows.html#title0):

  ```sh
  vcpkg install cgal:x64-windows
  vcpkg install qt5:x64-windows
  vcpkg install gflags:x64-windows
  vcpkg install glog:x64-windows
  vcpkg install tinyxml2:x64-windows
  vcpkg install unittest-cpp:x64-windows
  ```

  This step can take a very long time, especially compiling CGAL (around 30 minutes) and Qt (around 2 hours).

* **Ipelib.** This library is not available in vcpkg, so we will have to build it ourselves. Unfortunately, the [upstream version](https://github.com/otfried/ipe/releases/download/v7.2.24/ipe-7.2.24-src.tar.gz) of ipelib does not compile cleanly with MSVC. We prepared a patched version *(to do: link coming soon)* that can be compiled and installed with
```
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=install -DCMAKE_TOOLCHAIN_FILE=<path-to-vcpkg>\scripts\buildsystems\vcpkg.cmake -S . -B build
cmake --build build
sudo cmake --install build
```


### Linux

On Ubuntu, most dependencies can be obtained from the repository:

```sh
sudo apt install build-essential cmake
sudo apt install libcgal-dev libgflags-dev
```

The remaining dependencies need to be built manually.

* **glog and tinyxml2.** These are built manually because Ubuntu's packaging apparently does not include the CMake files we need.

  ```sh
  git clone git clone https://github.com/google/glog.git
  cmake -S . -B build
  cmake --build build
  sudo cmake --install build

  git clone git@github.com:leethomason/tinyxml2.git
  cmake -S . -B build
  cmake --build build
  sudo cmake --install build
  ```

* **unittest-cpp.** Also needs to be built manually, because it defines several macros that overlap with macros defined by glog. This causes issues when also compiling the unit tests of `unittest-cpp`, hence we turn these off.
  ```sh
  git clone -b v2.0.0 https://github.com/unittest-cpp/unittest-cpp
  cmake -S . -B build -DUTPP_INCLUDE_TESTS_IN_BUILD=OFF
  cmake --build build
  sudo cmake --install build
  ```

* **Ipelib.** Download the [source archive](https://github.com/otfried/ipe/releases/download/v7.2.24/ipe-7.2.24-src.tar.gz), unpack it, and compile and install it using the instructions given in `install.txt`.


## Compiling

CartoCrow uses CMake as its build system and can therefore be built like any other CMake application, for example:

**Windows**
```sh
cmake.exe -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=<install-directory> -DCMAKE_TOOLCHAIN_FILE=<path-to-vcpkg>\scripts\buildsystems\vcpkg.cmake -S . -B build
cmake.exe --build build
cmake.exe --install build
```

**Linux**
```sh
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=<install-directory> -S . -B build

cmake --build build
cmake --install build
```

where `<install-directory>/bin` is the directory where the executables will be installed. Note that on Windows, it is necessary to supply the `CMAKE_TOOLCHAIN_FILE` generated by vcpkg; see the [vcpkg documentation](https://github.com/microsoft/vcpkg/blob/master/docs/users/integration.md#cmake-toolchain-file-recommended-for-open-source-cmake-projects) for details.

If you want to use [cartocrow-web](https://github.com/tue-alga/cartocrow-web), clone that repository to a separate directory, and use that directory as `<install-directory>`, so that the executables are installed in a location where the web application can find them. (See also the [README](https://github.com/tue-alga/cartocrow-web/blob/master/README.md) in the [cartocrow-web](https://github.com/tue-alga/cartocrow-web) repository for details.)


## Usage

### Necklace map

To generate a necklace map, you need a map (in SVG format) and a data file:

```bash
build/console/necklace_map/necklace_map_cla --in_geometry_filename=<map_file> --in_data_filename=<data_file> --in_value_name=<column_name> --out_filename=<output_file>
```

We provide some sample input data to experiment with:

```bash
build/console/necklace_map/necklace_map_cla --in_geometry_filename=data/necklace_map/wEU.svg --in_data_filename=data/necklace_map/wEU.txt --in_value_name=value --out_filename=test.svg
```


## License

Copyright (c) 2019-2021 Netherlands eScience Center and TU Eindhoven
Licensed under the GPLv3.0 license. See LICENSE for details.

