Description

This is a GPU implementation of feature point tracking with and without
simultaneous gain estimation (i.e. changes in the overall image brightness are
detected and handled). For a technical description see C. Zach, D. Gallup, and
J.-M. Frahm, "Fast Gain-Adaptive KLT Tracking on the GPU,". CVPR Workshop on
Computer Vision on GPU's (CVGPU), 2008, available at
http://cs.unc.edu/~cmzach/publications.html.

Additionally, this package now includes the TV-L1 optical flow implementation
as described in C.Zach, T. Pock, and H. Bischof, "A Duality Based Approach for
Realtime TV-L1 Optical Flow", Pattern Recognition (Proc. DAGM), vol. 4792 of
Lecture Notes in Computer Science, 2007 (also available at
http://cs.unc.edu/~cmzach/publications.html).

Using the simple API for tracking is demonstrated in
Apps/GL/klt_for_video.cpp, which successively reads frames from videos (using
the OpenCV library) and displays the obtained feature tracks. Note that you
have to set the V3D_SHADER_DIR environment variable to point to the Shader
directory, e.g.  export
V3D_SHADER_DIR=/home/user/src/GPU-KLT+FLOW-1.0/GL/Shaders with a sh/bash
environment. A simple application for TV-L1 optical flow is provided in
Apps/GL/tvl1_flow.cpp.

Requirements

NVidias Cg toolkit (version 2 or later,
http://developer.nvidia.com/object/cg_toolkit.html) and the OpenGL extension
wrangler library (glew.sourceforge.net) are required to build the
library. Currenlty, the GPU tracker is limited to NVidia hardware (Geforce6
series or newer, the hardware must support the fp40 Cg profile). If you want
to run the simple demo applications, the OpenCV computer vision library
(http://sourceforge.net/projects/opencvlibrary/) with sufficient support for
video codecs (e.g. through ffmpeg or xinelib) is required. The build system
uses cmake (www.cmake.org).

The optical flow code can use libpng or libjpeg for loading images. If none of
these is available (modify the main CMakeLists.txt), then binary PNM images
(magic code either P5 or P6) are still supported. GLUT or freeglut is used for
GL window handling.

The library was developed under Linux, but should compile equally well on
other operating systems.

-Christopher Zach (chzach@inf.ethz.ch)


/*
Copyright (c) 2008-2010 UNC-Chapel Hill & ETH Zurich

This file is part of GPU-KLT+FLOW.

GPU-KLT+FLOW is free software: you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free
Software Foundation, either version 3 of the License, or (at your option) any
later version.

GPU-KLT+FLOW is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
details.

You should have received a copy of the GNU Lesser General Public License along
with GPU-KLT+FLOW. If not, see <http://www.gnu.org/licenses/>.
*/
