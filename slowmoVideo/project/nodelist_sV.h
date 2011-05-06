/*
slowmoUI is a user interface for slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef NODELIST_SV_H
#define NODELIST_SV_H

#include <QList>
#include <QtGlobal>

#include "node_sV.h"

class NodeList_sV
{
public:
    NodeList_sV(float minDist = 1/30.0f);
    void setMaxY(qreal time);

    qreal sourceTime(qreal targetTime) const;
    qreal startTime() const;
    qreal endTime() const;
    qreal totalTime() const;

    /**
      Add a new node at the given position.
      @return true if the node has been added. The node is NOT added
      if it is too close to another node.
      */
    bool add(const Node_sV &node);
    uint deleteSelected();

    void unselectAll();

    void shift(qreal after, qreal by);
    /**
      Move the selected nodes by the given time vector.
      Only succeeds if the nodes are still within valid bounds.
      A move has to be either confirmed or aborted.
      */
    void moveSelected(const Node_sV &time);
    /**
      Confirm the move on all nodes.
      */
    void confirmMove();
    /**
      Abort the move on all nodes. Resets the temporary movement vector.
      */
    void abortMove();


    /**
      @return The position of the node whose target time (x()) is <= time,
      or -1 if there is no such node.
      */
    int find(qreal time) const;

    /**
      @return The index of the node whose time is equal or greater than
      the given time, or -1 if there is no such node.
      */
    int nodeAfter(qreal time) const;
    /**
      @return A pointer to the node at the given time (or not further
      than minDist away), or NULL if there is no node at t.
      */
    const Node_sV* near(qreal t) const;
    const Node_sV& at(int i) const;
    Node_sV& operator[](int i);
    int size() const;

    /**
      @return false if nodes are not in a valid position.
      Nodes must be ordered, and the minimum distance (on the y axis)
      must be at least m_minDist.
      */
    bool validate() const;

private:
    qreal m_maxY;
    QList<Node_sV> m_list;
    const float m_minDist;
};

QDebug operator<<(QDebug qd, const NodeList_sV &list);

#endif // NODELIST_SV_H
