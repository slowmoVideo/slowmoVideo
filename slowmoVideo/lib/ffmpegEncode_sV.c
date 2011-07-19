/*
  This code is based on http://ffmpeg.org/doxygen/trunk/encoding_8c-source.html
                    and http://ffmpeg.org/doxygen/trunk/muxing_8c-source.html
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
    prepare(video, "/tmp/ffmpegTest.avi", NULL, 352, 288, 400000,
                 1, 24);
}

int open_video(VideoOut_sV *video)
{
    AVCodec *codec;
    AVCodecContext *cc;

    cc = video->streamV->codec;

    /* find the video encoder */
    codec = avcodec_find_encoder(cc->codec_id);
    if (!codec) {
        char s[200];
        sprintf(s, "Codec for ID %d could not be found.\n", cc->codec_id);
        fprintf(stderr, s);
        setErrorMessage(video, s);
        return 3;
    } else {
        printf("Codec used: %s\n", codec->name);
    }

    /* open the codec */
    if (avcodec_open(cc, codec) < 0) {
        char s[200];
        sprintf(s, "Could not open codec %s.\n", codec->long_name);
        fprintf(stderr, s);
        return 3;
    }

    video->outbufV = NULL;
    if (!(video->fc->oformat->flags & AVFMT_RAWPICTURE)) {
        /* allocate output buffer */
        /* XXX: API change will be done */
        /* buffers passed into lav* can be allocated any way you prefer,
           as long as they're aligned enough for the architecture, and
           they're freed appropriately (such as using av_free for buffers
           allocated with av_malloc) */
        // \todo av_get_picture_size?
        video->outbufSizeV = 200000;
        video->outbufV = av_malloc(video->outbufSizeV);
    }
    return 0;
}

