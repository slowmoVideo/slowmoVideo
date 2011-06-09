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

class FlowField_sV
{
public:
    FlowField_sV(int width, int height);
    ~FlowField_sV();

    float* data();
    int dataSize() const { return 2*m_width*m_height; }
    int width() const { return m_width; }
    int height() const { return m_height; }
    float x(int x, int y) const;
    float y(int x, int y) const;

private:
    int m_width;
    int m_height;
    float *m_data;
};

#endif // FLOWFIELD_SV_H
