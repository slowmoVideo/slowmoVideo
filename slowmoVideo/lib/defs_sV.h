#ifndef DEFS_SV_H
#define DEFS_SV_H

#include <inttypes.h>

typedef struct VideoInfoSV {
    int frameRateNum;
    int frameRateDen;
    int width;
    int height;
    int64_t framesCount;
    int streamsCount;
} VideoInfoSV;

#endif // DEFS_SV_H
