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

/**
  Represents a float vector. Supports adding and rotation by 90 degrees.
  */
class Vector_sV
{
public:
    /**
      \fn Vector_sV()
      \brief Creates a zero vector.
      */
    /**
      \fn Vector_sV(float x, float y)
      \brief Creates a vector with the given coordinates.
      */
    /**
      \fn Vector_sV(float fromX, float fromY, float toX, float toY)
      \brief Creates a vector from a \c from and a \c to point.
      */
    Vector_sV();
    Vector_sV(float x, float y);
    Vector_sV(float fromX, float fromY, float toX, float toY);

    /// Converts the vector to a point
    QPointF toQPointF() const;

    /// x component. See rx() for a reference to the x value.
    float x() const;
    /// y component. See ry() for a reference to the y value.
    float y() const;
    /// Reference to the x value, allows direct modification.
    float& rx();
    /// Reference to the y value, allows direct modification.
    float& ry();

    /// Calculates the euclidian length of the vector.
    float length() const;

    /// Rotates the vector by 90 degrees, clockwise if \c counterclock is set to \c false.
    Vector_sV& rotate90(bool counterclock = true);

    /// Multiplicates a vector with a constant factor.
    Vector_sV operator *(float factor);
    /// Adds two vectors.
    Vector_sV operator +(const Vector_sV &other);
    /// Subtracts two vectors.
    Vector_sV operator -(const Vector_sV &other);

    /// Multiplicates this vector with a factor.
    Vector_sV& operator *=(float factor);
    /// Adds a vector to this vector.
    Vector_sV& operator +=(const Vector_sV &other);
    /// Subtracts a vector from this vector.
    Vector_sV& operator -=(const Vector_sV &other);

    /// Equality test
    bool operator ==(const Vector_sV &other);
    /// Inequality test
    bool operator !=(const Vector_sV &other);

private:
    float m_x;
    float m_y;
};

Vector_sV operator *(const float &factor, const Vector_sV &other);

#endif // VECTOR_SV_H
