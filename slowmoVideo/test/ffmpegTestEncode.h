#ifndef FFMPEGTESTENCODE_H
#define FFMPEGTESTENCODE_H

/**
  Copyright (c) 2011 Simon A. Eugster

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  */

#include "libavcodec/avcodec.h"

/// This struct can eat frames and produces videos!
typedef struct VideoOut_sV {
    AVCodec *codec;
    AVCodecContext *cc;
    AVFrame *picture;
    int frameNr;

    struct SwsContext* rgbConversionContext;
    int rgbLinesize[4];

    FILE *file;
    const char *filename;

    int outbufSize, outSize;
    uint8_t *outbuf;

} VideoOut_sV;

void prepareDefault(VideoOut_sV *video);
void prepare(VideoOut_sV *video, const int width, const int height, const int bitrate,
             const unsigned int numerator, const unsigned int denominator, const int eatsRGB);
void eatARGB(VideoOut_sV *video, const unsigned char *data);
void eatSample(VideoOut_sV *video);
void finish(VideoOut_sV *video);

#endif // FFMPEGTESTENCODE_H
