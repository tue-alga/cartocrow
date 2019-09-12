#!/bin/bash
echo "Converting website icon..."

convert -resize x16 -gravity center -crop 16x16+0+0 wwwroot/res/b166_16-512.png -flatten -colors 256 -background transparent wwwroot/res/favicon.ico

echo "DONE"

