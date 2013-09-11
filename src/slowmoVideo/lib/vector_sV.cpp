/*
slowmoVideo creates slow-motion videos from normal-speed videos.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "vector_sV.h"
#include <cmath>

Vector_sV::Vector_sV()
{
}

Vector_sV::Vector_sV(float x, float y) :
    m_x(x),
    m_y(y)
{
}

Vector_sV::Vector_sV(float fromX, float fromY, float toX, float toY) :
    m_x(toX - fromX),
    m_y(toY - fromY)
{
}

QPointF Vector_sV::toQPointF() const
{
    return QPointF(m_x, m_y);
}

float Vector_sV::x() const { return m_x; }
float Vector_sV::y() const { return m_y; }
float& Vector_sV::rx() { return m_x; }
float& Vector_sV::ry() { return m_y; }

float Vector_sV::length() const
{
    return std::sqrt(std::pow(m_x, 2) + std::pow(m_y, 2));
}

Vector_sV& Vector_sV::rotate90(bool counterclock)
{
    float tmp = m_x;
    if (counterclock) {
        m_x = m_y;
        m_y = -tmp;
    } else {
        m_x = -m_y;
        m_y = tmp;
    }
    return *this;
}



Vector_sV Vector_sV::operator *(float factor)
{
    return Vector_sV(factor * m_x, factor * m_y);
}
Vector_sV Vector_sV::operator +(const Vector_sV &other)
{
    return Vector_sV(m_x + other.m_x, m_y + other.m_y);
}
Vector_sV Vector_sV::operator -(const Vector_sV &other)
{
    return Vector_sV(m_x - other.m_x, m_y - other.m_y);
}
Vector_sV operator *(const float &factor, const Vector_sV &other)
{
    return Vector_sV(factor * other.x(), factor * other.y());
}



Vector_sV& Vector_sV::operator *=(float factor)
{
    m_x *= factor;
    m_y *= factor;
    return *this;
}
Vector_sV& Vector_sV::operator +=(const Vector_sV &other)
{
    m_x += other.m_x;
    m_y += other.m_y;
    return *this;
}
Vector_sV& Vector_sV::operator -=(const Vector_sV &other)
{
    m_x -= other.m_x;
    m_y -= other.m_y;
    return *this;
}



bool Vector_sV::operator ==(const Vector_sV &other)
{
    return m_x == other.m_x && m_y == other.m_y;
}

bool Vector_sV::operator !=(const Vector_sV &other)
{
    return m_x != other.m_x || m_y != other.m_y;
}
