#!/bin/sh
#
# This file is intended to be run inside the Docker container.

# This script copies the built AppImage to /__build, so if the container is run with a volume mount on /__build,
# the AppImage will be copied to the host.
mkdir -p /__build

mkdir /slowmoVideo/build
cd /slowmoVideo/build

# Default steps for building the AppImage
cmake .. -DCMAKE_INSTALL_PREFIX=/usr
make -j9
make install DESTDIR=AppDir
/linuxdeployqt.AppImage  --appimage-extract
export VERSION=$(git describe HEAD)
squashfs-root/AppRun AppDir/usr/share/applications/slowmoUI.desktop -appimage

chmod 777 *.AppImage
cp *.AppImage /__build
