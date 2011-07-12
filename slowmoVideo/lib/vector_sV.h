/*
slowmoVideo creates slow-motion videos from normal-speed videos.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef VECTOR_SV_H
#define VECTOR_SV_H

#include <QPointF>

class Vector_sV
{
public:
    Vector_sV();
    Vector_sV(float x, float y);
    Vector_sV(float fromX, float fromY, float toX, float toY);

    QPointF toQPointF() const;

    float x() const;
    float y() const;
    float& rx();
    float& ry();

    float length() const;

    Vector_sV& rotate90(bool counterclock = true);

    Vector_sV operator *(float factor);
    Vector_sV operator +(const Vector_sV &other);
    Vector_sV operator -(const Vector_sV &other);

    Vector_sV& operator *=(float factor);
    Vector_sV& operator +=(const Vector_sV &other);
    Vector_sV& operator -=(const Vector_sV &other);

    bool operator ==(const Vector_sV &other);
    bool operator !=(const Vector_sV &other);

private:
    float m_x;
    float m_y;
};

Vector_sV operator *(const float &factor, const Vector_sV &other);

#endif // VECTOR_SV_H
