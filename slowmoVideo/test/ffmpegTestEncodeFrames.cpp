
// Against the «UINT64_C not declared» message.
// See: http://code.google.com/p/ffmpegsource/issues/detail?id=11
#ifdef __cplusplus
 #define __STDC_CONSTANT_MACROS
 #ifdef _STDINT_H
  #undef _STDINT_H
 #endif
 # include <stdint.h>
#endif

extern "C" {
#include "ffmpegTestEncode.h"
}

int main()
{
    VideoOut_sV video;
    prepareDefault(&video);
    for (int i = 0; i < 50; i++) {
        eatSample(&video);
    }
    finish(&video);
}
