#include "inputMonitor.h"
#include "ui_inputMonitor.h"

#include <QImage>
#include <QPainter>
#include <QDebug>

InputMonitor::InputMonitor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InputMonitor),
    m_semaphore(1)
{
    ui->setupUi(this);
    m_queue[0] = NULL;
    m_queue[1] = NULL;
}

InputMonitor::~InputMonitor()
{
    delete ui;
    if (m_queue[0] != NULL) { delete m_queue[0]; }
    if (m_queue[1] != NULL) { delete m_queue[1]; }
}

void InputMonitor::slotLoadImage(const QString &filename)
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

void InputMonitor::paintEvent(QPaintEvent *)
{
//    QWidget::paintEvent(event);

    m_semaphore.acquire();
    if (m_queue[0] != NULL) {
        m_currentImage = QImage(*m_queue[0]);
        delete m_queue[0];
        m_queue[0] = NULL;
    }
    if (m_queue[1] != NULL) {
        m_queue[0] = m_queue[1];
        m_queue[1] = NULL;
    }
    m_semaphore.release();

    QPainter davinci(this);
    davinci.drawImage(0, 0, m_currentImage);

}
