#ifndef FFMPEGENCODE_SV_H
#define FFMPEGENCODE_SV_H
/*
  Copyright (c) 2011 Simon A. Eugster

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  */

#include "defs_sV.h"

// Against the «UINT64_C not declared» message.
// See: http://code.google.com/p/ffmpegsource/issues/detail?id=11
#ifdef __cplusplus
 #ifndef __STDC_CONSTANT_MACROS
 #define __STDC_CONSTANT_MACROS
 #ifdef _STDINT_H
  #undef _STDINT_H
 #endif
 # include <stdint.h>
 #endif //  __STDC_CONSTANT_MACROS
#endif

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#if ((LIBAVFORMAT_VERSION_MAJOR == 53) && (LIBAVFORMAT_VERSION_MINOR >= 21) && (LIBAVFORMAT_VERSION_MICRO < 100))
#define MOST_LIKELY_LIBAV
#endif

/// This struct can eat frames and produces videos!
/// Variables should not be changed from the outside.
typedef struct VideoOut_sV {

    AVFrame *picture; ///< Temporary picture for the incoming frame
    AVFormatContext *fc; ///< Video's format context
    AVOutputFormat *format; ///< Just a shortcut to fc->format
    AVStream *streamV; ///< Video output stream

    /// Current frame number that is encoded
    int frameNr;

    /// Context for converting RGB frames to YUV420p
    struct SwsContext* rgbConversionContext;
    /// Required for converting RGB images
    int rgbLinesize[4];

    /// Video filename
    char *filename;

    int outSize;
    int outbufSizeV;
    uint8_t *outbufV;

    /// Set if an error occurs (file does not exist, for example), for more accurate information.
    char *errorMessage;

} VideoOut_sV;

/// Prepares a default VideoOut_sV struct, mainly for testing purposes with eatSample().
void prepareDefault(VideoOut_sV *video);
/**
  Prepares a VideoOut_sV struct. After preparation it is ready to eat RGB images.
  \param video VideoOut_sV struct to prepare.
  \param filename Target filename
  \param vcodec Video codec to use (see <tt>ffmpeg -codecs</tt>). May be \c NULL,
         in this case a default codec for the format will be chosen.
  \param width Video width
  \param height Video height
  \param bitrate Bit rate. <tt>width*height*fps</tt> seems to be a good choice for high quality.
  \param numerator A frame is shown for <tt>numerator/denominator s</tt>; i.e. the fps number is <tt>denominator/numerator</tt>.
  \param denominator See numerator.
  */
int prepare(VideoOut_sV *video, const char *filename, const char *vcodec, const int width, const int height, const int bitrate,
             const unsigned int numerator, const unsigned int denominator);

/// Eats an RGB image and deposits it in the output frame.
int eatARGB(VideoOut_sV *video, const unsigned char *data);
/// Eats a sample image. For testing.
void eatSample(VideoOut_sV *video);

/// Finishes the produced video file.
void finish(VideoOut_sV *video);

#endif // FFMPEGENCODE_SV_H
