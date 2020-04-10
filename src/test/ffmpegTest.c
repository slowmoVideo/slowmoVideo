// LibAV docs: http://libav.org/doxygen/master/avformat_8h.html
// Tutorial: http://dranger.com/ffmpeg/tutorial01.html

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

int main(int argc, char *argv[])
{
    av_register_all();

    AVFormatContext *pFormatContext;
    int ret;
    if (argc < 2) {
	printf("Usage: %s file\n", argv[0]);
	return -1;
    }

    if ((ret = av_open_input_file(&pFormatContext, argv[1], NULL, 0, NULL)) != 0) {
	printf("Could not open file %s.\n", argv[1]);
	return ret;
    }
    if (av_find_stream_info(pFormatContext) < 0) {
	printf("No stream information found.\n");
	return -2;
    }
    av_dump_format(pFormatContext, 0, argv[1], 0);

    AVCodecContext *pCodecContext;
    int videoStream = -1;
    for (int i = 0; i < pFormatContext->nb_streams; i++) {
	if (pFormatContext->streams[i]->codec->codec_type == CODEC_TYPE_VIDEO) {
	    videoStream = i;
	    pCodecContext = pFormatContext->streams[i]->codec;
	    AVRational fps = pFormatContext->streams[i]->r_frame_rate;
	    printf("Frame rate: %d/%d = %f\n", fps.num, fps.den, (float)fps.num / fps.den);
	}
    }
    return 0;
}
