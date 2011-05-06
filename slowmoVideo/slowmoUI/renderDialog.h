#ifndef RENDERDIALOG_H
#define RENDERDIALOG_H

#include <QDialog>

namespace Ui {
    class RenderDialog;
}

class RenderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RenderDialog(QWidget *parent = 0);
    ~RenderDialog();

public slots:
    void slotRenderingFinished();
    void slotRenderingAborted();
    void slotFrameRendered(qreal time, int frameNumber);

signals:
    void signalContinueRendering();
    void signalAbortRendering();
    void signalChangeFps(float fps);

private:
    Ui::RenderDialog *ui;

private slots:
    void slotStartClicked();
    void slotStopClicked();
    void slotFpsChanged();
};

#endif // RENDERDIALOG_H
