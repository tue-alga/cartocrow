#!/bin/bash
BUILD_DIR="build"
LATEX_DIR="$PWD/wwwroot/latex/"

echo "Generating documentation..."

# Remember the current directory to later return to it.
pushd . > /dev/null

# Run Doxygen to generate the HTML documentation and LaTeX sources.
cd $BUILD_DIR && doxygen

# Run LaTeX to generate the PDF documentation.
# Note that Doxygen PDF documentation is generally very ugly.
# Also note that LaTeX needs to run twice to fix references.
# Finally note that the "yes" command will input an empty string
# on each prompt for input from xelatex.
# While this is convenient, because doxygen invariably generates
# imperfect tex sources, it may be dangerous in some cases.
#Disabled due to dependencies and because the output is unlikely to add value.
#cd $LATEX_DIR && yes "" | xelatex refman.tex && yes "" | xelatex refman.tex

popd > /dev/null

echo "DONE"

