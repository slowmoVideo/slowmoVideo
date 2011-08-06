#ifndef FLOWEDITCANVAS_H
#define FLOWEDITCANVAS_H

#include <QWidget>
#include <QtCore/QRectF>

class FlowField_sV;
namespace Ui {
    class FlowEditCanvas;
}

class FlowEditCanvas : public QWidget
{
    Q_OBJECT

public:
    explicit FlowEditCanvas(QWidget *parent = 0);
    ~FlowEditCanvas();

public slots:
    void slotLoadFlow(QString filename);
    void slotSaveFlow(QString filename = QString());

private:
    Ui::FlowEditCanvas *ui;

    FlowField_sV *m_flowField;
    QString m_flowFilename;

private slots:
    void slotRectDrawn(QRectF imageRect);
};

#endif // FLOWEDITCANVAS_H
