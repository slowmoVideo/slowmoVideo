/**
  This code is based on http://ffmpeg.org/doxygen/trunk/encoding_8c-source.html
  Copyright (c) 2001 Fabrice Bellard
                2011 Simon A. Eugster
  */

#include "ffmpegTestEncode.h"
#include <libswscale/swscale.h>

void prepareDefault(VideoOut_sV *video)
{
    prepare(video, 352, 288, 400000,
                 1, 24, 0);
}

void prepare(VideoOut_sV *video, const int width, const int height, const int bitrate,
             const unsigned int numerator, const unsigned int denominator, const int eatsRGB)
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
        exit(1);
    }


    video->frameNr = 0;

    video->c = avcodec_alloc_context3(video->codec);

    /* put sample parameters */
    video->c->bit_rate = bitrate;
    /* resolution must be a multiple of two */
    video->c->width = width;
    video->c->height = height;
    /* frames per second */
    video->c->time_base= (AVRational){numerator, denominator};
    video->c->gop_size = 10; /* emit one intra frame every ten frames */
    video->c->max_b_frames=1;
    video->c->pix_fmt = PIX_FMT_YUV420P;


    /* open it */
    if (avcodec_open(video->c, video->codec) < 0) {
        fprintf(stderr, "could not open codec\n");
        exit(1);
    }



    video->rgbConversionContext = sws_getContext(
                video->c->width, video->c->height,
                PIX_FMT_BGRA,
                video->c->width, video->c->height,
                PIX_FMT_YUV420P,
                SWS_FAST_BILINEAR, NULL, NULL, NULL);
    // One line size for each plane. RGB consists of one plane only.
    // (YUV420p consists of 3, Y, Cb, and Cr
    video->rgbLinesize[0] = video->c->width*4;
    video->rgbLinesize[1] = 0;
    video->rgbLinesize[2] = 0;
    video->rgbLinesize[3] = 0;

    video->filename = "/tmp/ffmpegTest.avi";
    video->f = fopen(video->filename, "wb");
    if (!video->f) {
        fprintf(stderr, "could not open %s\n", video->filename);
        exit(1);
    }

    /* alloc image and output buffer */
    video->outbuf_size = avpicture_get_size(video->c->pix_fmt, width, height);
    video->outbuf = av_malloc(video->outbuf_size);

    video->picture = avcodec_alloc_frame();
    avpicture_alloc((AVPicture*)video->picture, video->c->pix_fmt, video->c->width, video->c->height);
}

void eatARGB(VideoOut_sV *video, const unsigned char *data)
{
    fflush(stdout);

    printf("Line size: %d %d %d %d\n", video->picture->linesize[0], video->picture->linesize[1], video->picture->linesize[2], video->picture->linesize[3]);

    sws_scale(video->rgbConversionContext,
              &data, video->rgbLinesize,
              0, video->c->height,
              video->picture->data, video->picture->linesize
              );

    /* encode the image */
    video->out_size = avcodec_encode_video(video->c, video->outbuf, video->outbuf_size, video->picture);
    printf("encoding frame %3d (size=%5d)\n", video->frameNr, video->out_size);
    fwrite(video->outbuf, 1, video->out_size, video->f);
    video->frameNr++;
}

void eatSample(VideoOut_sV *video)
{
    fflush(stdout);
    /* prepare a dummy image */
    /* Y */
    int x, y;
    for(y = 0; y < video->c->height; y++) {
        for(x = 0; x < video->c->width; x++) {
            video->picture->data[0][y * video->picture->linesize[0] + x] = x + y + video->frameNr * 3;
        }
    }

    /* Cb and Cr */
    for(y = 0; y < video->c->height/2; y++) {
        for(x = 0; x < video->c->width/2; x++) {
            video->picture->data[1][y * video->picture->linesize[1] + x] = 128 + y + video->frameNr * 2;
            video->picture->data[2][y * video->picture->linesize[2] + x] = 64 + x + video->frameNr * 5;
        }
    }

    /* encode the image */
    video->out_size = avcodec_encode_video(video->c, video->outbuf, video->outbuf_size, video->picture);
    printf("encoding frame %3d (size=%5d)\n", video->frameNr, video->out_size);
    fwrite(video->outbuf, 1, video->out_size, video->f);
    video->frameNr++;
}

void finish(VideoOut_sV *video)
{
    /* get the delayed frames */
    for(; video->out_size; video->frameNr++) {
        fflush(stdout);

        video->out_size = avcodec_encode_video(video->c, video->outbuf, video->outbuf_size, NULL);
        printf("write frame %3d (size=%5d)\n", video->frameNr, video->out_size);
        fwrite(video->outbuf, 1, video->out_size, video->f);
    }

    /* add sequence end code to have a real mpeg file */
    video->outbuf[0] = 0x00;
    video->outbuf[1] = 0x00;
    video->outbuf[2] = 0x01;
    video->outbuf[3] = 0xb7;
    fwrite(video->outbuf, 1, 4, video->f);
    fclose(video->f);
    av_free(video->outbuf);

    avcodec_close(video->c);
    av_free(video->c);
    av_free(video->picture->data[0]);
    av_free(video->picture);
    sws_freeContext(video->rgbConversionContext);
    printf("\nWrote to %s.\n", video->filename);
}
