
slowmoVideo can be compiled with MinGW, V3D with VisualStudio Express.
flowBuilder (V3D) crashes at the moment, i.e. it is not usable.


== General requirements
* CMake for Windows


== slowmoVideo requirements
All should be MinGW versions.
* Qt (Install the Qt SDK from http://qt.nokia.com/products)
* ffmpeg from http://ffmpeg.zeranoe.com/builds/
Headers -> slowmoVideo/libs/include
Libraries -> slowmoVideo/libs/lib
  

== V3D requirements
The following libraries are required (MSVC compiled):
* libpng
* libjpeg
* zlib 1.1.4 (not newer)
* SDL
* GLEW
* GLUT

.h files -> V3D/libs/include
.lib and .dll.a files -> V3D/libs/lib