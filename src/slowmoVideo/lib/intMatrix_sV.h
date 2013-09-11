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

/**
  \brief Simple matrix that can add image data to itself.

  This matrix is used for shutter simulation (i.e. motion blur).
*/
class IntMatrix_sV
{
public:
    /**
      \brief Creates a new image matrix.

      \c channels should match the number of channels of the images that will be added.
      */
    IntMatrix_sV(int width, int height, int channels);
    ~IntMatrix_sV();

    /// Matrix width
    int width() const;
    /// Matrix height
    int height() const;
    /// Number of colour channels
    int channels() const;

    /// Adds the input bytes (as row-wise image data, usually) to this matrix.
    void operator +=(const unsigned char *bytes);
    /// Scales the matrix, e.g. by the number of images added.
    void operator /=(int divisor);

    /// Converts the image to a byte array. Internal values are stored as int.
    unsigned char* toBytesArray() const;
    /// Image data.
    const int* data() const;

private:
    int m_width;
    int m_height;
    int m_channels;
    int *m_data;
};

#endif // INTMATRIX_SV_H
