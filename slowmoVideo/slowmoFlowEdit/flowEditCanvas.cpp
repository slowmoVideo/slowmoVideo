#include "flowEditCanvas.h"
#include "ui_flowEditCanvas.h"

#include "lib/flowRW_sV.h"
#include "lib/flowTools_sV.h"
#include "lib/flowVisualization_sV.h"

#include <QtCore/QDebug>

FlowEditCanvas::FlowEditCanvas(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FlowEditCanvas),
    m_flowField(NULL)
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

void FlowEditCanvas::slotRectDrawn(QRectF imageRect)
{
    qDebug() << "Rect drawn: " << imageRect;
    FlowTools_sV::refill(*m_flowField, imageRect.top(), imageRect.left(), imageRect.bottom(), imageRect.right());
    ui->flow->loadImage(FlowVisualization_sV::colourizeFlow(m_flowField));
    repaint();
}

void FlowEditCanvas::slotLoadFlow(QString filename)
{
    if (m_flowField != NULL) {
        delete m_flowField;
        m_flowField = NULL;
    }
    m_flowField = FlowRW_sV::load(filename.toStdString());
    m_flowFilename = filename;

    ui->flow->loadImage(FlowVisualization_sV::colourizeFlow(m_flowField));
    repaint();
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
