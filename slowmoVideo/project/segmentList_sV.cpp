/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "segmentList_sV.h"

#include <QtCore/QDebug>
#include <iostream>

SegmentList_sV::SegmentList_sV()
{
}

void SegmentList_sV::unselectAll()
{
    qDebug() << "Unselecting all nodes.";
    for (int i = 0; i < m_list.size(); i++) {
        m_list[i].select(false);
        std::cerr << i << " ...";
        Q_ASSERT(!m_list.at(i).selected());
    }
    std::cerr << " done. " << std::endl;
}

void SegmentList_sV::grow()
{
    m_list.append(Segment_sV(m_list.size()));
    unselectAll();
    qSort(m_list);

    for (int i = 0; i < m_list.size(); i++) {
        qDebug() << "Segment " << i << ": " << toString(m_list.at(i));
    }
}

void SegmentList_sV::shrink()
{
    m_list.removeLast();

    for (int i = 0; i < m_list.size(); i++) {
        qDebug() << "Segment " << i << ": " << toString(m_list.at(i));
    }
}

int SegmentList_sV::size() const
{
    return m_list.size();
}

const Segment_sV& SegmentList_sV::at(int i) const
{
    return m_list.at(i);
}

Segment_sV& SegmentList_sV::operator [](int i)
{
    return m_list[i];
}
