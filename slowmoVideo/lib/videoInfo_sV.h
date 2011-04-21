
#ifndef VIDEOINFO_SV_H
#define VIDEOINFO_SV_H

#include <inttypes.h>

typedef struct VideoInfoSV {
    int frameRateNum;
    int frameRateDen;
    int width;
    int height;
    int64_t framesCount;
    int streamsCount;
} VideoInfoSV;

VideoInfoSV getInfo(const char filename[]);

#endif //VIDEOINFO_SV_H
