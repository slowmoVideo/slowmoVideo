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
    enum GLFormat { GLFormat_RGB };

    /** Constructor for uninitialized data */
    FlowField_sV(int width, int height);

    /** Constructor for data read from OpenGL in the given \param format. */
    FlowField_sV(int width, int height, float *data, GLFormat format);

    ~FlowField_sV();

    /** \fn data()
      \return Pointer to the raw data. See the class description for the accurate format.
      */
    /** \fn dataSize()
      \return Number of elements in the data array.
      */
    /** \fn width()
      \return Width of the flow field
      */
    /** \fn height()
      \return Height of the flow field
      */
    /** \fn x(int, int)
      \return Flow in x direction at position \c (x|y)
      */
    /** \fn setX(int, int, float)
      Sets the flow in x direction for the position <code>(x|y)</code>
      */
    float* data();
    int dataSize() const { return 2*m_width*m_height; }
    int width() const { return m_width; }
    int height() const { return m_height; }
    float x(int x, int y) const;
    float y(int x, int y) const;
    void setX(int x, int y, float value);
    void setY(int x, int y, float value);

    bool operator==(const FlowField_sV& other) const;

private:
    int m_width;
    int m_height;
    float *m_data;
};

#endif // FLOWFIELD_SV_H