int prepare(VideoOut_sV *video, const char *filename, const char *vcodec, const int width, const int height, const int bitrate,
             const unsigned int numerator, const unsigned int denominator)
{

    /* must be called before using avcodec lib */
    avcodec_init();

    video->frameNr = 0;
    video->errorMessage = NULL;
    video->filename = malloc(strlen(filename)+1);
    strcpy(video->filename, filename);

    /* initialize libavcodec, and register all codecs and formats */
    av_register_all();

    /* allocate the output media context */
    avformat_alloc_output_context2(&video->fc, NULL, NULL, filename);
    if (!video->fc) {
        printf("Could not deduce output format from file extension: using MPEG.\n");
        avformat_alloc_output_context2(&video->fc, NULL, "mpeg", filename);
    }
    if (!video->fc) {
        const char *s = "Could allocate the output context, even MPEG is not available.\n";
        fprintf(stderr, s);
        setErrorMessage(video, s);
        return 2;
    }
    video->format = video->fc->oformat;
    printf("Using format %s.\n", video->format->name);

    /* Use the given vcodec if it is not NULL */
    if (vcodec != NULL) {
        AVCodec *codec = avcodec_find_encoder_by_name(vcodec);
        if (codec == NULL) {
            char s[strlen(vcodec)+50];
            sprintf(s, "No codec available for %s.\n", vcodec);
            fprintf(stderr, s);
            setErrorMessage(video, s);
            return 2;
        }
        printf("Found codec: %s\n", codec->long_name);
        video->format->video_codec = codec->id;
    }

    /* add the audio and video streams using the default format codecs
       and initialize the codecs */
    video->streamV = NULL;
    if (video->format->video_codec != CODEC_ID_NONE) {

        video->streamV = av_new_stream(video->fc, 0);
        if (!video->streamV) {
            const char *s = "Could not allocate the video stream.\n";
            fprintf(stderr, s);
            setErrorMessage(video, s);
            return 2;
        }

        AVCodecContext *cc = video->streamV->codec;

        cc->codec_id = video->format->video_codec;
        cc->codec_type = AVMEDIA_TYPE_VIDEO;

        cc->bit_rate = bitrate;

        /* resolution must be a multiple of two */
        cc->width = width;
        cc->height = height;


        /* time base: this is the fundamental unit of time (in seconds) in terms
           of which frame timestamps are represented. for fixed-fps content,
           timebase should be 1/framerate and timestamp increments should be
           identically 1. */
        cc->time_base = (AVRational){numerator, denominator};

        cc->gop_size = 12; /* emit one intra frame every ten frames */


        cc->pix_fmt = PIX_FMT_YUV420P;
        if (cc->codec_id == CODEC_ID_MPEG2VIDEO || cc->codec_id == CODEC_ID_MPEG4) {
            /* just for testing, we also add B frames */
            cc->max_b_frames = 2;
        }
        if (cc->codec_id == CODEC_ID_MPEG1VIDEO){
            /* Needed to avoid using macroblocks in which some coeffs overflow.
               This does not happen with normal video, it just happens here as
               the motion of the chroma plane does not match the luma plane. */
            cc->mb_decision=2;
        }
        // some formats want stream headers to be separate
        if(video->fc->oformat->flags & AVFMT_GLOBALHEADER) {
            cc->flags |= CODEC_FLAG_GLOBAL_HEADER;
        }

        video->rgbConversionContext = sws_getContext(
                    cc->width, cc->height,
                    PIX_FMT_BGRA,
                    cc->width, cc->height,
                    cc->pix_fmt,
                    SWS_BICUBIC, NULL, NULL, NULL);

        // One line size for each plane. RGB consists of one plane only.
        // (YUV420p consists of 3, Y, Cb, and Cr
        video->rgbLinesize[0] = cc->width*4;
        video->rgbLinesize[1] = 0;
        video->rgbLinesize[2] = 0;
        video->rgbLinesize[3] = 0;

        if (video->rgbConversionContext == NULL) {
            char s[200];
            sprintf(s, "Cannot initialize the RGB conversion context. Incorrect size (%dx%d)?\n", cc->width, cc->height);
            fprintf(stderr, s);
            setErrorMessage(video, s);
            return 2;
        }


        printf("Settings: %dx%d, %d bits/s (tolerance: %d), %d/%d fps\n", cc->width, cc->height,
               cc->bit_rate, cc->bit_rate_tolerance, cc->time_base.den, cc->time_base.num);
//        printf("Stream settings: %d/%d fps\n", video->streamV->time_base.den, video->streamV->time_base.num);
        fflush(stdout);
    } else {
        const char *s = "No codec ID given.\n";
        fprintf(stderr, s);
        setErrorMessage(video, s);
        return 2;
    }

    av_dump_format(video->fc, 0, filename, 1);

    /* now that all the parameters are set, we can open the audio and
       video codecs and allocate the necessary encode buffers */
    if (video->streamV) {
        int ret = open_video(video);
        if (ret != 0) {
            return ret;
        }
    } else {
        const char *s = "Could not open video stream.\n";
        fprintf(stderr, s);
        setErrorMessage(video, s);
        return 2;
    }



    /* open the output file, if needed */
    if (!(video->format->flags & AVFMT_NOFILE)) {
        if (avio_open(&video->fc->pb, filename, AVIO_FLAG_WRITE) < 0) {
            fprintf(stderr, "could not open %s\n", video->filename);
            char *msg = "Could not open file: ";
            char *msgAll = malloc(sizeof(char) * (strlen(filename) + strlen(msg)));
            strcpy(msgAll, msg);
            strcat(msgAll, filename);
            fprintf(stderr, msgAll);
            setErrorMessage(video, msgAll);
            free(msgAll);
            return 5;
        }
    }

    /* write the stream header, if any */
    avformat_write_header(video->fc, NULL);


    /* alloc image and output buffer */
    video->outbufSizeV = avpicture_get_size(video->streamV->codec->pix_fmt, width, height);
    video->outbufV = av_malloc(video->outbufSizeV);

    video->picture = avcodec_alloc_frame();
    avpicture_alloc((AVPicture*)video->picture, video->streamV->codec->pix_fmt,
                    video->streamV->codec->width, video->streamV->codec->height);
    if (!video->picture) {
        const char *s = "Could not allocate AVPicture.\n";
        fprintf(stderr, s);
        setErrorMessage(video, s);
        return 2;
    }

    return 0;
}

