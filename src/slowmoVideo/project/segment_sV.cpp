/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "segment_sV.h"

Segment_sV::Segment_sV(int leftNodeIndex) :
    m_leftNodeIndex(leftNodeIndex),
    m_selected(false)
{
}

int Segment_sV::leftNodeIndex() const
{
     return m_leftNodeIndex;
}

bool Segment_sV::selected() const
{
    return m_selected;
}

void Segment_sV::select(bool select)
{
    m_selected = select;
}

bool Segment_sV::operator <(const Segment_sV& other) const
{
    return m_leftNodeIndex < other.m_leftNodeIndex;
}

QString toString(const Segment_sV &segment)
{
    return QString("Left node: %1; selected: %2").arg(segment.leftNodeIndex()).arg(segment.selected());
}
