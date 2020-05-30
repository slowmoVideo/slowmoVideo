#!/bin/sh
#
# This file is intended to be run inside the Docker container.

# This script copies the built AppImage to /__build, so if the container is run with a volume mount on /__build,
# the AppImage will be copied to the host.
mkdir -p /__build

mkdir /slowmoVideo/build
cd /slowmoVideo/build

# Default steps for building the AppImage
# Configure and build slowmoVideo
cmake .. -DCMAKE_INSTALL_PREFIX=/usr
make -j9

# Install slowmoVideo to the AppDir directory for AppImage
make install DESTDIR=AppDir

# Extract the linuxdeployqt AppImage when FUSE is not available in a docker container
/linuxdeployqt.AppImage  --appimage-extract

# Set the version from Git
export VERSION=$(git describe HEAD)

# Create the AppImage
squashfs-root/AppRun AppDir/usr/share/applications/slowmoUI.desktop -appimage

# Make the AppImage executable for everybody
# It belongs to root, and without chmod only root can modify it outside of the container
chmod 777 *.AppImage

# Copy it to /__build where the volume mount might be
cp *.AppImage /__build
