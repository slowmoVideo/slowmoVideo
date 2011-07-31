/*
slowmoUI is a user interface for slowmoVideo.
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

#include <QtCore/QSettings>
#include <QtCore/QFileInfo>

ImageDisplay::ImageDisplay(QWidget *parent, Qt::WindowFlags f) :
    QFrame(parent, f)
{
    m_aScaling = new QAction("Scale image to widget size", this);
    m_aScaling->setCheckable(true);
    m_aScaling->setChecked(true);

    m_aExportImage = new QAction("Export image", this);

    bool b = true;
    b &= connect(m_aScaling, SIGNAL(triggered()), this, SLOT(repaint()));
    b &= connect(m_aExportImage, SIGNAL(triggered()), this, SLOT(slotExportImage()));
    Q_ASSERT(b);

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
    m_image = img;
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
void ImageDisplay::mouseMoveEvent(QMouseEvent *e)
{
    if (hasMouseTracking() && !m_image.isNull()) {
        int x = e->pos().x() - contentsRect().x();
        int y = e->pos().y() - contentsRect().y();
        if (x < 0 || y < 0 || x >= contentsRect().width() || y >= contentsRect().height()) {
            qDebug() << "Not inside drawing boundaries.";
            return;
        }
        if (!m_aScaling->isChecked()) {
            emit signalMouseMoved(x, y);
        } else {
            // The image has been scaled by this factor
            float scalingFactor = qMin(float(m_image.width())/width(), float(m_image.height())/height());
            // To get back the original image coordinates, the mouse coordinates have to unscaled.
            emit signalMouseMoved(x/scalingFactor, y/scalingFactor);
        }
    }
}


void ImageDisplay::paintEvent(QPaintEvent *e)
{
    QFrame::paintEvent(e);
    if (!m_image.isNull()) {
        QPainter p(this);
        if (m_aScaling->isChecked()) {
            p.drawImage(contentsRect().topLeft(), m_image.scaled(contentsRect().size(), Qt::KeepAspectRatio));
            p.drawImage(contentsRect().topLeft(), m_image.scaled(contentsRect().size(), Qt::KeepAspectRatio));
        } else {
            p.drawImage(contentsRect().topLeft(), m_image);
            p.drawImage(contentsRect().topLeft(), m_overlay);
        }
    }
}

void ImageDisplay::slotExportImage()
{
    Q_ASSERT(!m_image.isNull());

    QSettings settings;

    QFileDialog dialog(this, "Export render preview to image");
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setDirectory(settings.value("directories/imageDisplay", QDir::homePath()).toString());
    if (dialog.exec() == QDialog::Accepted) {
        m_image.save(dialog.selectedFiles().at(0));
        settings.setValue("directories/imageDisplay", QFileInfo(dialog.selectedFiles().at(0)).absolutePath());
    }
}
