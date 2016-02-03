/*
slowmoUI is a user interface for slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "frameMonitor.h"
#include "ui_frameMonitor.h"

#include <QImage>
#include <QPainter>
#include <QDebug>
#include <QSettings>

FrameMonitor::FrameMonitor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FrameMonitor),
    m_semaphore(1)
{
    ui->setupUi(this);
    
    m_queue[0] = NULL;
    m_queue[1] = NULL;
    
    imgCache.clear();
	  setCacheLimit(10240); // cache size of 10Mb
}

FrameMonitor::~FrameMonitor()
{
    delete ui;
    if (m_queue[0] != NULL) { delete m_queue[0]; }
    if (m_queue[1] != NULL) { delete m_queue[1]; }
}

/**
 *  Sets the cache limit to n kilobytes.
*/
void FrameMonitor::setCacheLimit(int n)
{
		cache_limit = n;
    imgCache.setMaxCost(1024 * cache_limit);
}

void FrameMonitor::slotLoadImage(const QString &filename)
{
    m_semaphore.acquire();
    if (m_queue[0] == NULL) {
        m_queue[0] = new QString(filename);
    } else {
        if (m_queue[1] != NULL) {
            delete m_queue[1];
            m_queue[1] = NULL;
        }
        m_queue[1] = new QString(filename);
    }
    m_semaphore.release();
    repaint();
}

void FrameMonitor::closeEvent(QCloseEvent *event)
{
    QWidget::closeEvent(event);
}

void FrameMonitor::paintEvent(QPaintEvent *)
{
    QString image;
    m_semaphore.acquire();
    if (m_queue[0] != NULL) {
        image = *m_queue[0];
        delete m_queue[0];
        m_queue[0] = NULL;
    }
    if (m_queue[1] != NULL) {
        m_queue[0] = m_queue[1];
        m_queue[1] = NULL;
    }
    m_semaphore.release();

    if (!image.isNull()) {
    	// add some better cache mgmt
    	QImage *_image;
        //qDebug() << "cost : " << imgCache.totalCost();
    	 if(imgCache.contains(image)) {
	     	//return *(frameCache.object(path));
	     	//qDebug() << "cache : " << image;
	     	_image = imgCache.object(image);
	     } else {
	     	_image = new QImage(image);
	     	//qDebug() << "cache store : " << image << "cost : " << _image->byteCount();
		    //TODO: provide a method for that
	     	bool success = imgCache.insert(image, _image, _image->byteCount());
		    if ( !success)  {
					qDebug() << "WARN: memory error";
				   _image = new QImage(image);
		    }
					
	     }
	    
	    if (_image != 0)
        	ui->imageDisplay->loadImage(*_image);
    }
}
