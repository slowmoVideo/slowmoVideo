#!/bin/sh

mkdir -p appimage-build || exit 1
cd appimage-build || exit 1
cmake .. -DCMAKE_INSTALL_PREFIX=/usr
make -j10 || exit 1

# Install slowmoVideo to the AppDir directory for AppImage
make install DESTDIR=AppDir || exit 1

# Extract the linuxdeployqt AppImage when FUSE is not available in a docker container
~/linuxdeployqt-6-x86_64.AppImage --appimage-extract

# Create the AppImage
squashfs-root/AppRun AppDir/usr/share/applications/slowmoUI.desktop -appimage -exclude-libs=libgmodule-2.0.so
