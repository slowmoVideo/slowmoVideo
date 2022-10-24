FROM ubuntu:18.04

RUN apt update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y wget build-essential cmake libopencv-dev qt5-default qttools5-dev-tools qtscript5-dev

# Get linuxdeployqt and make it executable
RUN wget https://github.com/probonopd/linuxdeployqt/releases/download/6/linuxdeployqt-6-x86_64.AppImage && \
    chmod +x linuxdeployqt-6-x86_64.AppImage && \
    mv linuxdeployqt-6-x86_64.AppImage ~
