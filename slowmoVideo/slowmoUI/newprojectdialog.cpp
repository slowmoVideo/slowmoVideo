#include "newprojectdialog.h"
#include "ui_newprojectdialog.h"


#include <QDebug>

#include <QtCore/QFile>
#include <QtCore/QDir>

#include <QFileDialog>
#include <QButtonGroup>

QColor NewProjectDialog::colOk(158, 245, 94);
QColor NewProjectDialog::colBad(247, 122, 48);

NewProjectDialog::NewProjectDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NewProjectDialog)
{
    ui->setupUi(this);

    m_buttonGroup = new QButtonGroup(this);
    m_buttonGroup->addButton(ui->radioVideo);
    m_buttonGroup->addButton(ui->radioImages);
    ui->radioVideo->setChecked(true);

    m_videoInfo.streamsCount = 0;

    bool b = true;
    b &= connect(ui->browseInputVideo, SIGNAL(clicked()), this, SLOT(slotSelectVideoFile()));
    b &= connect(ui->browseProjectDir, SIGNAL(clicked()), this, SLOT(slotSelectProjectDir()));
    b &= connect(ui->inputVideo, SIGNAL(textChanged(QString)), this, SLOT(slotUpdateVideoInfo()));
    b &= connect(ui->projectDir, SIGNAL(textChanged(QString)), this, SLOT(slotUpdateButtonStates()));

    b &= connect(ui->bAbort, SIGNAL(clicked()), this, SLOT(reject()));
    b &= connect(ui->bOk, SIGNAL(clicked()), this, SLOT(accept()));

    b &= connect(m_buttonGroup, SIGNAL(buttonClicked(int)), this, SLOT(slotUpdateFrameSourceType()));
    Q_ASSERT(b);

    slotUpdateButtonStates();
    slotUpdateFrameSourceType();
}

NewProjectDialog::~NewProjectDialog()
{
    delete ui;
    delete m_buttonGroup;
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
        slotUpdateButtonStates();
    }
}

void NewProjectDialog::slotUpdateVideoInfo()
{
    QFile file(ui->inputVideo->text());
    if (file.exists()) {
        m_videoInfo = getInfo(ui->inputVideo->text().toStdString().c_str());
        QString text = QString::fromUtf8("Number of video streams: %1\nFrames: %2\nSize: %3×%4\n")
                .arg(m_videoInfo.streamsCount).arg(m_videoInfo.framesCount)
                .arg(m_videoInfo.width).arg(m_videoInfo.height);
        text.append(QString("Frame rate: %1/%2").arg(m_videoInfo.frameRateNum).arg(m_videoInfo.frameRateDen));
        ui->txtVideoInfo->setPlainText(text);
    } else {
        m_videoInfo.streamsCount = 0;
        ui->txtVideoInfo->setPlainText("No video stream detected.");
    }
    slotUpdateButtonStates();
}

void NewProjectDialog::slotUpdateButtonStates()
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
        QDir dir(ui->projectDir->text());
        ui->cbDirectoryCreated->setChecked(!dir.exists());
        ui->projectDir->setStyleSheet(QString("QLineEdit { background-color: %1; }").arg(colOk.name()));
        m_projectDir = ui->projectDir->text();
    } else {
        ui->projectDir->setStyleSheet(QString("QLineEdit { background-color: %1; }").arg(colBad.name()));
        ok = false;
    }

    ui->bOk->setEnabled(ok);
}

void NewProjectDialog::slotUpdateFrameSourceType()
{
    ui->groupImages->setEnabled(ui->radioImages->isChecked());
    ui->groupImages->setVisible(ui->radioImages->isChecked());
    ui->groupVideo->setEnabled(ui->radioVideo->isChecked());
    ui->groupVideo->setVisible(ui->radioVideo->isChecked());
    adjustSize();
}
