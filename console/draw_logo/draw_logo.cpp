/*
The draw_logo application prints a logo formatted as an svg HTML element.
Copyright (C) 2021  Netherlands eScience Center and TU Eindhoven

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

Created by tvl (t.vanlankveld@esciencecenter.nl) on 09-10-2019
*/

#include <iostream>

#include "console/common/utils_cla.h"


int main(int argc, char **argv)
{
  InitApplication(argc, argv, "Command line application that prints a logo formatted as an svg HTML element.");

  std::cout << "<svg width='250' viewBox='0 0 200 85' xmlns='http://www.w3.org/2000/svg' version='1.1' bounds='[[52.356,4.945],[52.354,4.947]]'>";
  std::cout << "Sorry, your browser does not support the svg tag.";
  std::cout << "<defs>";
  std::cout << "<!-- Filter declaration -->";
  std::cout << "<filter id='MyFilter' filterUnits='userSpaceOnUse' x='0' y='0' width='200' height='120'>";
  std::cout << "<!-- offsetBlur -->";
  std::cout << "<feGaussianBlur in='SourceAlpha' stdDeviation='4' result='blur' />";
  std::cout << "<feOffset in='blur' dx='4' dy='4' result='offsetBlur' />";
  std::cout << "<!-- litPaint -->";
  std::cout << "<feSpecularLighting in='blur' surfaceScale='5' specularConstant='.75' specularExponent='20' lighting-color='#bbbbbb' result='specOut'>";
  std::cout << "<fePointLight x='-5000' y='-10000' z='20000' />";
  std::cout << "</feSpecularLighting>";
  std::cout << "<feComposite in='specOut' in2='SourceAlpha' operator='in' result='specOut' />";
  std::cout << "<feComposite in='SourceGraphic' in2='specOut' operator='arithmetic' k1='0' k2='1' k3='1' k4='0' result='litPaint' />";
  std::cout << "<!-- merge offsetBlur + litPaint -->";
  std::cout << "<feMerge><feMergeNode in='offsetBlur' /><feMergeNode in='litPaint' /></feMerge>";
  std::cout << "</filter>";
  std::cout << "</defs>";
  std::cout << "<!-- Graphic elements -->";
  std::cout << "<g filter='url(#MyFilter)'>";
  std::cout << "<path fill='none' stroke='#D90000' stroke-width='10' d='M50,66 c-50,0 -50,-60 0,-60 h100 c50,0 50,60 0,60z'/>";
  std::cout << "<path fill='#D90000' d='M60,56 c-30,0 -30,-40 0,-40 h80 c30,0 30,40 0,40z' />";
  std::cout << "<g fill='#FFFFFF' stroke='black' font-size='45' font-family='Verdana'><text x='52' y='52'>SVG</text></g>";
  std::cout << "</g>";
  std::cout << "</svg>";
  std::cout << std::endl;

  return 0;
}
