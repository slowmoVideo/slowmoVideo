/*
slowmoFlowEdit is a user interface for editing slowmoVideo's Optical Flow files.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#include "flowEditCanvas.h"
#include "ui_flowEditCanvas.h"

#include "lib/flowRW_sV.h"
#include "lib/flowTools_sV.h"
#include "lib/flowVisualization_sV.h"

#include <QtCore/QDebug>

FlowEditCanvas::FlowEditCanvas(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FlowEditCanvas),
    m_flowField(NULL),
    m_boost(1.0)
{
    ui->setupUi(this);

    ui->flow->trackMouse(true);

    connect(ui->flow, SIGNAL(signalRectDrawn(QRectF)), this, SLOT(slotRectDrawn(QRectF)));
    connect(ui->flow, SIGNAL(signalMouseMoved(float,float)), this, SLOT(slotExamineValues(float,float)));
    connect(ui->amplification, SIGNAL(valueChanged(int)),this, SLOT(newAmplification(int)));
    
}

FlowEditCanvas::~FlowEditCanvas()
{
    delete ui;
}

float FlowEditCanvas::amplification() const
{
    return m_boost;
}

void FlowEditCanvas::setAmplification(float val)
{
	//qDebug() << "setAmplification: " << val;
    Q_ASSERT(val > 0);
    m_boost = val;
    repaintFlow();
}

void FlowEditCanvas::newAmplification(int val)
{
	//qDebug() << "newAmplification: " << val;
    Q_ASSERT(val > 0);
    m_boost = (float)val;
    repaintFlow();
}

/// \todo Make flow visualization configurable
void FlowEditCanvas::repaintFlow()
{
    if (m_flowField != NULL) {
        ui->flow->loadImage(FlowVisualization_sV::colourizeFlow(m_flowField, FlowVisualization_sV::HSV, m_boost));
        repaint();
    }
}

void FlowEditCanvas::slotRectDrawn(QRectF imageRect)
{
    qDebug() << "Rect drawn: " << imageRect;
    if (m_flowField != NULL) {
    Kernel_sV k(8, 8);
    k.gauss();
    FlowTools_sV::deleteRect(*m_flowField, imageRect.top(), imageRect.left(), imageRect.bottom(), imageRect.right());
    FlowTools_sV::refill(*m_flowField, k, imageRect.top(), imageRect.left(), imageRect.bottom(), imageRect.right());
    repaintFlow();
    }
}

void FlowEditCanvas::slotLoadFlow(QString filename)
{
    if (m_flowField != NULL) {
        delete m_flowField;
        m_flowField = NULL;
    }
    m_flowField = FlowRW_sV::load(filename.toStdString());
    m_flowFilename = filename;

    repaintFlow();
}

void FlowEditCanvas::slotSaveFlow(QString filename)
{
    if (m_flowField != NULL) {
        if (filename.length() == 0) {
            filename = m_flowFilename;
        }
        FlowRW_sV::save(filename.toStdString(), m_flowField);
    } else {
        qDebug() << "No flow file loaded, cannot save.";
    }
}

void FlowEditCanvas::slotExamineValues(float x, float y)
{
    if (m_flowField != NULL) {
        if (x >= 0 && y >= 0
                && x <= m_flowField->width()-1 && y <= m_flowField->height()-1) {
            float dx = m_flowField->x(x,y);
            float dy = m_flowField->y(x,y);
            ui->lblValues->setText(QString("dx/dy: (%1|%2)").arg(dx, 0, 'f', 2).arg(dy, 0, 'f', 2));
            ui->lblPos->setText(QString("(%1|%2)").arg(x).arg(y));
        }
    }
}
