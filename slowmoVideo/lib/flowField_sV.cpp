/*
slowmoVideo creates slow-motion videos from normal-speed videos.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "flowField_sV.h"

FlowField_sV::FlowField_sV(int width, int height) :
    m_width(width),
    m_height(height)
{
    m_data = new float[2*m_width*m_height];
}

FlowField_sV::~FlowField_sV()
{
    delete m_data;
}

float FlowField_sV::x(int x, int y) const
{
    return m_data[2*(y*m_width+x)+0];
}
float FlowField_sV::y(int x, int y) const
{
    return m_data[2*(y*m_width+x)+1];
}

float* FlowField_sV::data()
{
    return m_data;
}
