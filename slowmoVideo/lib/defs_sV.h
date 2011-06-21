#ifndef DEFS_SV_H
#define DEFS_SV_H

#include <inttypes.h>

typedef struct VideoInfoSV {
    /// Frame rate numerator
    int frameRateNum;
    /// Frame rate denominator
    int frameRateDen;
    /// Frame width
    int width;
    /// Frame height
    int height;
    /// Number of frames in total
    int64_t framesCount;
    /// Number of available video streams
    int streamsCount;
} VideoInfoSV;

#endif // DEFS_SV_H
