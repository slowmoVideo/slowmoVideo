#include "preferencesDialog.h"
#include "ui_preferencesDialog.h"

#include "project/v3dFlowSource_sV.h"
#include "lib/defs_sV.hpp"
#include <QtCore/QProcess>
#include <QtGui/QFileDialog>

PreferencesDialog::PreferencesDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PreferencesDialog)
{
    ui->setupUi(this);
    ui->buildFlow->setText(m_settings.value("binaries/v3dFlowBuilder", "").toString());

    bool b = true;
    b &= connect(ui->bOk, SIGNAL(clicked()), this, SLOT(accept()));
    b &= connect(ui->bCancel, SIGNAL(clicked()), this, SLOT(reject()));
    b &= connect(ui->bBuildFlow, SIGNAL(clicked()), this, SLOT(slotBrowseFlow()));
    b &= connect(ui->buildFlow, SIGNAL(textChanged(QString)), this, SLOT(slotValidateFlowBinary()));
    Q_ASSERT(b);

    if (!V3dFlowSource_sV::validateFlowBinary(ui->buildFlow->text())) {
        V3dFlowSource_sV::correctFlowBinaryLocation();
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
    if (V3dFlowSource_sV::validateFlowBinary(ui->buildFlow->text())) {
        m_settings.setValue("binaries/v3dFlowBuilder", ui->buildFlow->text());
    }
    QDialog::accept();
}

void PreferencesDialog::slotValidateFlowBinary()
{
    if (V3dFlowSource_sV::validateFlowBinary(ui->buildFlow->text())) {
        ui->buildFlow->setStyleSheet(QString("QLineEdit { background-color: %1; }").arg(Colours_sV::colOk.name()));
    } else {
        ui->buildFlow->setStyleSheet(QString("QLineEdit { background-color: %1; }").arg(Colours_sV::colBad.name()));
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
