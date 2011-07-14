/*
  This code is based on http://ffmpeg.org/doxygen/trunk/encoding_8c-source.html
  and has been adjusted with a lot of help from Tjoppen at irc.freenode.org#ffmpeg. (Thanks!)
  Copyright (c) 2001 Fabrice Bellard
                2011 Simon A. Eugster
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.
  */

#include "ffmpegEncode_sV.h"
#include <libswscale/swscale.h>

void setErrorMessage(VideoOut_sV *video, const char *msg)
{
    if (video->errorMessage != NULL) {
        free(video->errorMessage);
    }
    video->errorMessage = malloc(strlen(msg)+1);
    strcpy(video->errorMessage, msg);
}

void prepareDefault(VideoOut_sV *video)
{
    prepare(video, "/tmp/ffmpegTest.avi", 352, 288, 400000,
                 1, 24);
}

int prepare(VideoOut_sV *video, const char *filename, const int width, const int height, const int bitrate,
             const unsigned int numerator, const unsigned int denominator)
{

    /* must be called before using avcodec lib */
    avcodec_init();

    /* register all the codecs */
    avcodec_register_all();


    printf("Video encoding\n");

    /* find the mpeg1 video encoder */
    video->codec = avcodec_find_encoder(CODEC_ID_MPEG1VIDEO);
    if (!video->codec) {
        fprintf(stderr, "codec not found\n");
        setErrorMessage(video, "Selected codec could not be found!");
        return -2;
    }


    video->frameNr = 0;
    video->errorMessage = NULL;

    video->cc = avcodec_alloc_context3(video->codec);

    video->cc->bit_rate = bitrate;

    /* resolution must be a multiple of two */
    video->cc->width = width;
    video->cc->height = height;

    /* frames per second */
    video->cc->time_base= (AVRational){numerator, denominator};
    video->cc->gop_size = 10; /* emit one intra frame every ten frames */
    video->cc->max_b_frames=1;
    video->cc->pix_fmt = PIX_FMT_YUV420P;


    printf("Settings: %dx%d, %d bits/s (tolerance: %d), %d fps\n", video->cc->width, video->cc->height,
           video->cc->bit_rate, video->cc->bit_rate_tolerance, video->cc->time_base.num);
    fflush(stdout);


    /* open it */
    if (avcodec_open(video->cc, video->codec) < 0) {
        setErrorMessage(video, "Selected codec could not be opened, see debug output for details.");
        fprintf(stderr, "could not open codec\n");
        return -3;
    }



    video->rgbConversionContext = sws_getContext(
                video->cc->width, video->cc->height,
                PIX_FMT_BGRA,
                video->cc->width, video->cc->height,
                PIX_FMT_YUV420P,
                SWS_FAST_BILINEAR, NULL, NULL, NULL);
    // One line size for each plane. RGB consists of one plane only.
    // (YUV420p consists of 3, Y, Cb, and Cr
    video->rgbLinesize[0] = video->cc->width*4;
    video->rgbLinesize[1] = 0;
    video->rgbLinesize[2] = 0;
    video->rgbLinesize[3] = 0;

    video->filename = malloc(strlen(filename)+1);
    strcpy(video->filename, filename);
    video->file = fopen(video->filename, "wb");
    if (!video->file) {
        fprintf(stderr, "could not open %s\n", video->filename);
        char *msg = "Could not open file: ";
        char *msgAll = malloc(sizeof(char) * (strlen(filename) + strlen(msg)));
        strcpy(msgAll, msg);
        strcat(msgAll, filename);
        setErrorMessage(video, msgAll);
        free(msgAll);
        return -1;
    }

    /* alloc image and output buffer */
    video->outbufSize = avpicture_get_size(video->cc->pix_fmt, width, height);
    video->outbuf = av_malloc(video->outbufSize);

    video->picture = avcodec_alloc_frame();
    avpicture_alloc((AVPicture*)video->picture, video->cc->pix_fmt, video->cc->width, video->cc->height);
}

void eatARGB(VideoOut_sV *video, const unsigned char *data)
{
    fflush(stdout);

    printf("Line size: %d %d %d %d\n", video->picture->linesize[0], video->picture->linesize[1], video->picture->linesize[2], video->picture->linesize[3]);

    sws_scale(video->rgbConversionContext,
              &data, video->rgbLinesize,
              0, video->cc->height,
              video->picture->data, video->picture->linesize
              );

    /* encode the image */
    video->outSize = avcodec_encode_video(video->cc, video->outbuf, video->outbufSize, video->picture);
    printf("encoding frame %3d (size=%5d)\n", video->frameNr, video->outSize);
    fwrite(video->outbuf, 1, video->outSize, video->file);
    video->frameNr++;
}

void eatSample(VideoOut_sV *video)
{
    fflush(stdout);
    /* prepare a dummy image */
    /* Y */
    int x, y;
    for(y = 0; y < video->cc->height; y++) {
        for(x = 0; x < video->cc->width; x++) {
            video->picture->data[0][y * video->picture->linesize[0] + x] = x + y + video->frameNr * 3;
        }
    }

    /* Cb and Cr */
    for(y = 0; y < video->cc->height/2; y++) {
        for(x = 0; x < video->cc->width/2; x++) {
            video->picture->data[1][y * video->picture->linesize[1] + x] = 128 + y + video->frameNr * 2;
            video->picture->data[2][y * video->picture->linesize[2] + x] = 64 + x + video->frameNr * 5;
        }
    }

    /* encode the image */
    video->outSize = avcodec_encode_video(video->cc, video->outbuf, video->outbufSize, video->picture);
    printf("encoding frame %3d (size=%5d)\n", video->frameNr, video->outSize);
    fwrite(video->outbuf, 1, video->outSize, video->file);
    video->frameNr++;
}

void finish(VideoOut_sV *video)
{
    /* get the delayed frames */
    for(; video->outSize; video->frameNr++) {
        fflush(stdout);

        video->outSize = avcodec_encode_video(video->cc, video->outbuf, video->outbufSize, NULL);
        printf("write frame %3d (size=%5d)\n", video->frameNr, video->outSize);
        fwrite(video->outbuf, 1, video->outSize, video->file);
    }

    /* add sequence end code to have a real mpeg file */
    video->outbuf[0] = 0x00;
    video->outbuf[1] = 0x00;
    video->outbuf[2] = 0x01;
    video->outbuf[3] = 0xb7;
    fwrite(video->outbuf, 1, 4, video->file);
    fclose(video->file);
    av_free(video->outbuf);

    avcodec_close(video->cc);
    av_free(video->cc);
    av_free(video->picture->data[0]);
    av_free(video->picture);
    sws_freeContext(video->rgbConversionContext);
    printf("\nWrote to %s.\n", video->filename);
}
