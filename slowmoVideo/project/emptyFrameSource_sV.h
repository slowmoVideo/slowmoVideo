/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef EMPTYFRAMESOURCE_H
#define EMPTYFRAMESOURCE_H

#include "abstractFrameSource_sV.h"

class EmptyFrameSource_sV : public AbstractFrameSource_sV
{
    Q_OBJECT

public:
    EmptyFrameSource_sV(const Project_sV *project);
    ~EmptyFrameSource_sV() {}

    void initialize();
    bool initialized() const { return true; }

    int64_t framesCount() const { return 1000; }
    int frameRateNum() const { return 24; }
    int frameRateDen() const { return 1; }
    QImage frameAt(const uint, const FrameSize = FrameSize_Orig) { return QImage(); }

public slots:
    void slotAbortInitialization() {}
};

#endif // EMPTYFRAMESOURCE_H
