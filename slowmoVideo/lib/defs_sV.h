#ifndef DEFS_SV_H
#define DEFS_SV_H

#include "macros_sV.h"

#if defined(WINDOWS) && !defined(MXE)
typedef __int64 int64_t;
#else
#include <inttypes.h>
#endif

/// Holds information about a video input file.
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
