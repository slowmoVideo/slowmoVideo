/*
slowmoVideo creates slow-motion videos from normal-speed videos.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef FLOWFIELD_SV_H
#define FLOWFIELD_SV_H

/**
  \brief Represents a dense optical flow field.

  Values are internally stored in row order:
  \code
  [ x0 y0  x1 y1  x2 y2 ...
    xi yi  xj yj  xk yk ... ]
  \endcode

  \see FlowRW_sV for reading and writing flow fields.
  */
class FlowField_sV
{
public:
    /** OpenGL format */
    enum GLFormat { GLFormat_RGB, GLFormat_RG };

    static float nullValue;

    /** Constructor for uninitialized data */
    FlowField_sV(int width, int height);

    /** Constructor for data read from OpenGL in the given \c format. */
    FlowField_sV(int width, int height, float *data, GLFormat format);

    ~FlowField_sV();

    /** \fn data()
      \return Pointer to the raw data. See the class description for the accurate format.
      */
    /** \fn dataSize()
      \return Number of elements in the data array.
      */

    /// Pointer to the raw data. See the class description for the accurate format.
    float* data();
    /// Number of elements in the data array.
    int dataSize() const { return 2*m_width*m_height; }

    /// Width of the flow field
    int width() const { return m_width; }
    /// Height of the flow field
    int height() const { return m_height; }

    /// Flow in x direction at position <code>(x|y)</code>
    float x(int x, int y) const;
    /// Flow in y direction at position <code>(x|y)</code>
    float y(int x, int y) const;
    /// Reference to the value x(x, y)
    float& rx(int x, int y);
    /// Reference to the value y(x, y)
    float& ry(int x, int y);
    /// Sets the flow in x direction for the position <code>(x|y)</code>
    void setX(int x, int y, float value);
    /// Sets the flow in y direction for the position <code>(x|y)</code>
    void setY(int x, int y, float value);

    /// Equality test. Equal if all entries match.
    bool operator==(const FlowField_sV& other) const;

private:
    int m_width;
    int m_height;
    float *m_data;
};

#endif // FLOWFIELD_SV_H
