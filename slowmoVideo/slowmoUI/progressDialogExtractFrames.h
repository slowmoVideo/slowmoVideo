#ifndef PROCESSDIALOGEXTRACTFRAMES_H
#define PROCESSDIALOGEXTRACTFRAMES_H

#include <QDialog>
#include "../project/project_sV.h"
#include "../lib/defs_sV.hpp"

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
    void slotExtractionFinished(FrameSize frameSize);
    void slotProgressUpdated(FrameSize frameSize, int value);

private:
    Ui::ProgressDialogExtractFrames *ui;
};

#endif // PROCESSDIALOGEXTRACTFRAMES_H
