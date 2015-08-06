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

#include "node_sV.h"
#include "segmentList_sV.h"
#include "canvasObject_sV.h"
#include "../lib/defs_sV.hpp"

#include <QList>
#include <QtGlobal>

/**
  \brief Represents a curve defined by a Node_sV list.

  This object can be queried for the source time given an output time.

  The curve is (ensured to be) injective, i.e.
  \f$ t_1 \neq t_2 \rightarrow f(t_1) \neq f(t_2) \f$
  with \f$ t_1,t_2 \in \f$ target time, \f$ f(t_1),f(t_2) \in \f$ source time. This means that
  there is alwas a non-ambiguous answer to the question:
  <em>Which frame from the input video has to be displayed at output time \f$ t \f$?</em>
  */
class NodeList_sV
{
public:
    NodeList_sV(float minDist = 1/30.0f);

    /// For sorting objects
    struct PointerWithDistance {

        /// Defines the order on object types. Nodes will come first in a sorted list.
        enum ObjectType { Node = 1, Handle = 2, Segment = 3, Tag = 4 };
        /// Pointer to the object
        const CanvasObject_sV* ptr;
        /// Distance to the object from the search position (e.g. mouse position)
        qreal dist;
        /// The object type should only be used for sorting!
        ObjectType type;

        bool operator <(const PointerWithDistance &other) const {
            return type < other.type || (type == other.type &&  dist < other.dist);
        }
        PointerWithDistance(const CanvasObject_sV* ptr, qreal dist, ObjectType type) :
            ptr(ptr),
            dist(dist),
            type(type)
        { }
    };


    void setMaxY(qreal time); ///< Sets the maximum y value that is allowed, usually the duration of the input.

    qreal sourceTime(qreal targetTime) const; ///< Calculates the source time in seconds for the given output time.
    qreal startTime(bool useMoved = false) const; ///< Time of the first node. useMoved uses the unconfirmed position of the node while it is moved.
    qreal endTime(bool useMoved = false) const; ///< Time of the rightmost node. See totalTime() for the curve length.
    qreal totalTime() const; ///< Length of the curve, ignores space (startTime())at the beginning.
    bool isInsideCurve(qreal targetTime, bool useMoved = false) const; ///< Returns true if startTime <= targetTime <= endTime

    /**
      Add a new node at the given position.
      @return true if the node has been added. The node is NOT added
      if it is too close to another node.
      */
    bool add(Node_sV node);
    uint deleteSelected();
    void deleteNode(int index);

    void select(const Node_sV *node, bool newSelection = true);
    void unselectAll();

    void shift(qreal after, qreal by);
    /**
      Move the selected nodes by the given time vector.
      Only succeeds if the nodes are still within valid bounds.
      A move has to be either confirmed or aborted.
      */
    void moveSelected(const Node_sV &time,bool snap = false);
    /**
      Confirm the move on all nodes.
      */
    void confirmMove();
    /**
      Abort the move on all nodes. Resets the temporary movement vector.
      */
    void abortMove();

    void moveHandle(const NodeHandle_sV *handle, Node_sV relPos);

    /// Sets the curve type for the segment at time \c segmentTime.
    void setCurveType(qreal segmentTime, CurveType type);

    void fixHandles(int leftIndex);

    void setSpeed(qreal segmentTime, qreal speed);



    /**
      \brief Returns the \c node's index in the node list
      \return -1 if the node could not be located
      */
    int indexOf(const Node_sV *node) const;
    /**
      @return The position of the node whose target time (x()) is <= time,
      or -1 if there is no such node.
      */
    int find(qreal time) const;

    /**
      @return The position of the first node in the list which is within a radius
      of \c tdelta around the point <tt>(tx|ty)</tt>, or -1 if no such node exists.
      */
    int find(QPointF pos, qreal tdelta) const;
    /**
      \brief Searches for a segment (or path) between two nodes at time \c tx.

      If a left or right node does not exist (when \c tx is outside of the curve), the return
      value for this index is -1.
      */
    void findBySegment(qreal tx, int& leftIndex_out, int& rightIndex_out) const;
    /**
      \brief Searches for node objects (nodes, handles, and segments) around a position.
      \param pos Center of the search position.
      \param tmaxdist This is the search radius. Elements are included if their euclidian distance
       to \c pos is <= tmaxdist, except for segments where only the x position is taken into account.
       */
    QList<PointerWithDistance> objectsNear(QPointF pos, qreal tmaxdist) const;

    /**
      @return The index of the node whose time is equal or greater than
      the given time, or -1 if there is no such node.
      */
    int nodeAfter(qreal time) const;

    const Node_sV& at(int i) const;
    Node_sV& operator [](int i);
    int size() const;

    /**
      @return false if nodes are not in a valid position.
      Nodes must be ordered, and the minimum distance (on the y axis)
      must be at least m_minDist.
      */
    bool validate() const;

    SegmentList_sV* segments();

private:
    qreal m_maxY;
    QList<Node_sV> m_list;
    SegmentList_sV m_segments;
    const float m_minDist;

    qreal bezierSourceTime(qreal targetTime, QPointF p0, QPointF p1, QPointF p2, QPointF p3) const;
    inline qreal dist2(QPointF point) const;
};

QDebug operator<<(QDebug qd, const NodeList_sV &list);

#endif // NODELIST_SV_H
