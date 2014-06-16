/*
This file is part of slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "imageDisplay.h"
#include <QtCore/QDebug>
#include <QtGui/QPainter>
#include <QtGui/QMenu>
#include <QtGui/QFileDialog>
#include <QtGui/QContextMenuEvent>

#include <QApplication>
#include <QtCore/QSettings>
#include <QtCore/QFileInfo>

#include <cmath>

ImageDisplay::ImageDisplay(QWidget *parent, Qt::WindowFlags f) :
    QFrame(parent, f),
    m_scale(1)
{
    m_aScaling = new QAction(tr("Scale image to widget size"), this);
    m_aScaling->setCheckable(true);
    m_aScaling->setChecked(true);

    m_aExportImage = new QAction(tr("Export image"), this);

    connect(m_aScaling, SIGNAL(triggered()), this, SLOT(repaint()));
    connect(m_aExportImage, SIGNAL(triggered()), this, SLOT(slotExportImage()));

    setContentsMargins(5, 5, 5, 5);
}
ImageDisplay::~ImageDisplay()
{
    delete m_aScaling;
}

void ImageDisplay::trackMouse(bool track)
{
    setMouseTracking(track);
}

void ImageDisplay::loadImage(const QImage img)
{
    if (!img.isNull()) {
        m_image = img;
    }
}
const QImage& ImageDisplay::image() const
{
    return m_image;
}

bool ImageDisplay::loadOverlay(const QImage img)
{
    if (img.size() != m_image.size()) {
        return false;
    }
    m_overlay = img;
    return true;
}
void ImageDisplay::clearOverlay()
{
    m_overlay = QImage();
}

void ImageDisplay::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu menu;
    menu.addAction(m_aScaling);
    menu.addAction(m_aExportImage);
    m_aExportImage->setEnabled(!m_image.isNull());
    menu.exec(e->globalPos());
}


QPointF ImageDisplay::convertCanvasToPixel(QPoint p) const
{
    float scale = m_scale;
    if (m_aScaling->isChecked()) {
        scale = m_scaledImageSize.width()/float(m_image.width());
    }
    return (p - contentsRect().topLeft())/scale;
}
QPointF ImageDisplay::convertCanvasToImage(QPoint p) const
{
    if (!m_aScaling->isChecked()) {
        return m_imageOffset + convertCanvasToPixel(p);
    } else {
        return convertCanvasToPixel(p);
    }
}
QPoint ImageDisplay::convertImageToPixel(QPointF p) const
{
    float scale = m_scale;
    if (m_aScaling->isChecked()) {
        scale = m_scaledImageSize.width()/float(m_image.width());
    }
    return (p*scale + QPointF(contentsRect().topLeft())).toPoint();
}
QPoint ImageDisplay::convertImageToCanvas(QPointF p) const
{
    if (!m_aScaling->isChecked()) {
        return convertImageToPixel(p - m_imageOffset);
    } else {
        return convertImageToPixel(p);
    }

}

void ImageDisplay::mousePressEvent(QMouseEvent *e)
{
    m_states.mouseInitialImagePos = convertCanvasToImage(e->pos());
    m_states.mousePrevPos = e->pos();
    m_states.manhattan = 0;
}

void ImageDisplay::mouseMoveEvent(QMouseEvent *e)
{

    if (e->buttons().testFlag(Qt::LeftButton)) {
        m_states.manhattan += (e->pos()-m_states.mousePrevPos).manhattanLength();
    }
    m_states.mousePrevPos = e->pos();

    if (!m_aScaling->isChecked()) {
#if QT_VERSION < 0x040700
        if (e->buttons().testFlag(Qt::MidButton)) {
#else
        if (e->buttons().testFlag(Qt::MiddleButton)) {
#endif
            // Move the viewport
            QPointF offset = m_states.mouseInitialImagePos - convertCanvasToPixel(e->pos());
            m_imageOffset = offset;
            repaint();
        }
    }

    if (hasMouseTracking() && !m_image.isNull()) {
        int x = e->pos().x() - contentsRect().x();
        int y = e->pos().y() - contentsRect().y();
        if (x < 0 || y < 0 || x >= contentsRect().width() || y >= contentsRect().height()) {
//            qDebug() << "Not inside drawing boundaries.";
            return;
        }
        QPointF pos = convertCanvasToImage(e->pos());
        emit signalMouseMoved(pos.x(), pos.y());
    }
    repaint();
}
void ImageDisplay::mouseReleaseEvent(QMouseEvent *e)
{
    QPointF p0 = m_states.mouseInitialImagePos;
    QPointF releasePos = convertCanvasToImage(e->pos());

    QPointF minPoint = min(p0, releasePos, true);
    QPointF maxPoint = max(p0, releasePos, true);

    qDebug() << p0 << releasePos << minPoint << maxPoint;
    QRectF mouseRect(minPoint, maxPoint);


    if (m_states.countsAsMove()) {
        emit signalRectDrawn(mouseRect);
    }
}

void ImageDisplay::wheelEvent(QWheelEvent *e)
{
    if (!m_aScaling->isChecked()) {
        QPointF mouseOffset = convertCanvasToImage(e->pos());
        if (e->delta() > 0) {
            if (m_scale < 20) {
                m_scale *= 1.4;
            }
        } else {
            if (m_scale > float(contentsRect().width())/(2*m_image.width())) {
                m_scale /= 1.4;
            }
        }
        m_imageOffset = mouseOffset - (e->pos()-contentsRect().topLeft())/m_scale;
        repaint();
    }
}


void ImageDisplay::paintEvent(QPaintEvent *e)
{
    QFrame::paintEvent(e);
    if (!m_image.isNull()) {
        QPainter p(this);

        QImage subImg;
        if (m_aScaling->isChecked()) {

            // Scale to frame size
            subImg = m_image.scaled(contentsRect().size(), Qt::KeepAspectRatio);
            m_scaledImageSize = subImg.size();

        } else {

            // User-defined scaling
            subImg = m_image.copy(std::floor(m_imageOffset.x()), std::floor(m_imageOffset.y()),
                                         std::ceil(contentsRect().width()/m_scale+1), std::ceil(contentsRect().height()/m_scale+1));
            subImg = subImg.scaled(m_scale*subImg.size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
            subImg = subImg.copy(m_scale*(m_imageOffset.x()-floor(m_imageOffset.x())),
                                 m_scale*(m_imageOffset.y()-floor(m_imageOffset.y())),
                                 contentsRect().width(),
                                 contentsRect().height());
        }

        p.drawImage(contentsRect().topLeft(), subImg);

        if (m_states.countsAsMove() && !QApplication::mouseButtons().testFlag(Qt::NoButton)) {
            QRect r;
            QPoint origin = convertImageToCanvas(m_states.mouseInitialImagePos);

            r.setTopLeft(min(m_states.mousePrevPos, origin));
            r.setWidth(abs(m_states.mousePrevPos.x()-origin.x()));
            r.setHeight(abs(m_states.mousePrevPos.y()-origin.y()));

            p.drawRect(r);
        }

    }
}

void ImageDisplay::slotExportImage()
{
    Q_ASSERT(!m_image.isNull());

    QSettings settings;

    QFileDialog dialog(this, tr("Export render preview to image"));
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setDirectory(settings.value("directories/imageDisplay", QDir::homePath()).toString());
    if (dialog.exec() == QDialog::Accepted) {
        m_image.save(dialog.selectedFiles().at(0));
        settings.setValue("directories/imageDisplay", QFileInfo(dialog.selectedFiles().at(0)).absolutePath());
    }
}




qreal ImageDisplay::clamp(qreal val, qreal min, qreal max) const
{
    return (val < min) ? min :  ( (val > max) ? max : val );
}
QPointF ImageDisplay::max(QPointF p1, QPointF p2, bool limitToImage) const
{
    QPointF p(qMax(p1.x(), p2.x()), qMax(p1.y(), p2.y()));
    if (limitToImage) {
        p.rx() = clamp(p.x(), 0, m_image.width()-1);
        p.ry() = clamp(p.y(), 0, m_image.height()-1);
    }
    return p;
}

QPointF ImageDisplay::min(QPointF p1, QPointF p2, bool limitToImage) const
{
    QPointF p(qMin(p1.x(), p2.x()), qMin(p1.y(), p2.y()));
    if (limitToImage) {
        p.rx() = clamp(p.x(), 0, m_image.width()-1);
        p.ry() = clamp(p.y(), 0, m_image.height()-1);
    }
    return p;
}

QPoint ImageDisplay::min(QPoint p1, QPoint p2) const
{
    return QPoint(qMin(p1.x(), p2.x()), qMin(p1.y(), p2.y()));
}
QPoint ImageDisplay::max(QPoint p1, QPoint p2) const
{
    return QPoint(qMax(p1.x(), p2.x()), qMax(p1.y(), p2.y()));
}
