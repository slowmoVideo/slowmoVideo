#include "newprojectdialog.h"
#include "ui_newprojectdialog.h"


#include <QDebug>

#include <QFile>
#include <QDir>
#include <QFileDialog>

QColor NewProjectDialog::colOk(158, 245, 94);
QColor NewProjectDialog::colBad(247, 122, 48);

NewProjectDialog::NewProjectDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewProjectDialog)
{
    ui->setupUi(this);

    m_videoInfo.streamsCount = 0;

    bool b = true;
    b &= connect(ui->browseInputVideo, SIGNAL(clicked()), this, SLOT(slotSelectVideoFile()));
    b &= connect(ui->browseProjectDir, SIGNAL(clicked()), this, SLOT(slotSelectProjectDir()));
    b &= connect(ui->inputVideo, SIGNAL(textChanged(QString)), this, SLOT(slotUpdateVideoInfo()));
    b &= connect(ui->projectDir, SIGNAL(textChanged(QString)), this, SLOT(updateButtonStates()));

    b &= connect(ui->bAbort, SIGNAL(clicked()), this, SLOT(reject()));
    b &= connect(ui->bOk, SIGNAL(clicked()), this, SLOT(accept()));
    Q_ASSERT(b);

    updateButtonStates();
}

NewProjectDialog::~NewProjectDialog()
{
    delete ui;
}

void NewProjectDialog::slotSelectVideoFile()
{
    QFileDialog dialog(this, "Select input video file");
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::ExistingFile);
    if (dialog.exec() == QDialog::Accepted) {
        ui->inputVideo->setText(dialog.selectedFiles().at(0));
        ui->txtVideoInfo->clear();

        slotUpdateVideoInfo();
    }
}

void NewProjectDialog::slotSelectProjectDir()
{
    QFileDialog dialog(this, "Select a project directory");
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::Directory);
    if (dialog.exec() == QDialog::Accepted) {
        ui->projectDir->setText(dialog.selectedFiles().at(0));
        updateButtonStates();
    }
}

void NewProjectDialog::slotUpdateVideoInfo()
{
    QFile file(ui->inputVideo->text());
    if (file.exists()) {
        m_videoInfo = getInfo(ui->inputVideo->text().toStdString().c_str());
        QString text = QString("Number of video streams: %1\nFrames: %2\n").arg(m_videoInfo.streamsCount).arg(m_videoInfo.framesCount);
        text.append(QString("Frame rate: %1/%2").arg(m_videoInfo.frameRateNum).arg(m_videoInfo.frameRateDen));
        ui->txtVideoInfo->setPlainText(text);
    } else {
        m_videoInfo.streamsCount = 0;
        ui->txtVideoInfo->setPlainText("No video stream detected.");
    }
    updateButtonStates();
}

void NewProjectDialog::updateButtonStates()
{
    bool ok = true;

    if (m_videoInfo.streamsCount > 0) {
        ui->inputVideo->setStyleSheet(QString("QLineEdit { background-color: %1; }").arg(colOk.name()));
        m_inputFile = ui->inputVideo->text();
    } else {
        ui->inputVideo->setStyleSheet(QString("QLineEdit { background-color: %1; }").arg(colBad.name()));
        ok = false;
    }
    if (ui->projectDir->text().length() > 0) {
        ui->projectDir->setStyleSheet(QString("QLineEdit { background-color: %1; }").arg(colOk.name()));
        m_projectDir = ui->projectDir->text();
    } else {
        ui->projectDir->setStyleSheet(QString("QLineEdit { background-color: %1; }").arg(colBad.name()));
        ok = false;
    }

    ui->bOk->setEnabled(ok);
}
