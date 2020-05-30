FROM ubuntu:16.04

# This image is intended for building an AppImage version of slowmoVideo and is therefore based
# on Ubuntu 16.04. As slowmoVideo requires an up-to-date OpenCV version, it first needs to be
# compiled as Ubuntu 16.04 ships an older version.

# Install all dependencies for building OpenCV and slowmoVideo, and for building the AppImage
RUN apt-get update \
    && apt-get -y install \
    git \
    wget \
    file \
    qt5-default \
    qttools5-dev-tools \
    qtscript5-dev \
    cmake \
    build-essential \
    zlib1g-dev \
    libtiff-dev \
    libjasper-dev \
    libpng-dev \
    libopenjpeg-dev \
    libavcodec-dev \
    libavformat-dev \
    libswscale-dev \
    libavresample-dev \
    libdc1394-22-dev \
    libopenblas-dev \
    && rm -rf /var/lib/apt/lists/*

# Get OpenCV source code from Git, including OpenCV Contrib (contains Optical Flow algorithms)
RUN git clone https://github.com/opencv/opencv.git /opencv \
    && cd /opencv && git checkout 4.2.0 \
    && git clone https://github.com/opencv/opencv_contrib.git /opencv_contrib \
    && cd /opencv_contrib && git checkout 4.2.0

# Build OpenCV
RUN mkdir /opencv/build \
    && cd /opencv/build \
    && cmake .. -DCMAKE_INSTALL_PREFIX=/usr -DOPENCV_EXTRA_MODULES_PATH=/opencv_contrib/modules/ \
    && make -j9 \
    && make install

# Get slowmoVideo and the linuxdeployqt tool (builds an AppImage from Qt apps)
RUN git clone https://github.com/slowmoVideo/slowmoVideo.git /slowmoVideo \
    && cd /slowmoVideo && git submodule update --init \
    && wget https://github.com/probonopd/linuxdeployqt/releases/download/6/linuxdeployqt-6-x86_64.AppImage -O /linuxdeployqt.AppImage \
    && chmod +x /linuxdeployqt.AppImage

ADD docker-build-appimage.sh /

CMD ./docker-build-appimage.sh
