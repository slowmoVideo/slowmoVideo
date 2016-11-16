
#include "../lib/ffmpegEncode_sV.h"

int main()
{
	  int i;

    VideoOut_sV video;
    prepareDefault(&video);
    for (i = 0; i < 50; i++) {
        eatSample(&video);
    }
    finish(&video);
}
