/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "segment_sV.h"

Segment_sV::Segment_sV(int index) :
    m_leftNodeIndex(index)
{
}

int Segment_sV::leftNodeIndex() const
{
     return m_leftNodeIndex;
}
