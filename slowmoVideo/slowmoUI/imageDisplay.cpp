/*
slowmoUI is a user interface for slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "imageDisplay.h"
#include <QtGui/QPainter>
#include <QtGui/QMenu>
#include <QtGui/QFileDialog>
#include <QtGui/QContextMenuEvent>

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
}
ImageDisplay::~ImageDisplay()
{
    delete m_aScaling;
}

void ImageDisplay::loadImage(const QImage img)
{
    m_image = img;
}

void ImageDisplay::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu menu;
    menu.addAction(m_aScaling);
    menu.addAction(m_aExportImage);
    m_aExportImage->setEnabled(!m_image.isNull());
    menu.exec(e->globalPos());
}


void ImageDisplay::paintEvent(QPaintEvent *e)
{
    QFrame::paintEvent(e);
    if (!m_image.isNull()) {
        QPainter p(this);
        if (m_aScaling->isChecked()) {
            p.drawImage(5, 5, m_image.scaled(width()-10, height()-10, Qt::KeepAspectRatio));
        } else {
            p.drawImage(5, 5, m_image);
        }
    }
}

void ImageDisplay::slotExportImage()
{
    Q_ASSERT(!m_image.isNull());

    QFileDialog dialog(this, "Export render preview to image");
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::AnyFile);
    if (dialog.exec() == QDialog::Accepted) {
        m_image.save(dialog.selectedFiles().at(0));
    }
}
