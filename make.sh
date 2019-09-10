#!/bin/bash
SOURCE_DIR="$PWD/src"
BUILD_DIR="build"

# Create build directory if necessary.
if [ ! -d "$BUILD_DIR" ]; then
  mkdir $BUILD_DIR
fi

# Remeber the current directory to later return to it.
pushd . > /dev/null

# Run CMake to configure the build project.
# Note that these commands are chained
# to terminate early if anything goes wrong.
cd $BUILD_DIR && cmake $SOURCE_DIR && make && make install

popd > /dev/null

