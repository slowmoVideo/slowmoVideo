/*
slowmoUI is a user interface for slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef NODE_H
#define NODE_H

#include <QtGlobal>

class Node
{
public:
    Node();
    Node(const qreal &x, const qreal &y);

    bool operator<(const Node &other) const;
    bool operator==(const Node &other) const;
    Node operator-(const Node &other) const;
    Node operator+(const Node &other) const;
    void operator+=(const Node &other);
    void operator-=(const Node &other);

    qreal x() const;
    qreal y() const;
    qreal xUnmoved() const;
    qreal yUnmoved() const;

    qreal setX(qreal x);
    qreal setY(qreal y);

    void select(bool);
    bool selected() const;

    void move(const Node &dist);
    void abortMove();
    void confirmMove();


private:
    qreal m_x;
    qreal m_y;

    qreal m_moveX;
    qreal m_moveY;

    bool m_selected;
};

QDebug operator<<(QDebug qd, const Node& n);

#endif // NODE_H
