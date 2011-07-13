
#include "ffmpegTestEncode.h"

int main()
{
    VideoOut_sV video;
    prepareDefault(&video);
    for (int i = 0; i < 50; i++) {
        eatSample(&video);
    }
    finish(&video);
}
