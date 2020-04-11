# slowmoVideo

slowmoVideo is a tool that uses optical flow for generating slow-motion videos.
See [here][demos] for some demo videos.


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
```

### Building AppImage on Ubuntu 16.04

This guide shows how to build a slowmoVideo AppImage in a Docker container with [linuxdeployqt release][ldq-r],
in this example [version 6][ldq-6].

```bash
# Run a docker container and mount the current directory to /build in the container
docker run -it --rm -v $(pwd):/build ubuntu:16.04

# Install all packages that are required for building slowmoVideo
apt update
apt install wget build-essential cmake libopencv-dev qt5-default qttools5-dev-tools qtscript5-dev

# Get linuxdeployqt and make it executable
cd
wget https://github.com/probonopd/linuxdeployqt/releases/download/6/linuxdeployqt-6-x86_64.AppImage
chmod +x linuxdeployqt-6-x86_64.AppImage


# Build slowmoVideo
mkdir appimage-build
cd appimage-build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr
make

# Install slowmoVideo to the AppDir directory for AppImage
make install DESTDIR=AppDir

# Extract the linuxdeployqt AppImage when FUSE is not available in a docker container
~/linuxdeployqt-6-x86_64.AppImage --appimage-extract

# Create the AppImage
squashfs-root/AppRun AppDir/usr/share/applications/slowmoUI.desktop -appimage
```

[ldq-r]: https://github.com/probonopd/linuxdeployqt/releases
[ldq-6]: https://github.com/probonopd/linuxdeployqt/releases/download/6/linuxdeployqt-6-x86_64.AppImage



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
