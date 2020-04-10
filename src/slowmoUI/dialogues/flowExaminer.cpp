#include "flowExaminer.h"
#include "ui_flowExaminer.h"
#include "project/project_sV.h"
#include "project/abstractFrameSource_sV.h"
#include "lib/flowVisualization_sV.h"

#include <QtCore/QDebug>
#include <QtGui/QPainter>
#include <QKeyEvent>

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

    connect(ui->leftFrame, SIGNAL(signalMouseMoved(float,float)), this, SLOT(slotMouseMoved(float,float)));
    connect(ui->rightFrame, SIGNAL(signalMouseMoved(float,float)), this, SLOT(slotMouseMoved(float,float)));
    connect(ui->bClose, SIGNAL(clicked()), this, SLOT(close()));
    
    connect(ui->amplification, SIGNAL(valueChanged(int)),this, SLOT(newAmplification(int)));
    
    connect(this, SIGNAL(frameChanged()),this, SLOT(updateFlow()));
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

/// \todo Make flow visualization configurable
void FlowExaminer::examine(int leftFrame)
{
	frame = leftFrame;
	frame= 0;
	loadFlow();;
}
	
void FlowExaminer::loadFlow()
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
        m_flowLR = m_project->requestFlow(frame, frame+1, FrameSize_Orig);
        m_flowRL = m_project->requestFlow(frame+1, frame, FrameSize_Orig);
        ui->leftFrame->loadImage(m_project->frameSource()->frameAt(frame, FrameSize_Orig));
        ui->rightFrame->loadImage(m_project->frameSource()->frameAt(frame+1, FrameSize_Orig));        
        ui->leftFlow->loadImage(FlowVisualization_sV::colourizeFlow(m_flowLR, FlowVisualization_sV::HSV,m_boost));
        ui->rightFlow->loadImage(FlowVisualization_sV::colourizeFlow(m_flowRL, FlowVisualization_sV::HSV,m_boost));
        emit updateFlow();
    } catch (FlowBuildingError &err) { }

    //repaint();
}

void FlowExaminer::updateFlow()
{
	ui->leftFrame->update();
	ui->rightFrame->update();
	ui->leftFlow->update();
	ui->rightFlow->update();
}

void FlowExaminer::newAmplification(int val)
{
	//qDebug() << "newAmplification: " << val;
    Q_ASSERT(val > 0);
    m_boost = (float)val;
    // reload flow with new gain
    loadFlow();
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

void FlowExaminer::keyPressEvent(QKeyEvent *event)
{
	//qDebug() << "keypressed : " << event->key();
	switch (event->key()) {
                case Qt::Key_Up:
                    qDebug() << "key up";
                    //m_states.prevMousePos += QPoint(0,-1);
                    break;
                case Qt::Key_Down:
                    qDebug() << "key down";
                    //m_states.prevMousePos += QPoint(0,1);
                    break;
                case Qt::Key_Right:
                    qDebug() << "key right";
                    //m_states.prevMousePos += QPoint(1,0);
                    frame++;
                    loadFlow();
                    break;
                case Qt::Key_Left:
                    qDebug() << "key left";
                    //m_states.prevMousePos += QPoint(-1,0);
                    frame--;
                    loadFlow();
                    break;
            }
	QWidget::keyPressEvent(event);
	//repaint();
}

void FlowExaminer::wheelEvent(QWheelEvent *event)
{
	int numDegrees = event->delta() / 8;
    int numSteps = numDegrees / 15;

     if (event->orientation() == Qt::Horizontal) {
         qDebug() << "wheel : horiz " << numSteps;
     } else {
         qDebug() << "wheel : vert " << numSteps;
     }
     qDebug() << "in wheel";
     event->accept();
}
