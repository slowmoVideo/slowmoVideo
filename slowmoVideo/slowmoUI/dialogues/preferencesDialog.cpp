#include "preferencesDialog.h"
#include "ui_preferencesDialog.h"

#include "project/flowSourceV3D_sV.h"
#include "lib/defs_sV.hpp"
#include "lib/avconvInfo_sV.h"
#include "../../project/videoFrameSource_sV.h"
#include <QtCore/QProcess>
#include <QtGui/QFileDialog>

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);
    ui->buildFlow->setText(m_settings.value("binaries/v3dFlowBuilder", "").toString());
    ui->ffmpeg->setText(m_settings.value("binaries/ffmpeg", "ffmpeg").toString());

#if QT_VERSION >= 0x040700
    ui->buildFlow->setPlaceholderText(QApplication::translate("PreferencesDialog", "flowBuilder binary location", 0, QApplication::UnicodeUTF8));
#endif

    m_flowMethodGroup.addButton(ui->methodOCV);
    m_flowMethodGroup.addButton(ui->methodV3D);
    m_flowMethodGroup.setExclusive(true);

    QString method = m_settings.value("preferences/flowMethod", "V3D").toString();
    if ("V3D" == method) {
        ui->methodV3D->setChecked(true);
    } else {
        ui->methodOCV->setChecked(true);
    }

    bool b = true;
    b &= connect(ui->bOk, SIGNAL(clicked()), this, SLOT(accept()));
    b &= connect(ui->bCancel, SIGNAL(clicked()), this, SLOT(reject()));
    b &= connect(ui->bBuildFlow, SIGNAL(clicked()), this, SLOT(slotBrowseFlow()));
    b &= connect(ui->buildFlow, SIGNAL(textChanged(QString)), this, SLOT(slotValidateFlowBinary()));
    b &= connect(&m_flowMethodGroup, SIGNAL(buttonClicked(int)), this, SLOT(slotUpdateFlowMethod()));
    Q_ASSERT(b);

    if (!FlowSourceV3D_sV::validateFlowBinary(ui->buildFlow->text())) {
        FlowSourceV3D_sV::correctFlowBinaryLocation();
        ui->buildFlow->setText(m_settings.value("binaries/v3dFlowBuilder", "").toString());
    }
    slotValidateFlowBinary();
}

PreferencesDialog::~PreferencesDialog()
{
    delete ui;
}

void PreferencesDialog::accept()
{
    // V3D binary location
    if (FlowSourceV3D_sV::validateFlowBinary(ui->buildFlow->text())) {
        m_settings.setValue("binaries/v3dFlowBuilder", ui->buildFlow->text());
    }

    // Flow method
    QString method("OpenCV-Farnback");
    if (ui->methodV3D->isChecked()) {
        method = "V3D";
    }
    m_settings.setValue("preferences/flowMethod", method);

    // ffmpeg location
    if (AvconvInfo::testAvconvExecutable(ui->ffmpeg->text())) {
        m_settings.setValue("binaries/ffmpeg", ui->ffmpeg->text());
    } else {
        qDebug() << "Not a valid ffmpeg/avconv executable: " << ui->ffmpeg->text();
    }

    // Store the values right now
    m_settings.sync();

    QDialog::accept();
}

void PreferencesDialog::slotUpdateFlowMethod()
{
}
void PreferencesDialog::slotUpdateFfmpeg()
{
    m_settings.setValue("binaries/ffmpeg", ui->ffmpeg->text());
}

void PreferencesDialog::slotValidateFlowBinary()
{
    if (FlowSourceV3D_sV::validateFlowBinary(ui->buildFlow->text())) {
        ui->buildFlow->setStyleSheet(QString("QLineEdit { background-color: %1; }").arg(Colours_sV::colOk.name()));
        ui->methodV3D->setEnabled(true);
    } else {
        ui->buildFlow->setStyleSheet(QString("QLineEdit { background-color: %1; }").arg(Colours_sV::colBad.name()));
        ui->methodV3D->setEnabled(false);
        ui->methodOCV->setChecked(true);
    }
}

void PreferencesDialog::slotBrowseFlow()
{
    QFileDialog dialog;
    dialog.setAcceptMode(QFileDialog::AcceptOpen);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setDirectory(QFileInfo(ui->buildFlow->text()).absolutePath());
    if (dialog.exec() == QDialog::Accepted) {
        ui->buildFlow->setText(dialog.selectedFiles().at(0));
        slotValidateFlowBinary();
    }
}
