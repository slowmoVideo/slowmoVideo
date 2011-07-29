#include "flowExaminer.h"
#include "ui_flowExaminer.h"
#include "project/project_sV.h"
#include "project/abstractFrameSource_sV.h"
#include "lib/flowVisualization_sV.h"

#include <QtCore/QDebug>
#include <QtGui/QPainter>

FlowExaminer::FlowExaminer(Project_sV *project, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FlowExaminer),
    m_project(project),
    m_flowLR(NULL),
    m_flowRL(NULL)
{
    ui->setupUi(this);
//    ui->leftFrame->trackMouse(true);
//    ui->rightFrame->trackMouse(true);

    bool b = true;
    b &= connect(ui->leftFrame, SIGNAL(signalMouseMoved(float,float)), this, SLOT(slotMouseMoved(float,float)));
    b &= connect(ui->rightFrame, SIGNAL(signalMouseMoved(float,float)), this, SLOT(slotMouseMoved(float,float)));
    b &= connect(ui->bClose, SIGNAL(clicked()), this, SLOT(close()));
    Q_ASSERT(b);
}

FlowExaminer::~FlowExaminer()
{
    delete ui;
    if (m_flowLR != NULL) {
        delete m_flowLR;
    }
    if (m_flowRL != NULL) {
        delete m_flowRL;
    }
}

void FlowExaminer::examine(int leftFrame)
{
    if (m_flowLR != NULL) {
        delete m_flowLR;
        m_flowLR = NULL;
    }
    if (m_flowRL != NULL) {
        delete m_flowRL;
        m_flowRL = NULL;
    }
    try {
        m_flowLR = m_project->requestFlow(leftFrame, leftFrame+1, FrameSize_Orig);
        m_flowRL = m_project->requestFlow(leftFrame+1, leftFrame, FrameSize_Orig);
        ui->leftFrame->loadImage(m_project->frameSource()->frameAt(leftFrame, FrameSize_Orig));
        ui->rightFrame->loadImage(m_project->frameSource()->frameAt(leftFrame+1, FrameSize_Orig));
        ui->leftFlow->loadImage(FlowVisualization_sV::colourizeFlow(m_flowLR));
        ui->rightFlow->loadImage(FlowVisualization_sV::colourizeFlow(m_flowRL));
    } catch (FlowBuildingError &err) { }

    repaint();
}

/// \todo Show vectors etc.
void FlowExaminer::slotMouseMoved(float x, float y)
{
    if (QObject::sender() == ui->leftFrame) {
        qDebug() << "Should display something in the right frame now.";
        if (m_flowLR != NULL) {
            float moveX = m_flowLR->x(x,y);
            float moveY = m_flowLR->y(x,y);
            QImage leftOverlay(m_flowLR->width(), m_flowLR->height(), QImage::Format_ARGB32);
            QImage rightOverlay(leftOverlay.size(), QImage::Format_ARGB32);
            QPainter davinci;

            davinci.begin(&leftOverlay);
            davinci.drawLine(x, y, x+moveX, y+moveY);
            davinci.end();

            qDebug() << "Line coordinates: " << x << y << moveX << moveY;
            bool ok;
            ok = ui->leftFrame->loadOverlay(leftOverlay);
            Q_ASSERT(ok);

            davinci.begin(&rightOverlay);
            davinci.drawEllipse(x+moveX, y+moveY, 2, 2);
            davinci.end();

            ui->rightFrame->loadOverlay(rightOverlay);

            repaint();
        } else {
            qDebug() << "Flow is 0!";
        }
    } else if (QObject::sender() == ui->rightFrame) {
        qDebug() << "Should display something in the left frame now.";

    } else {
        qDebug() << "Unknown sender!";
        Q_ASSERT(false);
    }
}
