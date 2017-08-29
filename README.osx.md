### Building for MacOS

here, I will describe how to build slowmoVideo for OSX from scratch.
you will need of course *Xcode* and *command line tools*, with *cmake*
will need some dependencies :

* glew (glew-1.10.0)
* ffmpeg (ffmpeg-2.2)
* jpeg (jpeg-9a)
* libpng (libpng-1.6.10)
* zlib (zlib-1.2.8)
* yasm (for ffmpeg) (yasm-1.2.0)
* opencv (opencv-2.4.8)
* qt4 (qt 4.8.5)
* x264 for ffmpeg


1- you need to specify where to find some libraries for cmake :
```export QTDIR=/Users/val/Documents/Sources/qt4
export FFMPEGDIR=/Users/val/Documents/Sources/ffmpeg
```

2- run cmake :
```
cmake ../slowmoVideo/src -DCMAKE_INSTALL_PREFIX=/Users/val/Applications/slowmoVideo -DQTDIR=/Users/val/Documents/Sources/qt4 -DQT_MAKE_EXECUTABLE=/Users/val/Documents/Sources/qt4/bin/qmake -DOpenCV_DIR=/Users/val/Documents/Sources/opencv/share/OpenCV -DGLEW_INCLUDE_DIR=/Users/val/Documents/Sources/slowlib/include -DGLEW_LIBRAIRIES=/Users/val/Documents/Sources/slowlib/lib/libGLEW.a -DJPEG_INCLUDE_DIR=/Users/val/Documents/Sources/slowlib/include -DJPEG_LIBRARY=/Users/val/Documents/Sources/slowlib/lib/libjpeg.a -DFFMPEG_LIBRARY_DIR=/Users/val/Documents/Sources/ffmpeg/lib -DFFMPEG_INCLUDE_PATHS="/Users/val/Documents/Sources/ffmpeg/include"
```

check if cmake find all the needed part. As in my case some library where not found, so I have to specify them in CMakeCache.txt directly …
they where : glew libraries and libswcale !


* if all is ok you can run `make ; make install`

you will have some warning during compilation…
you should now have a working GUI application bundle for MacOS in your install target directory.

