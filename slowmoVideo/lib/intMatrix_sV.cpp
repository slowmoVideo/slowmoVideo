/*
slowmoVideo creates slow-motion videos from normal-speed videos.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "intMatrix_sV.h"

#include <algorithm>

IntMatrix_sV::IntMatrix_sV(int width, int height, int channels) :
    m_width(width),
    m_height(height),
    m_channels(channels)
{
    m_data = new int[width*height*m_channels];
    std::fill(m_data, m_data + width*height*channels, 0);
}

IntMatrix_sV::~IntMatrix_sV()
{
    delete[] m_data;
}

int IntMatrix_sV::width() const { return m_width; }
int IntMatrix_sV::height() const { return m_height; }
int IntMatrix_sV::channels() const { return m_channels; }

void IntMatrix_sV::operator +=(const unsigned char *bytes)
{
    for (int i = 0; i < m_width*m_height*m_channels; i++) {
        m_data[i] += bytes[i];
    }
}

void IntMatrix_sV::operator /=(int divisor)
{
    for (int i = 0; i < m_width*m_height*m_channels; i++) {
        m_data[i] /= divisor;
    }
}

unsigned char* IntMatrix_sV::toBytesArray() const
{
    unsigned char *arr = new unsigned char[m_width*m_height*m_channels];
    for (int i = 0; i < m_width*m_height*m_channels; i++) {
        arr[i] = (unsigned char) m_data[i];
    }
    return arr;
}
const int* IntMatrix_sV::data() const
{
    return m_data;
}