int eatARGB(VideoOut_sV *video, const unsigned char *data)
{
    fflush(stdout);

    int ret = 0;
    AVCodecContext *cc = video->streamV->codec;

    sws_scale(video->rgbConversionContext,
              &data, video->rgbLinesize,
              0, cc->height,
              video->picture->data, video->picture->linesize
              );


    if (video->fc->oformat->flags & AVFMT_RAWPICTURE) {
        /* raw video case. The API will change slightly in the near
           future for that */
        AVPacket pkt;
        av_init_packet(&pkt);

        pkt.flags |= AV_PKT_FLAG_KEY;
        pkt.stream_index = video->streamV->index;
        pkt.data = (uint8_t *)video->picture;
        pkt.size = sizeof(AVPicture);

        ret = av_interleaved_write_frame(video->fc, &pkt);
    } else {
        /* encode the image */
        video->outSize = avcodec_encode_video(cc, video->outbufV, video->outbufSizeV, video->picture);
        /* if zero size, it means the image was buffered */
        if (video->outSize > 0) {
            AVPacket pkt;
            av_init_packet(&pkt);

            if (cc->coded_frame->pts != AV_NOPTS_VALUE) {
                pkt.pts = av_rescale_q(cc->coded_frame->pts, cc->time_base, video->streamV->time_base);
                printf("pkt.pts is %d.\n", pkt.pts);
            }
            if(cc->coded_frame->key_frame) {
                pkt.flags |= AV_PKT_FLAG_KEY;
            }
            pkt.stream_index = video->streamV->index;
            pkt.data = video->outbufV;
            pkt.size = video->outSize;

            /* write the compressed frame in the media file */
            ret = av_interleaved_write_frame(video->fc, &pkt);
        } else {
            ret = 0;
        }
    }
    if (ret != 0) {
        const char *s = "Error while writing video frame (interleaved_write).\n";
        fprintf(stderr, s);
        setErrorMessage(video, s);
        return ret;
    }
    printf("Added frame %d.\n", video->frameNr);
    video->frameNr++;

    return ret;
}

void eatSample(VideoOut_sV *video)
{
    fflush(stdout);
    /* prepare a dummy image */
    /* Y */
    int x, y;
    for(y = 0; y < video->streamV->codec->height; y++) {
        for(x = 0; x < video->streamV->codec->width; x++) {
            video->picture->data[0][y * video->picture->linesize[0] + x] = x + y + video->frameNr * 3;
        }
    }

    /* Cb and Cr */
    for(y = 0; y < video->streamV->codec->height/2; y++) {
        for(x = 0; x < video->streamV->codec->width/2; x++) {
            video->picture->data[1][y * video->picture->linesize[1] + x] = 128 + y + video->frameNr * 2;
            video->picture->data[2][y * video->picture->linesize[2] + x] = 64 + x + video->frameNr * 5;
        }
    }

    /* encode the image */
    AVCodecContext *cc = video->streamV->codec;
    video->outSize = avcodec_encode_video(cc, video->outbufV, video->outbufSizeV, video->picture);
    /* if zero size, it means the image was buffered */
    if (video->outSize > 0) {
        AVPacket pkt;
        av_init_packet(&pkt);

        if (cc->coded_frame->pts != AV_NOPTS_VALUE) {
            pkt.pts = av_rescale_q(cc->coded_frame->pts, cc->time_base, video->streamV->time_base);
            printf("pkt.pts is %d.\n", pkt.pts);
        }
        if(cc->coded_frame->key_frame) {
            pkt.flags |= AV_PKT_FLAG_KEY;
        }
        pkt.stream_index = video->streamV->index;
        pkt.data = video->outbufV;
        pkt.size = video->outSize;

        /* write the compressed frame in the media file */
        av_interleaved_write_frame(video->fc, &pkt);
    }

    video->frameNr++;
}

void finish(VideoOut_sV *video)
{

    /* write the trailer, if any.  the trailer must be written
     * before you close the CodecContexts open when you wrote the
     * header; otherwise write_trailer may try to use memory that
     * was freed on av_codec_close() */
    av_write_trailer(video->fc);

    /* close each codec */
    if (video->streamV) {
        avcodec_close(video->streamV->codec);
        av_free(video->picture->data[0]);
        av_free(video->picture);
        av_free(video->outbufV);
    }

    /* free the streams */
    for(int i = 0; i < video->fc->nb_streams; i++) {
        av_freep(&video->fc->streams[i]->codec);
        av_freep(&video->fc->streams[i]);
    }

    if (!(video->format->flags & AVFMT_NOFILE)) {
        /* close the output file */
        avio_close(video->fc->pb);
    }

    /* free the stream */
    av_free(video->fc);

    sws_freeContext(video->rgbConversionContext);
    printf("\nWrote to %s.\n", video->filename);
}
