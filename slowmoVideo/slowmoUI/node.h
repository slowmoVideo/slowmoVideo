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

    qreal x() const;
    qreal y() const;

    void select(bool);
    bool selected() const;

private:
    qreal m_x;
    qreal m_y;
    bool m_selected;
};

QDebug operator<<(QDebug qd, const Node& n);

#endif // NODE_H
