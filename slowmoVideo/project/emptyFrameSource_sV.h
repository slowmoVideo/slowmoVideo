#ifndef EMPTYFRAMESOURCE_H
#define EMPTYFRAMESOURCE_H

#include "abstractFrameSource_sV.h"

class EmptyFrameSource_sV : public AbstractFrameSource_sV
{
    Q_OBJECT

public:
    EmptyFrameSource_sV(const Project_sV *project);
    ~EmptyFrameSource_sV() {}

    int64_t framesCount() const { return 1000; }
    int frameRateNum() const { return 24; }
    int frameRateDen() const { return 1; }
    QImage frameAt(const uint, const FrameSize = FrameSize_Orig) const { return QImage(); }
};

#endif // EMPTYFRAMESOURCE_H
