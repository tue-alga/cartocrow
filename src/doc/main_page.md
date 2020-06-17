\mainpage Algorithmic Geographic Visualization Library

<a href="/index.html">GeoViz webpage</a>


\section sec_introduction Introduction

This library contains several algorithms for geographic visualization developed by the AGA group of TU Eindhoven. Specifically, algorithms for constructing Necklace Maps and Flow Maps.
 
Details on the papers at the basis of this code can be found in the main functional methods for these library modules.
* Necklace Maps: \ref necklace_scale_factor "ComputeScaleFactor" and \ref necklace_placement "ComputePlacement".
* Flow Diagrams: 


\section sec_install Installation

This library has several third party dependencies that must be installed before building the code. Check the README.md file in the root directory for more information.


\subsection ssec_ubuntu Ubuntu

Most third party dependencies are known to ubuntu's package manager and can be installed using the normal linux install procedure:

    sudo apt install build-essential cmake libcgal-dev doxygen libgoogle-glog-dev libgflags-dev imagemagick-6.q16 texlive-xetex php7.2-cli libtinyxml2-dev

The dependency for unit testing the code (unittest-cpp) requires manually cloning, building, and installation (see https://github.com/unittest-cpp/unittest-cpp):

    git clone -b v2.0.0 https://github.com/unittest-cpp/unittest-cpp
    cd unittest-cpp/builds/
    cmake -DUTPP_INCLUDE_TESTS_IN_BUILD=OFF ../
    make && sudo make install

Note that unittest-cpp defines several macros that overlap with macros defined for logging. This mainly comes into play when also compiling the unit tests of unittest-cpp. This option is disabled in the above example calls; enabling it may require some manual changes to the unittest-cpp code and compilation settings.

After installing these dependencies, you can call CMake to generate the project files and build them in your preferred IDE. Alternatively, you could use the scripts supplied in the root directory to build and install the source:

    ./rebuild_all.sh
    ./serve.sh 
