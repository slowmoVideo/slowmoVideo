/*
slowmoVideo creates slow-motion videos from normal-speed videos.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "flowField_sV.h"

#include <QDebug>

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

void FlowField_sV::setX(int x, int y, float value)
{
    m_data[2*(y*m_width+x)+0] = value;
}
void FlowField_sV::setY(int x, int y, float value)
{
    m_data[2*(y*m_width+x)+1] = value;
}

float* FlowField_sV::data()
{
    return m_data;
}

bool FlowField_sV::operator ==(const FlowField_sV& other) const
{
    bool equal = true;

    equal &= m_width == other.m_width;
    if (!equal) { qDebug() << "Width differs."; }
    equal &= m_height == other.m_height;
    if (!equal) { qDebug() << "Height differs."; }

    if (equal) {
        for (int y = 0; y < m_height; y++) {
            for (int x = 0; x < m_width; x++) {
                if (this->x(x,y) != other.x(x,y)) {
                    qDebug() << "x Value differs at " << x << "," << y << ": "
                             << this->x(x,y) << "/" << other.x(x,y);
                    equal = false;
                    goto endLoop;
                }
                if (this->y(x,y) != other.y(x,y)) {
                    qDebug() << "y Value differs at " << x << "," << y << ": "
                             << this->y(x,y) << "/" << other.y(x,y);
                    equal = false;
                    goto endLoop;
                }
            }
        }
        endLoop: ;
    }


    return equal;
}
