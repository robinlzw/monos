# MONOS

MONOS computes the straight skeleton of a given monotone polygon.

# Algorithm

The algorithm is based on the following scientific work: https://doi.org/10.1016/j.ipl.2014.09.021
"A simple algorithm for computing positively weighted straight skeletons of monotone polygons"

# Input/Output

Reads GraphML (.graphml) or  wavefront (.obj) format that describes a polygon. Writes 
.obj format with 3D coordinates which can be imported into programs like
Blender.

# Requirements 
- C++17 enabled complier (gcc,clang)
- CGAL 
- Linux or Mac OS

# Installation

- git clone --recurse-submodules https://gitlab.cosy.sbg.ac.at/cg/ord/monos
- mkdir -p monos/build && cd monos/build
- cmake ..
- make -j 

# Usage

<code>monos [--verbose][--normalize] [--obj &lt;filename&gt;] &lt;filename&gt;</code>

| options        | description           |
| -------------:|:------------- |
|  --help           |         print help |
|  --verbose           |         verbose mode, shows information about the computation |
|  --normalize   | write output normalized to the origin |
|  --obj      |            write output in wavefront obj format (3D coordinates) |
|  &lt;filename&gt; |           input type is either wavefront obj or GML format |

# Submodules

- easyloggingpp -- https://github.com/muflihun/easyloggingpp
- gml -- https://gitlab.cosy.sbg.ac.at/cg/ord/gml


# License
monos is written in C++ and uses CGAL.  It computes the weighted straight
skeleton of a monotone polygon.
Copyright (C) 2018 - Günther Eder - geder@cs.sbg.ac.at

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
