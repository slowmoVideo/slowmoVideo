#!/bin/sh
#
# This file is intended to be run inside the Docker container.
#
# Expected volume mounts:
# /slowmoVideo (Git repo)
# /__build (build directory, AppImage is written to here too)

# Ensure slowmoVideo exists
if [ ! -e /slowmoVideo ]
then
  git clone https://github.com/slowmoVideo/slowmoVideo.git /slowmoVideo
  cd /slowmoVideo || exit 1
  git submodule update --init
  cd ..
fi

# Ensure the build directory exists in the container
if [ ! -e /__build ]
then
  mkdir -p /__build
fi

# Set the version from Git
cd /slowmoVideo || exit 1
pwd
VERSION=$(git describe HEAD || exit 1)
export VERSION
echo "Building AppImage for version ${VERSION}"


cd /__build || exit 1

# Default steps for building the AppImage
# Configure and build slowmoVideo
cmake /slowmoVideo -DCMAKE_INSTALL_PREFIX=/usr
make -j9

# Install slowmoVideo to the AppDir directory for AppImage
make install DESTDIR=AppDir

# Extract the linuxdeployqt AppImage when FUSE is not available in a docker container
/linuxdeployqt.AppImage  --appimage-extract

# Create the AppImage
squashfs-root/AppRun AppDir/usr/share/applications/slowmoUI.desktop -appimage

# Make the AppImage executable for everybody
# It belongs to root, and without chmod only root can modify it outside of the container
chmod 777 *.AppImage
