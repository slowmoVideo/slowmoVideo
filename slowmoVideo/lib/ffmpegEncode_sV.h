#ifndef FFMPEGENCODE_SV_H
#define FFMPEGENCODE_SV_H
/*
  Copyright (c) 2011 Simon A. Eugster

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  */

#include <inttypes.h>

// Against the «UINT64_C not declared» message.
// See: http://code.google.com/p/ffmpegsource/issues/detail?id=11
#ifdef __cplusplus
 #define __STDC_CONSTANT_MACROS
 #ifdef _STDINT_H
  #undef _STDINT_H
 #endif
 # include <stdint.h>
#endif


#include "libavcodec/avcodec.h"

/// This struct can eat frames and produces videos!
typedef struct VideoOut_sV {
    /// Used codec
    AVCodec *codec;
    AVCodecContext *cc;
    AVFrame *picture;
    /// Current frame number that is encoded
    int frameNr;

    /// Context for converting RGB frames to YUV420p
    struct SwsContext* rgbConversionContext;
    /// Required for converting RGB images
    int rgbLinesize[4];

    /// Video file
    FILE *file;
    /// Video filename
    char *filename;

    int outbufSize, outSize;
    uint8_t *outbuf;

    /// Set if an error occurs (file does not exist, for example), for more accurate information.
    char *errorMessage;

} VideoOut_sV;

/// Prepares a default VideoOut_sV struct, mainly for testing purposes with eatSample().
void prepareDefault(VideoOut_sV *video);
/// Prepares a VideoOut_sV struct. After preparation it is ready to eat RGB images.
int prepare(VideoOut_sV *video, const char *filename, const int width, const int height, const int bitrate,
             const unsigned int numerator, const unsigned int denominator);

/// Eats an RGB image and deposits it in the output frame.
void eatARGB(VideoOut_sV *video, const unsigned char *data);
/// Eats a sample image. For testing.
void eatSample(VideoOut_sV *video);

/// Finishes the produced video file.
void finish(VideoOut_sV *video);

#endif // FFMPEGENCODE_SV_H
