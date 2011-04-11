
#ifndef VIDEOINFO_SV_H
#define VIDEOINFO_SV_H

typedef struct VideoInfoSV {
    int frameRateNum;
    int frameRateDen;
} VideoInfoSV;

VideoInfoSV getInfo(const char filename[]);

#endif //VIDEOINFO_SV_H
