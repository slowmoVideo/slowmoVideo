# slowmoVideo

slowmoVideo is a tool that uses optical flow for generating slow-motion videos.
See [here][demos] for some demo videos.

For the last changes, see the [Changelog](CHANGELOG.md).


## Building

slowmoVideo uses CMake for building. You may also want to build [V3D Flow Builder][v3d]
for fast GPU based rendering.

Dependencies on Ubuntu 19.10…16.04:

    build-essential cmake libopencv-dev qt5-default qttools5-dev-tools qtscript5-dev

### Building for Linux

```bash
git submodule update --init

mkdir build
cd build

cmake ..
make

# Run it
src/slowmoUI/slowmoUI
```

### Building AppImage on Ubuntu 16.04

This guide shows how to build a slowmoVideo AppImage in a Docker container with
[linuxdeployqt release][ldq-r], in this example [version 6][ldq-6].

See [Packaging native binaries][ai] for more information on AppImage packaging.

```bash
# Create the Docker image from the Dockerfile in this directory
# This can take some time as it will build OpenCV from source.
cd docs
docker build . -name sv-appimage-builder
cd ..

# Run the container to build slowmoVideo from the current directory.
#
# This will build the AppImage and copy it to /__build.
# With the volume mount, the AppImage will be available on the host. 
mkdir appimage
docker run -it --rm -v $(pwd):/slowmoVideo -v $(pwd)/appimage:/__build sv-appimage-builder
```

If you want to compile a different version, run the steps manually inside the
container, for example:

```bash
docker run -it --rm -v $(pwd)/sv-appimage:/__build sv-appimage-builder bash
cd slowmoVideo
git checkout v0.6
cd /
./docker-build-appimage.sh
exit
```

[ldq-r]: https://github.com/probonopd/linuxdeployqt/releases
[ldq-6]: https://github.com/probonopd/linuxdeployqt/releases/download/6/linuxdeployqt-6-x86_64.AppImage
[ai]: https://docs.appimage.org/packaging-guide/from-source/native-binaries.html


----

*Content after here is not up-to-date. It may still work, but no guarantee!*


### Building for Windows

*This guide is outdated.*

Compiling slowmoVideo for Windows using MXE on Linux:

1.  Get mxe _not_ from http://mxe.cc/ BUT, as long as OpenCV is not in the official branch, from
    https://github.com/Granjow/mxe/tree/opencv (Changes by Christian Frisson)
3.  Build opencv, qt, ffmpeg
    and copy the fixed CMake file (avoids library names like liblibjasper) with:
    `$ cp replaceOnTime/OpenCVConfig.cmake usr/i686-pc-mingw32/`
4.  Run cmake for slowmoVideo, but now give a toolchain file:
    `cmake .. -DCMAKE_TOOLCHAIN_FILE=/PATH_TO_MXE/usr/i686-pc-mingw32/share/cmake/mxe-conf.cmake`
5.  Compile!

#### Notes

Additionally to slowmoVideo, ffmpeg.exe (32-bit build, static) is required.
Download it from http://ffmpeg.zeranoe.com/builds/ and put it into the same directory as slowmoUI.exe.

### Building for MacOS

take a look at README.osx for more detailed instruction


## Translating

For this you should be in the `src` subdirectory which contains the `tr/` directory. 
The tools (`linguist`, `lupdate`, `lrelease`) are available in the `qttools5-dev-tools` package for Debian based systems.

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


[demos]: http://slowmovideo.granjow.net/videos.html
[v3d]: https://github.com/slowmoVideo/v3d-flow-builder
