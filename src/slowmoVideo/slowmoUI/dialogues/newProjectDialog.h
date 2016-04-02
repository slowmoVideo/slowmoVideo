/*
slowmoUI is a user interface for slowmoVideo.
Copyright (C) 2011  Simon A. Eugster (Granjow)  <simon.eu@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
*/

#ifndef NEWPROJECTDIALOG_H
#define NEWPROJECTDIALOG_H

#include <QDialog>
#include <QtCore/QStringList>
#include <QSettings>

#include "project/project_sV.h"

#include "lib/videoInfo_sV.h"

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

    Project_sV* buildProject() throw(FrameSourceError);
    const QString projectFilename() const;

private:
    Ui::NewProjectDialog *ui;
    QButtonGroup *m_buttonGroup;

    VideoInfoSV m_videoInfo;
    QStringList m_images;
    QString m_imagesMsg;

    QSettings m_settings;


private slots:
    void slotSelectProjectDir();
    void slotSelectVideoFile();
    void slotSelectImages();

    void slotUpdateVideoInfo();
    void slotUpdateImagesInfo();

    void slotUpdateButtonStates();
    void slotUpdateFrameSourceType();
};

#endif // NEWPROJECTDIALOG_H
