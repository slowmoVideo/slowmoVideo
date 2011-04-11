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

    av_register_all();

    AVFormatContext *pFormatContext;
 
    int ret;
    if ((ret = av_open_input_file(&pFormatContext, filename, NULL, 0, NULL)) != 0) {
	printf("Could not open file %s.\n", filename);
	return info;
    }
    if (av_find_stream_info(pFormatContext) < 0) {
	printf("No stream information found.\n");
	return info;
    }
    av_dump_format(pFormatContext, 0, filename, 0);

    AVCodecContext *pCodecContext;
    int videoStream = -1;
    for (int i = 0; i < pFormatContext->nb_streams; i++) {
	if (pFormatContext->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO) {
	    videoStream = i;
	    pCodecContext = pFormatContext->streams[i]->codec;
	    AVRational fps = pFormatContext->streams[i]->r_frame_rate;
	    printf("Frame rate: %d/%d = %f\n", fps.num, fps.den, (float)fps.num / fps.den);
	    info.frameRateNum = fps.num;
	    info.frameRateDen = fps.den;
	}
    }
    return info;
}

