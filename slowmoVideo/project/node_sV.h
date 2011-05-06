/*
slowmoUI is a user interface for slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef NODE_SV_H
#define NODE_SV_H

#include <QtGlobal>

class Node_sV
{
public:
    Node_sV();
    Node_sV(const qreal &x, const qreal &y);

    bool operator<(const Node_sV &other) const;
    bool operator==(const Node_sV &other) const;
    Node_sV operator-(const Node_sV &other) const;
    Node_sV operator+(const Node_sV &other) const;
    void operator+=(const Node_sV &other);
    void operator-=(const Node_sV &other);

    qreal x() const;
    qreal y() const;
    qreal xUnmoved() const;
    qreal yUnmoved() const;

    qreal setX(qreal x);
    qreal setY(qreal y);

    void select(bool);
    bool selected() const;

    void move(const Node_sV &dist);
    void abortMove();
    void confirmMove();


private:
    qreal m_x;
    qreal m_y;

    qreal m_moveX;
    qreal m_moveY;

    bool m_selected;
};

QDebug operator<<(QDebug qd, const Node_sV& n);

#endif // NODE_SV_H
