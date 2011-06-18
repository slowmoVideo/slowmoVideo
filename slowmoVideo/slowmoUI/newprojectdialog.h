#ifndef NEWPROJECTDIALOG_H
#define NEWPROJECTDIALOG_H

#include <QDialog>

#include "../project/project_sV.h"

extern "C" {
    #include "../lib/videoInfo_sV.h"
}

namespace Ui {
    class NewProjectDialog;
}

class QButtonGroup;

class NewProjectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewProjectDialog(QWidget *parent = 0);
    ~NewProjectDialog();

    QString m_inputFile;
    QString m_projectDir;

    Project_sV* buildProject();
    const QString projectFilename() const;

private:


    Ui::NewProjectDialog *ui;
    QButtonGroup *m_buttonGroup;

    VideoInfoSV m_videoInfo;


private slots:
    void slotSelectVideoFile();
    void slotSelectProjectDir();

    void slotUpdateVideoInfo();

    void slotUpdateButtonStates();
    void slotUpdateFrameSourceType();
};

#endif // NEWPROJECTDIALOG_H
