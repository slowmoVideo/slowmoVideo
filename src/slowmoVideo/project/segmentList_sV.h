/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef SEGMENTLIST_SV_H
#define SEGMENTLIST_SV_H

#include "segment_sV.h"
#include <QtCore/QList>

/**
  \brief List of Segment_sV
  */
class SegmentList_sV
{
public:
    SegmentList_sV();

    void unselectAll();
    void select(int segment);

    void shrink();
    void grow();

    int size() const;

    const Segment_sV& at(int i) const;
    Segment_sV& operator [](int i);

private:
    QList<Segment_sV> m_list;
};

#endif // SEGMENTLIST_SV_H
