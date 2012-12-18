slowmoVideo
===========

Building for Linux
------------------
http://slowmovideo.granjow.net/download.php

Building for Windows
--------------------
Compiling slowmoVideo for Windows using MXE on Linux:
1. Get mxe not from http://mxe.cc/ BUT, as long as OpenCV is not in the official branch, from
   https://github.com/Granjow/mxe/tree/opencv (Changes by Christian Frisson)
3. Build opencv, qt, ffmpeg
   and copy the fixed CMake file (avoids library names like liblibjasper) with:
   $ cp replaceOnTime/OpenCVConfig.cmake usr/i686-pc-mingw32/
4. Run cmake for slowmoVideo, but now give a toolchain file:
   cmake .. -DCMAKE_TOOLCHAIN_FILE=/PATH_TO_MXE/usr/i686-pc-mingw32/share/cmake/mxe-conf.cmake
5. Compile!


Notes
-----
Additionally to slowmoVideo, ffmpeg.exe (32-bit build, static) is required.
Download it from http://ffmpeg.zeranoe.com/builds/ and put it into the same directory as slowmoUI.exe.
