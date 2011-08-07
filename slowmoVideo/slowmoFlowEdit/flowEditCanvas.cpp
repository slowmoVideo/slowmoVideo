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

    bool b = true;
    b &= connect(ui->flow, SIGNAL(signalRectDrawn(QRectF)), this, SLOT(slotRectDrawn(QRectF)));
    Q_ASSERT(b);
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
    Q_ASSERT(val > 0);
    m_boost = val;
    repaintFlow();
}

void FlowEditCanvas::repaintFlow()
{
    if (m_flowField != NULL) {
        ui->flow->loadImage(FlowVisualization_sV::colourizeFlow(m_flowField, m_boost));
        repaint();
    }
}

void FlowEditCanvas::slotRectDrawn(QRectF imageRect)
{
    qDebug() << "Rect drawn: " << imageRect;
    FlowTools_sV::refill(*m_flowField, imageRect.top(), imageRect.left(), imageRect.bottom(), imageRect.right());
    repaintFlow();
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
