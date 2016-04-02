slowmoVideo
===========

Hello! This is a short introduction for you if you want to:
- compile
- develop
- translate

slowmoVideo. For everything else please go to the 
[web page](http://slowmoVideo.granjow.net) or the 
[Google+ group](https://plus.google.com/communities/116570263544012246711).

Building
--------

### Building for Linux
http://slowmovideo.granjow.net/download.php

### Building for Windows
Compiling slowmoVideo for Windows using MXE on Linux:

1.  Get mxe _not_ from http://mxe.cc/ BUT, as long as OpenCV is not in the official branch, from
    https://github.com/Granjow/mxe/tree/opencv (Changes by Christian Frisson)
3.  Build opencv, qt, ffmpeg
    and copy the fixed CMake file (avoids library names like liblibjasper) with:
    `$ cp replaceOnTime/OpenCVConfig.cmake usr/i686-pc-mingw32/`
4.  Run cmake for slowmoVideo, but now give a toolchain file:
    `cmake .. -DCMAKE_TOOLCHAIN_FILE=/PATH_TO_MXE/usr/i686-pc-mingw32/share/cmake/mxe-conf.cmake`
5.  Compile!

### Building for MacOS
take a look at README.osx for more detailed instruction

#### Notes
Additionally to slowmoVideo, ffmpeg.exe (32-bit build, static) is required.
Download it from http://ffmpeg.zeranoe.com/builds/ and put it into the same directory as slowmoUI.exe.


Translating
-----------

For this you should be in the slowmoVideo subdirectory which contains the tr/ directory. 
The tools (`linguist`, `lupdate`, `lrelease`) are available in the `qt4-dev-tools` package for Debian based systems.

### Adding your language
To add your language xx (like fr, it), run the following command to generate the respective .ts file:

    lupdate . -ts tr/slowmoVideo_xx.ts
    
After this you can start translating. To make slowmoVideo actually use the translation, add this entry
to `slowmoUI/resources.qrc`:

    <qresource lang="xx">
        <file alias="translations.qm">../tr/slowmoVideo_xx.qm</file>
    </qresource>

### Translation
First, run `lupdate` to get the newest strings to translate from the code. 
(Otherwise you might be translating something that does not even exist anymore.)

Then the .ts file can be translated, preferrably with qt’s Linguist, or with any other 
translation tool you like.

Finally, to see your translation “in action”, release the .ts file (this creates a .qm file).

    lupdate src/ -ts tr/slowmoVideo_xx.ts
    linguist tr/slowmoVideo_xx.ts
    lrelease tr/slowmoVideo_xx.ts

Now you can push your `.ts` file to git.

