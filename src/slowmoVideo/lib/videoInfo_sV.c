/*
slowmoVideo creates slow-motion videos from normal-speed videos.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

// LibAV docs: http://libav.org/doxygen/master/avformat_8h.html
// Tutorial: http://dranger.com/ffmpeg/tutorial01.html

#include "videoInfo_sV.h"

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

VideoInfoSV getInfo(const char filename[])
{
    VideoInfoSV info;
    info.frameRateNum = 0;
    info.frameRateDen = 0;
    info.streamsCount = 0;
    info.framesCount = 0;

    av_register_all();
#if LIBAVFORMAT_VERSION_INT >= AV_VERSION_INT(53,19,0)
    avformat_network_init();
#endif

    AVFormatContext *pFormatContext = NULL;

    printf("Reading info for file %s.\n", filename);
    fflush(stdout);
 
    int ret;
#if LIBAVFORMAT_VERSION_MAJOR < 53
    if ((ret = av_open_input_file(&pFormatContext, filename, NULL, 0, NULL)) != 0) {
#else
    if ((ret = avformat_open_input(&pFormatContext, filename, NULL, NULL)) != 0) {
#endif
        printf("Could not open file %s.\n", filename);
        return info;
    }
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(53,9,0)
    if (av_find_stream_info(pFormatContext) < 0) {
#else
    if (avformat_find_stream_info(pFormatContext, NULL) < 0) {
#endif
        printf("No stream information found.\n");
        return info;
    }
#if LIBAVFORMAT_VERSION_MAJOR < 53
    dump_format(pFormatContext, 0, filename, 0);
#else
    av_dump_format(pFormatContext, 0, filename, 0);
#endif

    AVCodecContext *pCodecContext;
    int videoStream = -1;
    int i;
    for (i = 0; i < pFormatContext->nb_streams; i++) {
#if LIBAVCODEC_VERSION_INT < (52<<16 | 64<<8 | 0)
        if (pFormatContext->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO) {
#else
        if (pFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
#endif
            videoStream = i;
            pCodecContext = pFormatContext->streams[i]->codec;
            AVRational fps = pFormatContext->streams[i]->r_frame_rate;
            printf("Frame rate: %d/%d = %f\n", fps.num, fps.den, (float)fps.num / fps.den);
            info.frameRateNum = fps.num;
            info.frameRateDen = fps.den;
            info.width = pCodecContext->width;
            info.height = pCodecContext->height;
            // info.framesCount = pFormatContext->streams[i]->nb_frames; // Doesn't work for each format.
            AVRational tb = pFormatContext->streams[i]->time_base; // Use the precise timebase of this stream.
            info.framesCount = (long)( (double)fps.num/fps.den * pFormatContext->streams[i]->duration * (double)tb.num/tb.den ); // Works really good for any type of video.
            info.streamsCount++;
            printf("Total frames: %lld (Length: %f s)\n", info.framesCount, info.framesCount/((float)fps.num/fps.den));
        }
    }

    av_free(pFormatContext);
    return info;
}

