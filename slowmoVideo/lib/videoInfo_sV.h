/*
slowmoVideo creates slow-motion videos from normal-speed videos.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

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
