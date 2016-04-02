/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef IMAGEDISPLAY_H
#define IMAGEDISPLAY_H

#include <QFrame>
#include <QtCore/QRect>

/**
  \brief Simple image display.

  Images can be scaled to the frame size and exported to a file
  via the context menu.
  */
class ImageDisplay : public QFrame
{
    Q_OBJECT
public:
    explicit ImageDisplay(QWidget *parent = 0, Qt::WindowFlags f = 0);
    ~ImageDisplay();

    void trackMouse(bool track);

    /// \return The image that is currently displayed
    const QImage& image() const;

public slots:
    /// Loads the given image; does \em not call repaint()!
    void loadImage(const QImage img);
    /// Loads the overlay that will be painted over the image; does \em not call repaint() either.
    /// \return \c false if the image sizes do not match
    bool loadOverlay(const QImage img);
    void clearOverlay();

signals:
    void signalMouseMoved(float x, float y);
    void signalMousePressed(float x, float y);
    void signalRectDrawn(QRectF imageRect);

protected slots:
    virtual void paintEvent(QPaintEvent *e);
    virtual void contextMenuEvent(QContextMenuEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void wheelEvent(QWheelEvent *e);

private:
    QImage m_image;
    QImage m_overlay;

    QAction *m_aScaling;
    QAction *m_aExportImage;

    float m_scale;
    QPointF m_imageOffset;
    QSize m_scaledImageSize;

    struct {
        QPointF mouseInitialImagePos;

        QPoint mousePrevPos;

        int manhattan;

        bool countsAsMove() { return manhattan > 4; }

    } m_states;

    /// Convert canvas coordinates to image coordinates.
    /// The image may have an offset.
    QPointF convertCanvasToImage(QPoint p) const;
    /// Convert canvas coordinates to pixel coordinates (ignores the image offset)
    QPointF convertCanvasToPixel(QPoint p) const;
    QPoint convertImageToCanvas(QPointF p) const;
    QPoint convertImageToPixel(QPointF p) const;

    qreal clamp(qreal val, qreal min, qreal max) const;
    QPointF max(QPointF p1, QPointF p2, bool limitToImage) const;
    QPointF min(QPointF p1, QPointF p2, bool limitToImage) const;
    QPoint min(QPoint p1, QPoint p2) const;
    QPoint max(QPoint p1, QPoint p2) const;

private slots:
    void slotExportImage();

};

#endif // IMAGEDISPLAY_H
