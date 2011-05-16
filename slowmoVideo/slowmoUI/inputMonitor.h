#ifndef INPUTMONITOR_H
#define INPUTMONITOR_H

#include <QWidget>
#include <QSemaphore>

namespace Ui {
    class InputMonitor;
}

class QImage;
class InputMonitor : public QWidget
{
    Q_OBJECT

public:
    explicit InputMonitor(QWidget *parent = 0);
    ~InputMonitor();

protected:
    virtual void paintEvent(QPaintEvent *event);


public slots:
    void slotLoadImage(const QString &filename);

private:
    Ui::InputMonitor *ui;

    QImage m_currentImage;

    QSemaphore m_semaphore;
    QString *m_queue[2];
};

#endif // INPUTMONITOR_H
