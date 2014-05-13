/*
slowmoUI is a user interface for slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef CANVAS_H
#define CANVAS_H

#include "../project/node_sV.h"
#include "../project/nodeList_sV.h"
#include "../project/project_sV.h"
#include "../project/tag_sV.h"

#include <QWidget>
#include <QList>
#include <QSettings>



#define NODE_RADIUS 6
#define SELECT_RADIUS 12
#define HANDLE_RADIUS 4
#define MOVE_THRESHOLD 3
#define SCROLL_FACTOR 3
#define ZOOM_FACTOR 1.414



class QColor;
class QPoint;
class CanvasTools;
class ShutterFunctionDialog;

namespace Ui {
    class Canvas;
}



/**
  This class is for building helper objects for the signals&slots mechanism
  for passing pointers to objects which are not QObjects.
  */
class TransferObject : public QObject {
    Q_OBJECT

public:
    CanvasObject_sV* objectPointer;
    enum Reason {
        ACTION_DELETE,
        ACTION_RENAME,
        ACTION_SETTIME,
        ACTION_SNAPIN
    } reason;


    TransferObject() : objectPointer(NULL), reason(ACTION_SNAPIN) {}
    TransferObject(CanvasObject_sV* objectPointer, Reason reason) :
        objectPointer(objectPointer), reason(reason) {}
};


class Project_sV;

/**
  \brief Canvas for drawing motion curves.

  \todo Frame lines on high zoom
  \todo Custom speed factor to next node
  */
class Canvas : public QWidget
{
    Q_OBJECT

    friend class CanvasTools;

public:
    explicit Canvas(Project_sV *project, QWidget *parent = 0);
    ~Canvas();

    enum ToolMode { ToolMode_Select, ToolMode_Move };
    enum Abort { Abort_General, Abort_Selection };

    static QColor lineCol;
    static QColor selectedLineCol;
    static QColor nodeCol;
    static QColor gridCol;
    static QColor fatGridCol;
    static QColor minGridCol;
    static QColor selectedCol;
    static QColor hoverCol;
    static QColor srcTagCol;
    static QColor outTagCol;
    static QColor handleLineCol;
    static QColor backgroundCol;
    static QColor shutterRegionCol;
    static QColor shutterRegionBoundCol;

    void load(Project_sV *project);

    void showHelp(bool show);
    void toggleHelp();

    const QPointF prevMouseTime() const;
    const float prevMouseInFrame() const;

public slots:
    void slotAbort(Canvas::Abort abort);
    void slotAddTag();
    void slotDeleteNodes();
    void slotSetToolMode(Canvas::ToolMode mode);

signals:
    void signalMouseInputTimeChanged(qreal frame);
    void signalMouseCurveSrcTimeChanged(qreal frame);
    void nodesChanged();

protected:
    void paintEvent(QPaintEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void wheelEvent(QWheelEvent *);
    void keyPressEvent(QKeyEvent *event);
    void leaveEvent(QEvent *);
    void contextMenuEvent(QContextMenuEvent *);

private:
    Ui::Canvas *ui;
    QSettings m_settings;
    Project_sV *m_project;
    ShutterFunctionDialog *m_shutterFunctionDialog;
    bool m_mouseWithinWidget;
    int m_distLeft;
    int m_distBottom;
    int m_distRight;
    int m_distTop;
    Node_sV m_t0;    ///< Viewport, bottom left
    Node_sV m_tmax;  ///< Upper bounds for the viewport (so the user does not get lost by zooming too far up)
    float m_secResX; ///< How many pixels wide is one output second?
    float m_secResY; ///< How many pixels wide is one input second?

    bool m_showHelp;

    NodeList_sV *m_nodes;
    QList<Tag_sV> *m_tags;

    ToolMode m_mode;
    /**
      Saves states about mouse events.
      The prev... variables are updated when the mouse moves,
      the initial... variables only on mouse clicks.
      */
    struct {
        bool nodesMoved;
        bool selectAttempted;
        bool moveAborted;

        QPoint prevMousePos;
        QPoint initialMousePos;
        QPointF contextmenuMouseTime;
        Node_sV initial_t0;

        Qt::KeyboardModifiers prevModifiers;
        Qt::KeyboardModifiers initialModifiers;

        Qt::MouseButtons initialButtons;

        const CanvasObject_sV *initialContextObject;

        void reset() {
            moveAborted = false;
            nodesMoved = false;
            selectAttempted = false;
            initialContextObject = NULL;
            travelledDistance = 0;
        }
        void travel(int length) { travelledDistance += length; }
        bool countsAsMove() { return travelledDistance >= MOVE_THRESHOLD; }

    private:
        int travelledDistance;
    } m_states;

    /*
      The transfer objects to each action defines the action to take
      when the slot is called, and additionally stores a pointer to the
      object it was called on (the object is known in the context menu event).
      */
    QAction *m_aDeleteNode; TransferObject m_toDeleteNode;
    QAction *m_aSnapInNode; TransferObject m_toSnapInNode;
    QAction *m_aDeleteTag; TransferObject m_toDeleteTag;
    QAction *m_aRenameTag; TransferObject m_toRenameTag;
    QAction *m_aSetTagTime; TransferObject m_toSetTagTime;
    QSignalMapper *m_hackMapper;

    QSignalMapper *m_curveTypeMapper;
    QSignalMapper *m_handleMapper;
    QSignalMapper *m_speedsMapper;
    QAction *m_aLinear;
    QAction *m_aBezier;
    QAction *m_aResetLeftHandle;
    QAction *m_aResetRightHandle;
    QAction *m_aCustomSpeed;
    QAction *m_aShutterFunction;
    std::vector<QAction *> m_aSpeeds;

    Node_sV convertCanvasToTime(const QPoint &p) const;
    QPoint convertTimeToCanvas(const Node_sV &p) const;
    QPoint convertTimeToCanvas(const QPointF &p) const;
    QPointF convertDistanceToTime(const QPoint &p) const;
    QPoint convertTimeToDistance(const QPointF &time) const;

    /** \return The distance in px converted to time */
    float delta(int px) const;

    bool insideCanvas(const QPoint& pos);

    bool selectAt(const QPoint& pos, bool addToSelection = false);

    void drawModes(QPainter &davinci, int top, int right);

    const CanvasObject_sV* objectAt(QPoint pos, Qt::KeyboardModifiers modifiers) const;

    void setCurveSpeed(double speed);

private slots:
    void slotRunAction(QObject *o);
    void slotChangeCurveType(int curveType);
    void slotResetHandle(const QString &position);
    void slotSetSpeed();
    void slotSetSpeed(QString s);
    void slotSetShutterFunction();
    void slotZoomIn();
    void slotZoomOut();

private:
    void zoom(bool in, QPoint pos);

    QRect leftDrawingRect(int y, const int height = 12, const int min = -1, const int max = -1) const;
    QRect bottomDrawingRect(int x, const int width = 160, const int min = -1, const int max = -1, bool rightJustified = true) const;

};

QDebug operator<<(QDebug qd, const Canvas::ToolMode &mode);
QDebug operator<<(QDebug qd, const Canvas::Abort &abort);

QString toString(TransferObject::Reason reason);

#endif // CANVAS_H
