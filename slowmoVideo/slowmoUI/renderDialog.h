#ifndef RENDERDIALOG_H
#define RENDERDIALOG_H

#include <QDialog>

#include "../lib/defs_sV.h"

namespace Ui {
    class RenderDialog;
}

class Project_sV;
class RenderDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RenderDialog(const Project_sV *project, QWidget *parent = 0);
    ~RenderDialog();

public slots:
    void slotRenderingFinished();
    void slotRenderingAborted();
    void slotFrameRendered(qreal time, int frameNumber);

signals:
    void signalContinueRendering();
    void signalAbortRendering();
    void signalChangeFps(float fps);
    void signalChangeRenderFrameSize(FrameSize frameSize);

private:
    Ui::RenderDialog *ui;

private slots:
    void slotStartClicked();
    void slotStopClicked();
    void slotFpsChanged();
    void slotRenderFrameSizeChanged();
};

#endif // RENDERDIALOG_H
