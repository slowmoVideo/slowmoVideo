#ifndef FFMPEGTESTENCODE_H
#define FFMPEGTESTENCODE_H

#include "libavcodec/avcodec.h"

typedef struct VideoOut_sV {
    AVCodec *codec;
    AVCodecContext *c;
    AVFrame *picture;
    FILE *f;
    const char *filename;
    int outbuf_size, out_size;
    uint8_t *outbuf;
    int frameNr;
} VideoOut_sV;

void prepareDefault(VideoOut_sV *video);
void prepare(VideoOut_sV *video, const int width, const int height, const int bitrate,
             const unsigned int numerator, const unsigned int denominator, const int eatsRGB);
void eatARGB(VideoOut_sV *video, const unsigned char *data);
void eatSample(VideoOut_sV *video);
void finish(VideoOut_sV *video);

#endif // FFMPEGTESTENCODE_H
