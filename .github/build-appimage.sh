#!/bin/sh
#
# This file is intended to be run inside the Docker container.

VERSION=$(git describe HEAD || exit 1)
export VERSION
echo "Building AppImage for version ${VERSION}"

mkdir -p build
cd build || exit 1

# Default steps for building the AppImage
# Configure and build slowmoVideo
cmake .. -DCMAKE_INSTALL_PREFIX=/usr
make -j3

# Install slowmoVideo to the AppDir directory for AppImage
make install DESTDIR=AppDir

# Extract the linuxdeployqt AppImage when FUSE is not available in a docker container
/linuxdeployqt.AppImage  --appimage-extract

# Create the AppImage
squashfs-root/AppRun AppDir/usr/share/applications/slowmoUI.desktop -appimage

# Make the AppImage executable for everybody
# It belongs to root, and without chmod only root can modify it outside of the container
chmod 777 *.AppImage
