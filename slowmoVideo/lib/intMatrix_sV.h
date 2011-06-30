/*
slowmoVideo creates slow-motion videos from normal-speed videos.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef INTMATRIX_SV_H
#define INTMATRIX_SV_H

/** \brief Simple matrix that can add image data to itself. */
class IntMatrix_sV
{
public:
    IntMatrix_sV(int width, int height, int channels);
    ~IntMatrix_sV();

    int width() const;
    int height() const;
    int channels() const;

    void operator +=(const unsigned char *bytes);
    void operator /=(int divisor);

    unsigned char* toBytesArray() const;
    const int* data() const;

private:
    int m_width;
    int m_height;
    int m_channels;
    int *m_data;
};

#endif // INTMATRIX_SV_H
