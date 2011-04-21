#ifndef PROCESSDIALOGEXTRACTFRAMES_H
#define PROCESSDIALOGEXTRACTFRAMES_H

#include <QDialog>
#include "../project/project_sV.h"

namespace Ui {
    class ProgressDialogExtractFrames;
}

class ProgressDialogExtractFrames : public QDialog
{
    Q_OBJECT

public:
    explicit ProgressDialogExtractFrames(QWidget *parent = 0);
    ~ProgressDialogExtractFrames();

public slots:
    void slotExtractionFinished(Project_sV::FrameSize frameSize);

private:
    Ui::ProgressDialogExtractFrames *ui;
};

#endif // PROCESSDIALOGEXTRACTFRAMES_H
